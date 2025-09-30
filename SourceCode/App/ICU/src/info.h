/*******************************************************************************
 * @file info.h
 * @brief Device info
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __INFO_H__
#define __INFO_H__

#include <stdint.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "events.h"

//*< firmware version
typedef struct {
    uint8_t x;  //!< major
    uint8_t y;  //!< minor
    uint8_t z;  //!< patch
    uint8_t b;  //!< build
} fw_ver_t;

typedef struct {
    int16_t bat_discharging_curr_limit;
    int16_t bat_charging_curr_limit;
    int16_t titianum_case_temp_soft_limit;
    int16_t titianum_case_temp_hard_limit;
    int16_t board_temp_limit;
    int16_t probe_current_limit;
    int16_t undervoltage_limit;
    int16_t overtemp_lazy_time_limit;
} system_limit_t;

typedef enum {
    LIMIT_BAT_DISCAHRGE_CURR = 0,
    LIMIT_BAT_CHARGE_CURR    = 1,
    LIMIT_CASE_TEMP_SOFT     = 2,
    LIMIT_CASE_TEMP_HARD     = 3,
    LIMIT_BOARD_TEMP         = 4,
    LIMIT_PROBE_CURR         = 5,
    LIMIT_UNDERVOLATGE       = 6,
    LIMIT_OVERTEMP_LAZY_TIME = 7,
} limit_type_t;

struct event_meta {
    uint16_t index;
    uint16_t size;
};

#define DEV_SN_LEN 9
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define CRITICAL_EVENT_MAX_NUM (50)
#define NORMAL_EVENT_MAX_NUM (200)

extern const fw_ver_t g_appcore_version;
extern const fw_ver_t g_netcore_version;
extern char device_name[CONFIG_BT_DEVICE_NAME_MAX];
extern int32_t current_sleep_time;
extern int32_t current_sync_time;
extern system_limit_t system_limits;

extern void get_device_id(uint8_t *mac);
extern void print_bt_address(void);
extern void device_name_load(void);
extern int device_name_get(uint8_t *sn);
extern int device_name_set(const uint8_t *sn);
extern void sleep_time_get(void);
extern int sleep_time_set(int32_t new_time);
extern void sync_time_get(void);
extern int sync_time_set(int32_t new_time);
extern bool validate_icu_name(const char *serial);
extern ssize_t nvs_get_free_space(void);
extern int nvs_event_save(const struct event *evt);
extern int nvs_event_load(struct event *evt, uint16_t index);
extern int nvs_meta_load(event_level lvl, struct event_meta *meta);
extern int nvs_meta_clear(event_level lvl);
extern int16_t sensor_limit_get(limit_type_t type);
extern int sensor_limit_set(limit_type_t type, int16_t value);
extern void system_limits_init(void);
extern void info_init(void);
extern void info_clear(void);

#endif /** __INFO_H__ */
