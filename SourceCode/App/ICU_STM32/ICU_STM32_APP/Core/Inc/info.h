/**
  * @file info.h
  * @author chunxiao.dai@neuroxess.com
  * @brief System info
  * @version 0.1
  * @date 2025-07-14
  *
  * @copyright Copyright (C) 2025 NeuroXess
  *
  */

#ifndef __INFO_H__
#define __INFO_H__

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * @defgroup Macros
 *****************************************************************************/
#define EVENT_MAGIC 0xC3

#define EEPROM_BASE                (0x08080000)
#define EMER_EVT_1ST_ADDR          (EEPROM_BASE + 0x0080)
#define EMER_EVT_2ND_ADDR          (EEPROM_BASE + 0x03E0)
#define DISCHARGE_CURR_LIMIT_ADDR  (EEPROM_BASE)
#define CHARGE_CURR_LIMIT_ADDR     (EEPROM_BASE + 0x0004)
#define TEMP_SOFT_LIMIT_ADDR       (EEPROM_BASE + 0x0008)
#define TEMP_HARD_LIMIT_ADDR       (EEPROM_BASE + 0x000C)
#define UNDERVOLTAGE_LIMIT_ADDR    (EEPROM_BASE + 0x0010)
#define RECOVERY_MAGIC_ADDR        (EEPROM_BASE + 0x0060)

#define DEFAULT_DISCHARGING_CURR_LIMIT  (100) // mA
#define DEFAULT_CHARGING_CURR_LIMIT     (200) // mA
#define DEFAULT_TEMP_SOFT_LIMIT         (390) // 0.1C
#define DEFAULT_TEMP_HARD_LIMIT         (410) // 0.1C
#define DEFAULT_UNDERVOLTAGE_LIMIT      (3250) // mV

#define DISCHARGING_CURR_LOWER_LIMIT    (50)
#define DISCHARGING_CURR_UPPER_LIMIT    (150)
#define CHARGING_CURR_LOWER_LIMIT       (50)
#define CHARGING_CURR_UPPER_LIMIT       (150)
#define TEMP_SOFT_LOWER_LIMIT           (380)
#define TEMP_SOFT_UPPER_LIMIT           (420)
#define TEMP_HARD_LOWER_LIMIT           (400)
#define TEMP_HARD_UPPER_LIMIT           (430)
#define UNDERVOLTAGE_LOWER_LIMIT        (3000)
#define UNDERVOLTAGE_UPPER_LIMIT        (3500)

/******************************************************************************
 * @defgroup Enums
 *****************************************************************************/
typedef enum {
    EMER_EVT_OVERCURR = 0xF1,
    EMER_EVT_OVERTEMP = 0xF2,
} emer_evt_t;

/*******************************************************************************
 * @defgroup Structs
 ******************************************************************************/

#pragma pack(push, 1)

struct emer_event {
    uint8_t exist;
    uint8_t type;
    uint16_t value;
    uint32_t time;
};

#pragma pack(pop)

struct fw_info {
    uint8_t x;  //!< major enhancement class
    uint8_t y;  //!< slightly enhanced class
    uint8_t z;  //!< corrective class
    uint16_t b; //!< build number from VCS
};

struct system_limit {
    int16_t discharging_curr_limit;
    int16_t charging_curr_limit;
    int16_t temp_soft_limit;
    int16_t temp_hard_limit;
    int16_t undervoltage_limit;
};

/******************************************************************************
 * @defgroup declare variables
 *****************************************************************************/
extern struct fw_info app_info;
extern struct system_limit sys_limits;
extern struct emer_event g_emer_event;

/******************************************************************************
 * @defgroup function prototypes
 *****************************************************************************/
void system_limit_init(void);

#endif /* __INFO_H__ */
