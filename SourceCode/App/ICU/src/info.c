/*******************************************************************************
 * @file  info.c
 * @brief Device info
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include "info.h"
#include "public_api.h"
#include <ctype.h>
#include <hal/nrf_ficr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#define LOG_MODULE_NAME OPT_INFO
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

const fw_ver_t g_appcore_version = {
    .x = 1,
    .y = 1,
    .z = 0,
    .b = 1,
};

const fw_ver_t g_netcore_version = {
    .x = 1,
    .y = 0,
    .z = 0,
    .b = 1,
};

#define DEFAULT_SLEEP_MS (5 * 60 * 1000)
#define DEFAULT_SYNC_MS  (1000)
#define DEFAULT_DISCHARGING_CURR_LIMIT (100) // mA
#define DEFAULT_CHARGING_CURR_LIMIT    (100) // mA
#define DEFAULT_PROBE_CURR_LIMIT       (30)  // mA
#define DEFAULT_SHELL_TEMP_SOFT_LIMIT  (390) // 0.1 C
#define DEFAULT_SHELL_TEMP_HARD_LIMIT  (410) // 0.1 C
#define DEFAULT_BOARD_TEMP_LIMIT       (410) // 0.1 C
#define DEFAULT_OVERTEMP_LAZY_TIME     (30)  // s
#define DEFAULT_UNDERVOLTAGE_LIMIT     (3200) // mV

#define DISCHARGING_CURR_LOWER_LIMIT   (100)
#define DISCHARGING_CURR_UPPER_LIMIT   (250)
#define CHARGING_CURR_LOWER_LIMIT      (100)
#define CHARGING_CURR_UPPER_LIMIT      (250)
#define PROBE_CURR_LOWER_LIMIT         (16)
#define PROBE_CURR_UPPER_LIMIT         (50)
#define SHELL_TEMP_LOWER_SOFT_LIMIT    (370)
#define SHELL_TEMP_UPPER_SOFT_LIMIT    (410)
#define SHELL_TEMP_LOWER_HARD_LIMIT    (390)
#define SHELL_TEMP_UPPER_HARD_LIMIT    (430)
#define BOARD_TEMP_LOWER_LIMIT         (380)
#define BOARD_TEMP_UPPER_LIMIT         (450)
#define UNDERVOLTAGE_LOWER_LIMIT       (3000)
#define UNDERVOLTAGE_UPPER_LIMIT       (3500)
#define OVERTEMP_LAZY_TIME_UPPER_LIMIT (60)


static struct nvs_fs fs;
#define NVS_PARTITION           nvs_storage
#define NVS_PARTITION_DEVICE    FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET    FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define DEVICE_NAME_ID  1
#define SLEEP_TIME_ID   2
#define STM32_TIME_ID   3
#define LOW_BATTERY_THRESHOLD_ID              10
#define POWER_SUPPLY_TEMP_THRESHOLD_ID        11
#define PROBE_CURRENT_THRESHOLD_ID            12
#define TITANIUM_CASE_TEMP_SOFT_THRESHOLD_ID  13
#define TITANIUM_CASE_TEMP_HARD_THRESHOLD_ID  14
#define BATT_CHARGING_CURR_THRESHOLD_ID       15
#define BATT_DISCHARGING_CURR_THRESHOLD_ID    16
#define OVERTEMP_TIME_THRESHOLD_ID            17
#define CRITICAL_EVENT_INDEX_KEY_ID           100
#define CRITICAL_EVENT_BASE_ID                101
#define NORMAL_EVENT_INDEX_KEY_ID             200
#define NORMAL_EVENT_BASE_ID                  201


char device_name[CONFIG_BT_DEVICE_NAME_MAX];
int32_t current_sleep_time = DEFAULT_SLEEP_MS;
int32_t current_sync_time = DEFAULT_SYNC_MS;

//!< Sensor limits
system_limit_t system_limits = {
    .bat_discharging_curr_limit = DEFAULT_DISCHARGING_CURR_LIMIT,
    .bat_charging_curr_limit = DEFAULT_CHARGING_CURR_LIMIT,
    .titianum_case_temp_soft_limit = DEFAULT_SHELL_TEMP_SOFT_LIMIT,
    .titianum_case_temp_hard_limit = DEFAULT_SHELL_TEMP_HARD_LIMIT,
    .board_temp_limit = DEFAULT_BOARD_TEMP_LIMIT,
    .probe_current_limit = DEFAULT_PROBE_CURR_LIMIT,
    .undervoltage_limit = DEFAULT_UNDERVOLTAGE_LIMIT,
};

void get_device_id(uint8_t *mac)
{
    uint32_t device_id[2] = {0};

    device_id[0] = nrf_ficr_deviceid_get(NRF_FICR, 0);
    device_id[1] = nrf_ficr_deviceid_get(NRF_FICR, 1);

    mac[0] = (device_id[0] >> 24) & 0xFF;
    mac[1] = (device_id[0] >> 16) & 0xFF;
    mac[2] = (device_id[0] >> 8) & 0xFF;
    mac[3] = (device_id[0]) & 0xFF;
    mac[4] = (device_id[1] >> 24) & 0xFF;
    mac[5] = (device_id[1] >> 16) & 0xFF;
    mac[6] = (device_id[1] >> 8) & 0xFF;
    mac[7] = (device_id[1]) & 0xFF;
}

void print_bt_address(void)
{
    bt_addr_le_t addrs[CONFIG_BT_ID_MAX];

    size_t count = ARRAY_SIZE(addrs);

    bt_id_get(addrs, &count);

    for (size_t i = 0; i < count; i++) {
        char str[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(&addrs[i], str, sizeof(str));
        LOG_INF("Bluetooth address %zu: %s", i, str);
    }
}

/**
 * @brief Validate ICU serial number format manually.
 * @details Format: OPDD L 0001 (OP + D + YearCode + MonthCode + 4 digits)
 *      Rules:
 *      - Starts with "OP"
 *      - 3rd char is 'D/P' (pre-production/post-production)
 *      - 4th char is A-Z (year code, 2021=A, 2024=D, etc.)
 *      - 5th char is A-L (month code, 1=A, 12=L)
 *      - Ends with 4 digits
 * 
 * @param serial Input serial string
 * @return true if valid, false otherwise
 */
bool validate_icu_name(const char *serial)
{
    // Validate "OP" prefix
    if (serial[0] != 'O' || serial[1] != 'P') {
        return false;
    }

    // Check production symbol 'D'
    if (serial[2] != 'D' && serial[2] != 'P') {
        return false;
    }

    // Validate year code (A-Z)
    if (serial[3] < 'A' || serial[3] > 'Z') {
        return false;
    }

    // Validate month code (A-L)
    if (serial[4] < 'A' || serial[4] > 'L') {
        return false;
    }

    // Validate 4-digit numeric suffix
    for (int i = 5; i < 9; i++) {
        if (!isdigit((unsigned char)serial[i])) {
            return false;
        }
    }

    return true;
}

void device_name_load(void)
{
    int rc = nvs_read(&fs, DEVICE_NAME_ID, device_name, sizeof(device_name));
    if (rc <= 0) {
        printk("No stored name, using default\n");
        strncpy(device_name, DEVICE_NAME, sizeof(device_name));
        device_name[sizeof(device_name)-1] = '\0';
        rc = nvs_write(&fs, DEVICE_NAME_ID, device_name, strlen(device_name)+1);
        if (rc < 0) {
            printk("Error saving default name: %d\n", rc);
        }
    }
    else {
        printk("device_name_load: %s\n", device_name);
    }
    char adv_name[CONFIG_BT_DEVICE_NAME_MAX + 4] = "ICU_";
    size_t available = sizeof(adv_name) - strlen(adv_name) - 1;
    strncat(adv_name, device_name, available);
    bt_set_name(adv_name);
}

void sleep_time_get(void)
{
    int err = nvs_read(&fs, SLEEP_TIME_ID, &current_sleep_time, sizeof(current_sleep_time));
    if (err <= 0 || (current_sleep_time < 60 * 1000 || current_sleep_time > 60 * 60 * 1000)) {
        current_sleep_time = DEFAULT_SLEEP_MS;
        nvs_write(&fs, SLEEP_TIME_ID, &current_sleep_time, sizeof(current_sleep_time));
    }
}

int sleep_time_set(int32_t new_time)
{
    if (new_time < 60 * 1000 || new_time > 3600000) {
        return -EINVAL;
    }

    int err = nvs_write(&fs, SLEEP_TIME_ID, &new_time, sizeof(new_time));
    if (err >= 0) {
        current_sleep_time = new_time;
        k_timer_stop(&sleep_timer);
        k_timer_start(&sleep_timer, K_MSEC(current_sleep_time), K_MSEC(current_sleep_time));
    }
    return err;
}

void sync_time_get(void)
{
    int err = nvs_read(&fs, STM32_TIME_ID, &current_sync_time, sizeof(current_sync_time));
    if (err <= 0 || (current_sync_time < 1000 || current_sync_time > 60 * 60 * 1000)) {
        current_sync_time = DEFAULT_SYNC_MS;
        nvs_write(&fs, STM32_TIME_ID, &current_sync_time, sizeof(current_sync_time));
    }
}

int sync_time_set(int32_t new_time)
{
    if (new_time < 1000 || new_time > 3600000) {
        return -EINVAL;
    }

    int err = nvs_write(&fs, STM32_TIME_ID, &new_time, sizeof(new_time));
    if (err >= 0) {
        current_sync_time = new_time;
        k_timer_stop(&stm_heartbeat_timer);
        k_timer_start(&stm_heartbeat_timer, K_MSEC(current_sync_time), K_MSEC(current_sync_time));
    }
    return err;
}

int device_name_get(uint8_t *sn)
{
    int err = 0;
    char sn_buf[DEV_SN_LEN + 1] = {0};
    err = nvs_read(&fs, DEVICE_NAME_ID, &sn_buf, DEV_SN_LEN);
    if (err > 0) { /* item was found, show it */
        printk("Id: %d, Device_Name: %s\n", DEVICE_NAME_ID, sn_buf);
        memcpy(sn, sn_buf, DEV_SN_LEN);
    }
    return err;
}

int device_name_set(const uint8_t *sn)
{
    int err = 0;
    err = nvs_write(&fs, DEVICE_NAME_ID, sn, DEV_SN_LEN);
    return err;
}

ssize_t nvs_get_free_space(void)
{
    ssize_t free = nvs_calc_free_space(&fs);
    if (free >= 0) {
        LOG_INF("NVS free space: %zd B (%0.1fKB)", free, (double)free / 1024);
    } else {
        LOG_ERR("NVS Get free space fail");
    }
    return free;
}

int nvs_event_save(const struct event *evt)
{
    LOG_DBG("event_save: %d, %d", evt->level, evt->type);

    int ret;

    static struct event_meta meta;
    // Read meta data
    if (evt->level == EVENT_CRITICAL) {
        ret = nvs_read(&fs, CRITICAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        if (ret < 0) {
            meta.index = 0;
            meta.size = 0;
            nvs_write(&fs, CRITICAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
            LOG_ERR("read critical event meta failed");
        }
    }
    else if (evt->level == EVENT_NORMAL) {
        ret = nvs_read(&fs, NORMAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        if (ret < 0) {
            meta.index = 0;
            meta.size = 0;
            nvs_write(&fs, NORMAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
            LOG_ERR("read normal event meta failed");
        }
    }
    else {
        return -1;
    }

    // Write event data
    if (evt->level == EVENT_CRITICAL) {
        ret = nvs_write(&fs, CRITICAL_EVENT_BASE_ID + meta.index, evt, sizeof(struct event));
        if (ret == 0 || ret == sizeof(struct event)) {
            LOG_DBG("nvs_write: %d", ret);
            meta.index = (meta.index + 1) % CRITICAL_EVENT_MAX_NUM;
            if (meta.size < CRITICAL_EVENT_MAX_NUM) {
                meta.size++;
            }
            nvs_write(&fs, CRITICAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        }
        else
        {
            LOG_ERR("critical event save fail");
        }
    }
    else if (evt->level == EVENT_NORMAL) {
        ret = nvs_write(&fs, NORMAL_EVENT_BASE_ID + meta.index, evt, sizeof(struct event));
        if (ret == 0 || ret == sizeof(struct event)) {
            LOG_DBG("nvs_write: %d", ret);
            meta.index = (meta.index + 1) % NORMAL_EVENT_MAX_NUM;
            if (meta.size < NORMAL_EVENT_MAX_NUM) {
                meta.size++;
            }
            nvs_write(&fs, NORMAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        }
        else
        {
            LOG_ERR("normal event save fail");
        }
    }
    else {
        return -1;
    }

    return 0;
}

int nvs_meta_load(event_level lvl, struct event_meta *meta)
{
    int ret;

    if (lvl == EVENT_CRITICAL)
    {
        ret = nvs_read(&fs, CRITICAL_EVENT_INDEX_KEY_ID, meta, sizeof(struct event_meta));
    }
    else if (lvl == EVENT_NORMAL)
    {
        ret = nvs_read(&fs, NORMAL_EVENT_INDEX_KEY_ID, meta, sizeof(struct event_meta));
    }
    else {
        ret = -1;
    }

    return ret;
}

int nvs_meta_clear(event_level lvl)
{
    int ret;
    struct event_meta meta;

    if (lvl == EVENT_CRITICAL) {
        meta.index = 0;
        meta.size = 0;
        ret = nvs_write(&fs, CRITICAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        if (ret < 0) {
            LOG_ERR("Clear cirtical meta failed (%d)", ret);
            return ret;
        }
    }

    if (lvl == EVENT_NORMAL) {
        meta.index = 0;
        meta.size = 0;
        ret = nvs_write(&fs, NORMAL_EVENT_INDEX_KEY_ID, &meta, sizeof(struct event_meta));
        if (ret < 0) {
            LOG_ERR("Clear normal meta failed (%d)", ret);
            return ret;
        }
    }

    return 0;
}

int nvs_event_load(struct event *evt, uint16_t index)
{
    int ret;

    if (evt->level == EVENT_CRITICAL) {
        ret = nvs_read(&fs, CRITICAL_EVENT_BASE_ID + index, evt, sizeof(struct event));
    }
    else if (evt->level == EVENT_NORMAL) {
        ret = nvs_read(&fs, NORMAL_EVENT_BASE_ID + index, evt, sizeof(struct event));
    }
    else {
        return -1;
    }

    return ret;
}

/**
 * @brief get the sampling data limits of sensor
 * 
 * @param type 
 * @return int16_t 
 */
int16_t sensor_limit_get(limit_type_t type)
{
    int err = 0;
    int16_t value = 0;

    switch (type) {
        case LIMIT_BAT_DISCAHRGE_CURR:
            err = nvs_read(&fs, BATT_DISCHARGING_CURR_THRESHOLD_ID,
                                &system_limits.bat_discharging_curr_limit,
                                sizeof(system_limits.bat_discharging_curr_limit));
            if (err <= 0) {
                system_limits.bat_discharging_curr_limit = DEFAULT_DISCHARGING_CURR_LIMIT;
                nvs_write(&fs, BATT_DISCHARGING_CURR_THRESHOLD_ID,
                               &system_limits.bat_discharging_curr_limit,
                               sizeof(system_limits.bat_discharging_curr_limit));
            }
            value = system_limits.bat_discharging_curr_limit;
            break;
        case LIMIT_BAT_CHARGE_CURR:
            err = nvs_read(&fs, BATT_CHARGING_CURR_THRESHOLD_ID,
                                &system_limits.bat_charging_curr_limit,
                                sizeof(system_limits.bat_charging_curr_limit));
            if (err <= 0) {
                system_limits.bat_charging_curr_limit = DEFAULT_CHARGING_CURR_LIMIT;
                nvs_write(&fs, BATT_CHARGING_CURR_THRESHOLD_ID,
                               &system_limits.bat_charging_curr_limit,
                               sizeof(system_limits.bat_charging_curr_limit));
            }
            value = system_limits.bat_charging_curr_limit;
            break;
        case LIMIT_CASE_TEMP_SOFT:
            err = nvs_read(&fs, TITANIUM_CASE_TEMP_SOFT_THRESHOLD_ID,
                                &system_limits.titianum_case_temp_soft_limit,
                                sizeof(system_limits.titianum_case_temp_soft_limit));
            if (err <= 0) {
                system_limits.titianum_case_temp_soft_limit = DEFAULT_SHELL_TEMP_SOFT_LIMIT;
                nvs_write(&fs, TITANIUM_CASE_TEMP_SOFT_THRESHOLD_ID,
                               &system_limits.titianum_case_temp_soft_limit,
                               sizeof(system_limits.titianum_case_temp_soft_limit));
            }
            value = system_limits.titianum_case_temp_soft_limit;
            break;
        case LIMIT_CASE_TEMP_HARD:
            err = nvs_read(&fs, TITANIUM_CASE_TEMP_HARD_THRESHOLD_ID,
                                &system_limits.titianum_case_temp_hard_limit,
                                sizeof(system_limits.titianum_case_temp_hard_limit));
            if (err <= 0) {
                system_limits.titianum_case_temp_hard_limit = DEFAULT_SHELL_TEMP_HARD_LIMIT;
                nvs_write(&fs, TITANIUM_CASE_TEMP_HARD_THRESHOLD_ID,
                               &system_limits.titianum_case_temp_hard_limit,
                               sizeof(system_limits.titianum_case_temp_hard_limit));
            }
            value = system_limits.titianum_case_temp_hard_limit;
            break;
        case LIMIT_BOARD_TEMP:
            err = nvs_read(&fs, POWER_SUPPLY_TEMP_THRESHOLD_ID,
                                &system_limits.board_temp_limit,
                                sizeof(system_limits.board_temp_limit));
            if (err <= 0) {
                system_limits.board_temp_limit = DEFAULT_BOARD_TEMP_LIMIT;
                nvs_write(&fs, POWER_SUPPLY_TEMP_THRESHOLD_ID,
                               &system_limits.board_temp_limit,
                               sizeof(system_limits.board_temp_limit));
            }
            value = system_limits.board_temp_limit;
            break;
        case LIMIT_PROBE_CURR:
            err = nvs_read(&fs, PROBE_CURRENT_THRESHOLD_ID,
                                &system_limits.probe_current_limit,
                                sizeof(system_limits.probe_current_limit));
            if (err <= 0) {
                system_limits.probe_current_limit = DEFAULT_PROBE_CURR_LIMIT;
                nvs_write(&fs, PROBE_CURRENT_THRESHOLD_ID,
                               &system_limits.probe_current_limit,
                               sizeof(system_limits.probe_current_limit));
            }
            value = system_limits.probe_current_limit;
            break;
        case LIMIT_UNDERVOLATGE:
            err = nvs_read(&fs, LOW_BATTERY_THRESHOLD_ID,
                                &system_limits.undervoltage_limit,
                                sizeof(system_limits.undervoltage_limit));
            if (err <= 0) {
                system_limits.undervoltage_limit = DEFAULT_UNDERVOLTAGE_LIMIT;
                nvs_write(&fs, LOW_BATTERY_THRESHOLD_ID,
                               &system_limits.undervoltage_limit,
                               sizeof(system_limits.undervoltage_limit));
            }
            value = system_limits.undervoltage_limit;
            break;
        case LIMIT_OVERTEMP_LAZY_TIME:
            err = nvs_read(&fs, OVERTEMP_TIME_THRESHOLD_ID,
                                &system_limits.overtemp_lazy_time_limit,
                                sizeof(system_limits.overtemp_lazy_time_limit));
            if (err <= 0) {
                system_limits.overtemp_lazy_time_limit = DEFAULT_OVERTEMP_LAZY_TIME;
                nvs_write(&fs, OVERTEMP_TIME_THRESHOLD_ID,
                               &system_limits.overtemp_lazy_time_limit,
                               sizeof(system_limits.overtemp_lazy_time_limit));
            }
            value = system_limits.overtemp_lazy_time_limit;
            break;
        default:
            break;
    }

    return value;
}

/**
 * @brief set the sampling data limits of sensor
 * 
 * @param type 
 * @return int 
 */
int sensor_limit_set(limit_type_t type, int16_t value)
{
    int err = 0;

    switch (type) {
        case LIMIT_BAT_DISCAHRGE_CURR:
            if (value < DISCHARGING_CURR_LOWER_LIMIT || value > DISCHARGING_CURR_UPPER_LIMIT)
            {
                value = DEFAULT_DISCHARGING_CURR_LIMIT;
            }
            system_limits.bat_discharging_curr_limit = value;
            err = nvs_write(&fs, BATT_DISCHARGING_CURR_THRESHOLD_ID,
                                 &system_limits.bat_discharging_curr_limit,
                                 sizeof(system_limits.bat_discharging_curr_limit));
            break;
        case LIMIT_BAT_CHARGE_CURR:
            if (value < CHARGING_CURR_LOWER_LIMIT || value > CHARGING_CURR_UPPER_LIMIT)
            {
                value = DEFAULT_CHARGING_CURR_LIMIT;
            }
            system_limits.bat_charging_curr_limit = value;
            err = nvs_write(&fs, BATT_CHARGING_CURR_THRESHOLD_ID,
                                 &system_limits.bat_charging_curr_limit,
                                 sizeof(system_limits.bat_charging_curr_limit));
            break;
        case LIMIT_CASE_TEMP_SOFT:
            if (value < SHELL_TEMP_LOWER_SOFT_LIMIT || value > SHELL_TEMP_UPPER_SOFT_LIMIT)
            {
                value = DEFAULT_SHELL_TEMP_SOFT_LIMIT;
            }
            system_limits.titianum_case_temp_soft_limit = value;
            err = nvs_write(&fs, TITANIUM_CASE_TEMP_SOFT_THRESHOLD_ID,
                                 &system_limits.titianum_case_temp_soft_limit,
                                 sizeof(system_limits.titianum_case_temp_soft_limit));
            break;
        case LIMIT_CASE_TEMP_HARD:
            if (value < SHELL_TEMP_LOWER_HARD_LIMIT || value > SHELL_TEMP_UPPER_HARD_LIMIT)
            {
                value = DEFAULT_SHELL_TEMP_HARD_LIMIT;
            }
            system_limits.titianum_case_temp_hard_limit = value;
            err = nvs_write(&fs, TITANIUM_CASE_TEMP_HARD_THRESHOLD_ID,
                                 &system_limits.titianum_case_temp_hard_limit,
                                 sizeof(system_limits.titianum_case_temp_hard_limit));
            break;
        case LIMIT_BOARD_TEMP:
            if (value < BOARD_TEMP_LOWER_LIMIT || value > BOARD_TEMP_UPPER_LIMIT)
            {
                value = DEFAULT_BOARD_TEMP_LIMIT;
            }
            system_limits.board_temp_limit = value;
            err = nvs_write(&fs, POWER_SUPPLY_TEMP_THRESHOLD_ID,
                                 &system_limits.board_temp_limit,
                                 sizeof(system_limits.board_temp_limit));
            break;
        case LIMIT_PROBE_CURR:
            if (value < PROBE_CURR_LOWER_LIMIT || value > PROBE_CURR_UPPER_LIMIT)
            {
                value = DEFAULT_PROBE_CURR_LIMIT;
            }
            system_limits.probe_current_limit = value;
            err = nvs_write(&fs, PROBE_CURRENT_THRESHOLD_ID,
                                 &system_limits.probe_current_limit,
                                 sizeof(system_limits.probe_current_limit));
            break;
        case LIMIT_UNDERVOLATGE:
            if (value < UNDERVOLTAGE_LOWER_LIMIT || value > UNDERVOLTAGE_UPPER_LIMIT)
            {
                value = DEFAULT_UNDERVOLTAGE_LIMIT;
            }
            system_limits.undervoltage_limit = value;
            err = nvs_write(&fs, LOW_BATTERY_THRESHOLD_ID,
                                 &system_limits.undervoltage_limit,
                                 sizeof(system_limits.undervoltage_limit));
            break;
        case LIMIT_OVERTEMP_LAZY_TIME:
                if (value > OVERTEMP_LAZY_TIME_UPPER_LIMIT)
                {
                    value = DEFAULT_OVERTEMP_LAZY_TIME;
                }
                system_limits.overtemp_lazy_time_limit = value;
                err = nvs_write(&fs, OVERTEMP_TIME_THRESHOLD_ID,
                                 &system_limits.overtemp_lazy_time_limit,
                                 sizeof(system_limits.overtemp_lazy_time_limit));
            break;
        default:
            break;
    }

    return err;
}

/**
 * @brief Initialize system limits
 * 
 */
void system_limits_init(void)
{
    int16_t limit_value;

    limit_value = sensor_limit_get(LIMIT_BAT_DISCAHRGE_CURR);
    if (limit_value < DISCHARGING_CURR_LOWER_LIMIT || limit_value > DISCHARGING_CURR_UPPER_LIMIT)
    {
        limit_value = DEFAULT_DISCHARGING_CURR_LIMIT;
    }
    system_limits.bat_discharging_curr_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_BAT_CHARGE_CURR);
    if (limit_value < CHARGING_CURR_LOWER_LIMIT || limit_value > CHARGING_CURR_UPPER_LIMIT)
    {
        limit_value = DEFAULT_CHARGING_CURR_LIMIT;
    }
    system_limits.bat_charging_curr_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_CASE_TEMP_SOFT);
    if (limit_value < SHELL_TEMP_LOWER_SOFT_LIMIT || limit_value > SHELL_TEMP_UPPER_SOFT_LIMIT)
    {
        limit_value = DEFAULT_SHELL_TEMP_SOFT_LIMIT;
    }
    system_limits.titianum_case_temp_soft_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_CASE_TEMP_HARD);
    if (limit_value < SHELL_TEMP_LOWER_HARD_LIMIT || limit_value > SHELL_TEMP_UPPER_HARD_LIMIT)
    {
        limit_value = DEFAULT_SHELL_TEMP_HARD_LIMIT;
    }
    system_limits.titianum_case_temp_hard_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_BOARD_TEMP);
    if (limit_value < BOARD_TEMP_LOWER_LIMIT || limit_value > BOARD_TEMP_UPPER_LIMIT)
    {
        limit_value = DEFAULT_BOARD_TEMP_LIMIT;
    }
    system_limits.board_temp_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_PROBE_CURR);
    if (limit_value < PROBE_CURR_LOWER_LIMIT || limit_value > PROBE_CURR_UPPER_LIMIT)
    {
        limit_value = DEFAULT_PROBE_CURR_LIMIT;
    }
    system_limits.probe_current_limit = limit_value;

    limit_value = sensor_limit_get(LIMIT_UNDERVOLATGE);
    if (limit_value < UNDERVOLTAGE_LOWER_LIMIT || limit_value > UNDERVOLTAGE_UPPER_LIMIT)
    {
        limit_value = DEFAULT_UNDERVOLTAGE_LIMIT;
    }
    system_limits.undervoltage_limit = limit_value;

    LOG_INF("System Limits: discharing_current (%d), charging_current (%d), case_temp (%d-%d), board_temp (%d), probe_current (%d), undervolatge (%d)",
                            system_limits.bat_discharging_curr_limit,
                            system_limits.bat_charging_curr_limit,
                            system_limits.titianum_case_temp_soft_limit,
                            system_limits.titianum_case_temp_hard_limit,
                            system_limits.board_temp_limit,
                            system_limits.probe_current_limit,
                            system_limits.undervoltage_limit);
}

void info_init(void)
{
    int err = 0;
    struct flash_pages_info info;
    fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(fs.flash_device)) {
        printk("Flash device %s is not ready\n", fs.flash_device->name);
        return ;
    }
    fs.offset = NVS_PARTITION_OFFSET;
    err = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (err) {
        printk("Unable to get page info\n");
        return ;
    }
    printk("size = %d \n", info.size);
    fs.sector_size = info.size;
    fs.sector_count = 4U;

    err = nvs_mount(&fs);
    if (err) {
        printk("NVS mount failed\n");
        return ;
    }
}

void info_clear(void)
{
    int err = 0;
    struct flash_pages_info info;
    fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(fs.flash_device)) {
        printk("Flash device %s is not ready\n", fs.flash_device->name);
        return ;
    }
    fs.offset = NVS_PARTITION_OFFSET;
    err = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (err) {
        printk("Unable to get page info\n");
        return ;
    }
    printk("size = %d \n", info.size);
    fs.sector_size = info.size;
    fs.sector_count = 4U;

    err = nvs_clear(&fs);
    if (err) {
        printk("NVS Clear failed\n");
        return ;
    }
    else
    {
        err = nvs_mount(&fs);
        if (err) {
            printk("NVS remount failed\n");
            return ;
        }
    }
}