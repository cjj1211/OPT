/*******************************************************************************
 * @file  sensor.c
 * @brief Sensor data read interface
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include "sensor.h"
#include "info.h"
#include "events.h"
#include "rtc.h"
#include "public_api.h"
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/drivers/adc.h>

#define LOG_MODULE_NAME OPT_SENSOR
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

//!< Avoid conflicts between the bq27427 and tmp119 read/write operations
K_MUTEX_DEFINE(i2c1_bus_lock);

//!< Store the temperature of power supply
struct ring_buf temp_ring_buf;
//!< Store the probe current
struct ring_buf curr_ring_buf;
//!< Store the sensor data obtained from STM32
struct ring_buf stm32_sensor_ring_buf;
//!< Store the data read from the fuel gauge
struct ring_buf fuel_gauge_ring_buf;

struct k_mutex temp_mutex;
struct k_mutex curr_mutex;
struct k_mutex stm32_sensor_mutex;
struct k_mutex fuel_gauge_mutex;

//!< Event Notify
struct k_event temp_event;
struct k_event curr_event;
struct k_event stm32_sensor_event;
struct k_event fuel_gauge_event;

#define STM32_BUFFER_ITEMS 2
static uint8_t stm32_buffer[STM32_SENSOR_ITEM_SIZE * STM32_BUFFER_ITEMS];

#define CURR_BUFFER_ITEMS 2
static uint8_t curr_buffer[CURR_ITEM_SIZE * CURR_BUFFER_ITEMS];

//!< Thread Config
#define CURRENT_MONITOR_PRIORITY    7
#define CURRENT_MONITOR_STACK_SIZE  1024

void override_buffer_init(void)
{
    ring_buf_init(&stm32_sensor_ring_buf, sizeof(stm32_buffer), stm32_buffer);
    k_mutex_init(&stm32_sensor_mutex);
    k_event_init(&stm32_sensor_event);
}

/**
 * @brief Probe Current buffer
 */
static void pc_override_buffer_init(void)
{
    ring_buf_init(&curr_ring_buf, sizeof(curr_buffer), curr_buffer);
    k_mutex_init(&curr_mutex);
    k_event_init(&curr_event);
}

/**
 * @brief override write function
 */
void smart_buffer_write(const struct stm32_sensor_data *value)
{
    k_mutex_lock(&stm32_sensor_mutex, K_FOREVER);

    //!< Dynamic override
    while (ring_buf_space_get(&stm32_sensor_ring_buf) < STM32_SENSOR_ITEM_SIZE) {
        uint8_t dummy[STM32_SENSOR_ITEM_SIZE];
        ring_buf_get(&stm32_sensor_ring_buf, dummy, STM32_SENSOR_ITEM_SIZE);
    }

    ring_buf_put(&stm32_sensor_ring_buf, (uint8_t *)value, STM32_SENSOR_ITEM_SIZE);

    //!< Notify data ready
    k_event_set(&stm32_sensor_event, STM32_UPDATE_EVENT);

    k_mutex_unlock(&stm32_sensor_mutex);
}

/**
 * @brief override write function for Probe Current
 */
void pc_smart_buffer_write(const int32_t value)
{
    k_mutex_lock(&curr_mutex, K_FOREVER);

    //!< Dynamic override
    while (ring_buf_space_get(&curr_ring_buf) < CURR_ITEM_SIZE) {
        uint8_t dummy[CURR_ITEM_SIZE];
        ring_buf_get(&curr_ring_buf, dummy, CURR_ITEM_SIZE);
    }

    ring_buf_put(&curr_ring_buf, (uint8_t *)&value, CURR_ITEM_SIZE);

    //!< Notify data ready
    k_event_set(&curr_event, CURR_UPDATE_EVENT);

    k_mutex_unlock(&curr_mutex);
}

void power_temp_get(uint8_t *temp_i, uint8_t *temp_d)
{
    struct sensor_value latest;
    uint32_t events;
    bool data_valid = false;

    events = k_event_wait(&temp_event, TEMP_UPDATE_EVENT, false, K_MSEC(1200));

    k_mutex_lock(&temp_mutex, K_FOREVER);

    if (ring_buf_peek(&temp_ring_buf, (uint8_t *)&latest, TEMP_ITEM_SIZE) == TEMP_ITEM_SIZE) {
        data_valid = true;
    }

    k_mutex_unlock(&temp_mutex);

    if (data_valid) {
        *temp_i = (uint8_t)latest.val1;
        *temp_d = (uint8_t)(latest.val2 / 100000);
    } else {
        *temp_i = 0;
        *temp_d = 0;
    }
    LOG_DBG("power_temp_get: %d.%d", *temp_i, *temp_d);
}

int8_t probe_current_get(void)
{
    int32_t latest;
    uint32_t events;
    bool data_valid = false;

    events = k_event_wait(&curr_event, CURR_UPDATE_EVENT, false, K_MSEC(1200));

    k_mutex_lock(&curr_mutex, K_FOREVER);

    if (ring_buf_peek(&curr_ring_buf, (uint8_t *)&latest, CURR_ITEM_SIZE) == CURR_ITEM_SIZE) {
        data_valid = true;
    }

    k_mutex_unlock(&curr_mutex);

    if (data_valid) {
        return (latest / 20);
    } else {
        return 0;
    }
}

struct sensor_value fuel_gauge_value_get(fuel_gauge_chan_t chan)
{
    struct fuel_gauge_data latest;
    struct sensor_value value = {0};
    uint32_t events;
    bool data_valid = false;

    events = k_event_wait(&fuel_gauge_event, FUEL_GAUGE_UPDATE_EVENT, false, K_MSEC(1200));

    k_mutex_lock(&fuel_gauge_mutex, K_FOREVER);

    if (ring_buf_peek(&fuel_gauge_ring_buf, (uint8_t *)&latest, FUEL_GAUGE_ITEM_SIZE) == FUEL_GAUGE_ITEM_SIZE) {
        data_valid = true;
    }

    k_mutex_unlock(&fuel_gauge_mutex);

    if (data_valid)
    {
        switch (chan) {
        case FG_VOLTAGE:
            value = latest.voltage;
            break;
        case FG_CURRENT:
            value = latest.current;
            break;
        case FG_STATE_OF_CHARGE:
            value = latest.state_of_charge;
            break;
        case FG_STATE_OF_HEALTH:
            value = latest.state_of_health;
            break;
        default:
            value.val1 = 0;
            value.val2 = 0;
            break;
        }
    }

    return value;
}

uint16_t stm32_sensor_data_get(sensor_stm32_t type)
{
    struct stm32_sensor_data latest;
    uint16_t sensor_value = 0;
    uint32_t events;
    bool data_valid = false;

    events = k_event_wait(&stm32_sensor_event, STM32_UPDATE_EVENT, false, K_MSEC(1200));

    k_mutex_lock(&stm32_sensor_mutex, K_FOREVER);

    if (ring_buf_peek(&stm32_sensor_ring_buf, (uint8_t *)&latest, STM32_SENSOR_ITEM_SIZE) == STM32_SENSOR_ITEM_SIZE) {
        data_valid = true;
    }

    k_mutex_unlock(&stm32_sensor_mutex);

    if (data_valid) {
        switch (type) {
        case SENSOR_SHELL_TEMP:
            sensor_value = latest.shell_temp;
            break;
        case SENSOR_BAT_CURR:
            sensor_value = latest.batt_curr;
            break;
        case SENSOR_BAT_VOLT:
            sensor_value = latest.batt_volt;
            break;
        case SENSOR_PTX_VOLT:
            sensor_value = latest.ptx_volt;
            break;
        default:
            sensor_value = 0;
            break;
        }
    } else {
        sensor_value = 0;
    }

    return sensor_value;
}

void current_monitor_thread(void)
{
    int err = 0;
    int16_t buf;

    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };

    pc_override_buffer_init();

    /* Don't go any further until ADC is initialized */
    k_sem_take(&sem_adc_rdy, K_FOREVER);

    while (1) {
        (void)adc_sequence_init_dt(&adc_channels[0], &sequence);
        err = adc_read_dt(&adc_channels[0], &sequence);
        if (err < 0)
        {
            LOG_ERR("Could not read (%d)", err);
        }
        else
        {
            int32_t val_mv = (int32_t)buf;
            err = adc_raw_to_millivolts_dt(&adc_channels[0], &val_mv);
            if (err < 0)
            {
                LOG_ERR(" (Value in mV not available)");
            }
            else
            {
                pc_smart_buffer_write(val_mv);
                LOG_DBG("ProbeCurrent: %d, %d", val_mv, (uint8_t)(val_mv / 20));
                if ((uint8_t)(val_mv / 20) >= system_limits.probe_current_limit)
                // if (val_mv >= 5)
                {
                    if (event_subsys.evt_probe_overcurr.triggered)
                    {
                        event_subsys.evt_probe_overcurr.duration_s += 100;

                        if (event_subsys.evt_probe_overcurr.duration_s >= 1300)
                        {
                            if (!event_subsys.evt_probe_overcurr.handled) {
                                event_subsys.evt_probe_overcurr.handled = true;
                                struct event evt = {
                                    .level = EVENT_CRITICAL,
                                    .type = EVENT_CRIT_PROBE_OVERCURR,
                                    .timestamp = rtc_calibrated_time_get(rtc_dev),
                                    .data = val_mv,
                                };
                                zbus_chan_pub(&critical_chan, &evt, K_MSEC(10));
                            }
                        }
                        else
                        {
                            k_sleep(K_MSEC(100));
                            continue;
                        }
                    }
                    else
                    {
                        event_subsys.evt_probe_overcurr.triggered = true;
                        event_subsys.evt_probe_overcurr.duration_s = 1000;
                    }
                }
                else
                {
                    event_subsys.evt_probe_overcurr.triggered = false;
                    event_subsys.evt_probe_overcurr.handled = false;
                    event_subsys.evt_probe_overcurr.duration_s = 0;
                }
            }
        }

        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(current_monitor_thread_id, CURRENT_MONITOR_STACK_SIZE, current_monitor_thread, NULL, NULL,
        NULL, CURRENT_MONITOR_PRIORITY, 0, 0);