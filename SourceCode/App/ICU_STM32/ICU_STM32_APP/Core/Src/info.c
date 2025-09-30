/**
 * @file info.c
 * @author chunxiao.dai@neuroxess.com
 * @brief System info
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (C) 2025 NeuroXess
 *
 */

#include "info.h"
#include "main.h"
#include <string.h>
#include "usart.h"

/******************************************************************************
 * @defgroup Variables
 *****************************************************************************/
struct emer_event g_emer_event;

struct fw_info app_info = {
    .x = 1,
    .y = 1,
    .z = 0,
    .b = 1,
};

struct system_limit sys_limits = {
    .discharging_curr_limit = DEFAULT_DISCHARGING_CURR_LIMIT,
    .charging_curr_limit = DEFAULT_CHARGING_CURR_LIMIT,
    .temp_soft_limit = DEFAULT_TEMP_SOFT_LIMIT,
    .temp_hard_limit = DEFAULT_TEMP_HARD_LIMIT,
    .undervoltage_limit = DEFAULT_UNDERVOLTAGE_LIMIT,
};

void system_limit_init(void)
{
    int16_t curr_discharge;
    int16_t temp_soft;
    int16_t temp_hard;
    int16_t undervolt;

    curr_discharge = *((__IO uint16_t *)DISCHARGE_CURR_LIMIT_ADDR);
    if (curr_discharge < DISCHARGING_CURR_LOWER_LIMIT || curr_discharge > DISCHARGING_CURR_UPPER_LIMIT) {
        curr_discharge = DEFAULT_DISCHARGING_CURR_LIMIT;
    }
    temp_soft = *((__IO uint16_t *)TEMP_SOFT_LIMIT_ADDR);
    if (temp_soft < TEMP_SOFT_LOWER_LIMIT || temp_soft > TEMP_SOFT_UPPER_LIMIT) {
        temp_soft = DEFAULT_TEMP_SOFT_LIMIT;
    }
    temp_hard = *((__IO uint16_t *)TEMP_HARD_LIMIT_ADDR);
    if (temp_hard < TEMP_HARD_LOWER_LIMIT || temp_hard > TEMP_HARD_UPPER_LIMIT) {
        temp_hard = DEFAULT_TEMP_HARD_LIMIT;
    }
    undervolt = *((__IO uint16_t *)UNDERVOLTAGE_LIMIT_ADDR);
    if (undervolt < UNDERVOLTAGE_LOWER_LIMIT || temp_hard > UNDERVOLTAGE_UPPER_LIMIT) {
        undervolt = DEFAULT_UNDERVOLTAGE_LIMIT;
    }

    sys_limits.discharging_curr_limit = curr_discharge;
    sys_limits.temp_soft_limit = temp_soft;
    sys_limits.temp_hard_limit = temp_hard;
    sys_limits.undervoltage_limit = undervolt;
}