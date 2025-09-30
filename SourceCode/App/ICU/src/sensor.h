/*******************************************************************************
 * @file sensor.h
 * @brief Sensor
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#define TEMP_UPDATE_EVENT 0x01
#define TEMP_ITEM_SIZE sizeof(struct sensor_value)
#define CURR_UPDATE_EVENT 0x01
#define CURR_ITEM_SIZE sizeof(int32_t)
#define STM32_UPDATE_EVENT 0x01
#define STM32_SENSOR_ITEM_SIZE sizeof(struct stm32_sensor_data)
#define FUEL_GAUGE_UPDATE_EVENT 0x01
#define FUEL_GAUGE_ITEM_SIZE sizeof(struct fuel_gauge_data)

/**
 * @brief sensor type of STM32
 */
typedef enum {
    SENSOR_SHELL_TEMP = 0,
    SENSOR_BAT_CURR = 1,
    SENSOR_BAT_VOLT = 2,
    SENSOR_PTX_VOLT = 3,
} sensor_stm32_t;

typedef enum {
    FG_VOLTAGE = 0,
    FG_CURRENT = 1,
    FG_STATE_OF_CHARGE = 2,
    FG_STATE_OF_HEALTH = 3,
} fuel_gauge_chan_t;


/**
 * @brief Sampling data in STM32
 */
struct stm32_sensor_data {
    uint16_t shell_temp;
    uint16_t batt_volt;
    uint16_t ptx_volt;
    int16_t batt_curr;
};

/**
 * @brief Sampling data of fuel gauge (bq274xx)
 */
struct fuel_gauge_data {
    struct sensor_value voltage;
    struct sensor_value current;
    struct sensor_value state_of_charge;
    struct sensor_value state_of_health;
};

extern struct k_mutex i2c1_bus_lock;

//!< temperature collection related variables
extern struct ring_buf temp_ring_buf;
extern struct k_mutex temp_mutex;
extern struct k_event temp_event;
//!< fuel gauge collection related variables
extern struct ring_buf fuel_gauge_ring_buf;
extern struct k_mutex fuel_gauge_mutex;
extern struct k_event fuel_gauge_event;

extern void override_buffer_init(void);
extern void smart_buffer_write(const struct stm32_sensor_data *value);
extern void power_temp_get(uint8_t *temp_i, uint8_t *temp_d);
extern int8_t probe_current_get(void);
extern uint16_t stm32_sensor_data_get(sensor_stm32_t type);
extern struct sensor_value fuel_gauge_value_get(fuel_gauge_chan_t chan);

#endif /** __SENSOR_H__ */
