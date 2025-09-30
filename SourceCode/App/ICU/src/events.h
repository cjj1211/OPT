/*******************************************************************************
 * @file events.h
 * @brief System events process
 * @author chunxiao.dai@neuroxess.com
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <stdint.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

typedef enum {
    EVENT_NORMAL,
    EVENT_CRITICAL,
} event_level;

/**
 * @brief critical event (high priority)
 */
enum critical_event_type {
    EVENT_CRIT_NONE,
    EVENT_CRIT_BATTERY_UNDERVOLTAGE,
    EVENT_CRIT_CASE_TEMP_OVERHEAT,
    EVENT_CRIT_BATT_TEMP_OVERHEAT,
    EVENT_CRIT_DISCHARGE_OVERCURR,
    EVENT_CRIT_PROBE_OVERCURR,
};

/**
 * @brief normal event (low priority)
 */
enum normal_event_type {
    EVENT_NORM_NONE,
    EVENT_NORM_BLE_CONNECTED,
    EVENT_NORM_BLE_DISCONNECTED,
    EVENT_NORM_BATTERY_LOW,
    EVENT_NORM_TEMP_WARNING,
    EVENT_NORM_SAMPLING_START,
    EVENT_NORM_SAMPLING_STOP,
    EVENT_NORM_CHANNELS_SET,
};

struct event {
    uint32_t timestamp;
    uint8_t level;
    uint8_t type;
    uint16_t data;
};

typedef struct {
    bool triggered;
    bool handled;
    uint32_t duration_s;
} event_status_t;

typedef struct {
    event_status_t evt_batt_undervoltage;
    event_status_t evt_discharge_overcurr;
    event_status_t evt_charge_overcurr;
    event_status_t evt_board_overtemp;
    event_status_t evt_case_overtemp;      //!< Titanium case overtemp
    event_status_t evt_probe_overcurr;     //!< Intan probe overcurrent
} event_subsys_t;

struct event_work_item {
    struct k_work work;
    const struct zbus_channel *channel;
};

ZBUS_CHAN_DECLARE(critical_chan);
ZBUS_CHAN_DECLARE(normal_chan);

extern struct event_work_item work_items[];
extern atomic_t work_item_idx;
extern event_subsys_t event_subsys;

extern int zbus_link_init(void);
extern void event_workq_init(void);
// extern void event_subsystem_init(void);

#endif /** __EVENTS_H__ */
