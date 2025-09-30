/*******************************************************************************
 * @file  thermal_monitor.c
 * @brief thermal
 * @author chunxiao.dai@neuroxess.com
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/sys/mutex.h>
#include "driver/bsp_tmp119.h"
#include "rtc.h"
#include "sensor.h"
#include "events.h"
#include "info.h"


#define LOG_MODULE_NAME thermal
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

const struct i2c_dt_spec thermal_dev = I2C_DT_SPEC_GET(DT_NODELABEL(sensor_tmp119));

#define TEMP_BUFFER_ITEMS 5
static uint8_t temp_buffer[TEMP_ITEM_SIZE * TEMP_BUFFER_ITEMS];

//!< Thread Config
#define THERMAL_COLLECTION_PRIORITY    7
#define THERMAL_COLLECTION_STACK_SIZE  1024


static void tm_override_buffer_init(void)
{
    ring_buf_init(&temp_ring_buf, sizeof(temp_buffer), temp_buffer);
    k_mutex_init(&temp_mutex);
    k_event_init(&temp_event);
}

/**
 * @brief override write function
 */
static void tm_smart_buffer_write(const struct sensor_value *temp)
{
    k_mutex_lock(&temp_mutex, K_FOREVER);

    //!< Dynamic override
    while (ring_buf_space_get(&temp_ring_buf) < TEMP_ITEM_SIZE) {
        uint8_t dummy[TEMP_ITEM_SIZE];
        ring_buf_get(&temp_ring_buf, dummy, TEMP_ITEM_SIZE);
    }

    ring_buf_put(&temp_ring_buf, (uint8_t *)temp, TEMP_ITEM_SIZE);

    //!< Notify data ready
    k_event_set(&temp_event, TEMP_UPDATE_EVENT);

    k_mutex_unlock(&temp_mutex);
}

void thermal_collection_thread(void)
{
    struct sensor_value temp_value;
    struct tmp119_data drv_data;

    tm_override_buffer_init();

    int err = tmp119_init(&thermal_dev);
    if (err < 0) {
        LOG_ERR("TMP119 init fail!");
        return ;
    }

    //!< It has been measured that the single collection process requires approximately 1.3 ms
    while (1) {

        err = k_mutex_lock(&i2c1_bus_lock, K_NO_WAIT);
        if (err < 0) {
            LOG_DBG("I2C bus lock failed (%d)", err);
            k_sleep(K_MSEC(5));
            continue;
        }
        err = tmp119_sample_fetch(&thermal_dev, &drv_data);
        k_mutex_unlock(&i2c1_bus_lock);
        if (err < 0)
        {
            LOG_ERR("Failed to fetch measurements (%d)", err);
            k_sleep(K_MSEC(1000));
            continue;
        }

        tmp119_channel_get(thermal_dev.bus, &temp_value, &drv_data);

        tm_smart_buffer_write(&temp_value);
        LOG_DBG("<>temp is %d.%06d oC, %d", temp_value.val1, temp_value.val2, ring_buf_space_get(&temp_ring_buf));
#if 0
        int16_t value = temp_value.val1 * 10 + temp_value.val2 / 100000;
        if (value >= system_limits.board_temp_limit)
        // if (value >= 275)
        {
            struct event evt = {
                .level = EVENT_CRITICAL,
                .type = EVENT_CRIT_BATT_TEMP_OVERHEAT,
                .timestamp = rtc_calibrated_time_get(rtc_dev),
                .data = value,
            };
            zbus_chan_pub(&critical_chan, &evt, K_MSEC(10));
        }
#endif
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(thermal_collection_thread_id, THERMAL_COLLECTION_STACK_SIZE, thermal_collection_thread, NULL, NULL,
        NULL, THERMAL_COLLECTION_PRIORITY, 0, 0);
