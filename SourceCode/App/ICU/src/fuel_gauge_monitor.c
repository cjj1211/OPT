/*******************************************************************************
 * @file  fuel_gauge_monitor.c
 * @brief fuel gauge
 * @author chunxiao.dai@neuroxess.com
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/ring_buffer.h>
#include "sensor.h"
#include "events.h"
#include "info.h"
#include "rtc.h"
#include "public_api.h"
#include "driver/bq27427/bq274xx.h"

#define LOG_MODULE_NAME fuel_gauge
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

#define FG_BUFFER_ITEMS 2
static uint8_t fuel_gauge_buffer[FUEL_GAUGE_ITEM_SIZE * FG_BUFFER_ITEMS];

//!< Thread Config
#define FUEL_GAUGE_PRIORITY    7
#define FULE_GAUGE_STACK_SIZE  1024

#define SAMPLING_TIMES_MS  (5000)

static void fgm_override_buffer_init(void)
{
    ring_buf_init(&fuel_gauge_ring_buf, sizeof(fuel_gauge_buffer), fuel_gauge_buffer);
    k_mutex_init(&fuel_gauge_mutex);
    k_event_init(&fuel_gauge_event);
}

/**
 * @brief override write function
 */
static void fgm_smart_buffer_write(const struct fuel_gauge_data *value)
{
    k_mutex_lock(&fuel_gauge_mutex, K_FOREVER);

    //!< Dynamic override
    while (ring_buf_space_get(&fuel_gauge_ring_buf) < FUEL_GAUGE_ITEM_SIZE) {
        uint8_t dummy[FUEL_GAUGE_ITEM_SIZE];
        ring_buf_get(&fuel_gauge_ring_buf, dummy, FUEL_GAUGE_ITEM_SIZE);
    }

    ring_buf_put(&fuel_gauge_ring_buf, (uint8_t *)value, FUEL_GAUGE_ITEM_SIZE);

    //!< Notify data ready
    k_event_set(&fuel_gauge_event, FUEL_GAUGE_UPDATE_EVENT);

    k_mutex_unlock(&fuel_gauge_mutex);
}

static void bq274xx_show_values(const char *type, struct sensor_value value)
{
    if ((value.val2 < 0) && (value.val1 >= 0)) {
        value.val2 = -(value.val2);
        LOG_DBG("%s: -%d.%06d", type, value.val1, value.val2);
    } else if ((value.val2 > 0) && (value.val1 < 0)) {
        LOG_DBG("%s: %d.%06d", type, value.val1, value.val2);
    } else if ((value.val2 < 0) && (value.val1 < 0)) {
        value.val2 = -(value.val2);
        LOG_DBG("%s: %d.%06d", type, value.val1, value.val2);
    } else {
        LOG_DBG("%s: %d.%06d", type, value.val1, value.val2);
    }
}

static void do_main(const struct device *dev)
{
    int err = 0;
    struct fuel_gauge_data rb_data;

    while (1) {
        err = k_mutex_lock(&i2c1_bus_lock, K_NO_WAIT);
        if (err < 0) {
            LOG_DBG("I2C bus lock failed (%d)", err);
            k_sleep(K_MSEC(2));
            continue;
        }
        //!< The actual measurement shows that the collection process requires approximately 4.8 ms
        err = sensor_sample_fetch_chan(dev, SENSOR_CHAN_GAUGE_VOLTAGE);
        if (err < 0) {
            LOG_ERR("Unable to fetch the voltage (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        err = sensor_channel_get(dev, SENSOR_CHAN_GAUGE_VOLTAGE, &rb_data.voltage);
        if (err < 0) {
            LOG_ERR("Unable to get the voltage value (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        LOG_DBG("Voltage: %d.%06dV", rb_data.voltage.val1, rb_data.voltage.val2);

        err = sensor_sample_fetch_chan(dev, SENSOR_CHAN_GAUGE_AVG_CURRENT);
        if (err < 0) {
            LOG_ERR("Unable to fetch the Average current (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        err = sensor_channel_get(dev, SENSOR_CHAN_GAUGE_AVG_CURRENT, &rb_data.current);
        if (err < 0) {
            LOG_ERR("Unable to get the current value (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        bq274xx_show_values("Avg Current in Amps", rb_data.current);

        err = sensor_sample_fetch_chan(dev, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE);
        if (err < 0) {
            LOG_ERR("Unable to fetch State of Charge (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        err = sensor_channel_get(dev, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE, &rb_data.state_of_charge);
        if (err < 0) {
            LOG_ERR("Unable to get state of charge (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        LOG_DBG("State of charge: %d%%", rb_data.state_of_charge.val1);

        err = sensor_sample_fetch_chan(dev, SENSOR_CHAN_GAUGE_STATE_OF_HEALTH);
        if (err < 0) {
            LOG_ERR("Failed to fetch State of Health (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        err = sensor_channel_get(dev, SENSOR_CHAN_GAUGE_STATE_OF_HEALTH, &rb_data.state_of_health);
        if (err < 0) {
            LOG_ERR("Unable to get state of health (%d)", err);
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
            continue;
        }

        LOG_DBG("State of health: %d%%", rb_data.state_of_health.val1);
        k_mutex_unlock(&i2c1_bus_lock);

        fgm_smart_buffer_write(&rb_data);

        //!< Under-voltage monitoring
        uint16_t value = rb_data.voltage.val1 * 1000 + rb_data.voltage.val2 / 1000;
        if (value <= system_limits.undervoltage_limit)
        // if (value <= 5100)
        {
            LOG_DBG("voltage: %d", rb_data.voltage.val1 * 1000 + rb_data.voltage.val2 / 1000);

            if (event_subsys.evt_batt_undervoltage.triggered)
            {
                if (event_subsys.evt_batt_undervoltage.duration_s >= 30)
                {
                    if (!event_subsys.evt_batt_undervoltage.handled) {
                        event_subsys.evt_batt_undervoltage.handled = true;
                        struct event evt = {
                            .level = EVENT_CRITICAL,
                            .type = EVENT_CRIT_BATTERY_UNDERVOLTAGE,
                            .timestamp = rtc_calibrated_time_get(rtc_dev),
                            .data = value,
                        };
                        zbus_chan_pub(&critical_chan, &evt, K_MSEC(10));
                    }
                }
            }
            else
            {
                event_subsys.evt_batt_undervoltage.triggered = true;
            }

            event_subsys.evt_batt_undervoltage.duration_s += 5;
        }
        else
        {
            event_subsys.evt_batt_undervoltage.triggered = false;
            event_subsys.evt_batt_undervoltage.handled = false;
            event_subsys.evt_batt_undervoltage.duration_s = 0;
        }

        if (atomic_load(&g_chrg_triggered)) {
            k_sleep(K_MSEC(1000));
        } else {
            k_sleep(K_MSEC(SAMPLING_TIMES_MS));
        }
    }
}

void fuel_gauge_thread(void)
{
    const struct device *const dev = DEVICE_DT_GET_ONE(ti_bq274xx);

    fgm_override_buffer_init();

    if (!device_is_ready(dev)) {
        LOG_ERR("Device %s is not ready\n", dev->name);
        return ;
    }

    LOG_INF("Device is %p, name is %s\n", dev, dev->name);

    bq27427_init_complete(dev);
    do_main(dev);
}

K_THREAD_DEFINE(fuel_gauge_thread_id, FULE_GAUGE_STACK_SIZE, fuel_gauge_thread, NULL, NULL,
    NULL, FUEL_GAUGE_PRIORITY, 0, 0);
