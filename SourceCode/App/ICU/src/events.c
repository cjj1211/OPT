/*******************************************************************************
 * @file  events.c
 * @brief System events process
 * @author chunxiao.dai@neuroxess.com
 *
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include "events.h"
#include "info.h"
#include "driver/bsp_gpio.h"
#include "public_api.h"
#include <ctype.h>
#include <zephyr/sys/timeutil.h>

#define LOG_MODULE_NAME OPT_EVTS
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

//!< Thread Config
#define CRITICAL_THREAD_PRIORITY    1
#define CRITICAL_THREAD_STACK_SIZE  2048

ZBUS_CHAN_DEFINE(
    critical_chan,
    struct event,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(0)
);

ZBUS_CHAN_DEFINE(
    normal_chan,
    struct event,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(0)
);

event_subsys_t event_subsys = {
    .evt_batt_undervoltage = {0},
    .evt_board_overtemp = {0},
    .evt_case_overtemp = {0},
    .evt_charge_overcurr = {0},
    .evt_discharge_overcurr = {0},
    .evt_probe_overcurr = {0},
};

//!< Proprietary work queue
K_THREAD_STACK_DEFINE(event_workq_stack, CONFIG_EVENT_WORKQUEUE_STACK_SIZE);
struct k_work_q event_workq;

//!< Work item pool
#define WORK_ITEM_POOL_SIZE 8
struct event_work_item work_items[WORK_ITEM_POOL_SIZE];
atomic_t work_item_idx = ATOMIC_INIT(0);

void event_ble_notify(const struct event *evt)
{
    if (atomic_load(&g_ble_connected))
    {
        uint8_t data[COMMON_FRAME_SIZE] = {0};
        Frame *frmAck = (Frame *)data;
        frmAck->header = HEADER;
        frmAck->seq = 0;
        frmAck->type = FRAME_EVENT;
        frmAck->dataLen = 0x08;
        frmAck->data[0] = (uint8_t)(evt->timestamp);
        frmAck->data[1] = (uint8_t)(evt->timestamp >> 8);
        frmAck->data[2] = (uint8_t)(evt->timestamp >> 16);
        frmAck->data[3] = (uint8_t)(evt->timestamp >> 24);
        frmAck->data[4] = evt->level;
        frmAck->data[5] = evt->type;
        frmAck->data[6] = (uint8_t)(evt->data);
        frmAck->data[7] = (uint8_t)(evt->data >> 8);
        sendFrame(frmAck);
    }
}

void event_work_handler(struct k_work *work)
{
#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    struct event_work_item *item = CONTAINER_OF(work, struct event_work_item, work);

    if (item->channel == NULL) {
        LOG_ERR("illegal zbus_channel pointer");
        return;
    }

    // alter
    const struct event *evt = zbus_chan_msg(item->channel);
    if (evt == NULL) {
        LOG_ERR("Failed to fetch channel message");
        return;
    }

    time_t time_seconds = evt->timestamp;
    struct tm current_tm;
    gmtime_r(&time_seconds, &current_tm);

    LOG_INF("#[%04d-%02d-%02d %02d:%02d:%02d] type = %d, value = 0x%0x",
                    current_tm.tm_year + 1900,
                    current_tm.tm_mon + 1,
                    current_tm.tm_mday,
                    current_tm.tm_hour,
                    current_tm.tm_min,
                    current_tm.tm_sec,
                    evt->type,
                    evt->data);

    nvs_event_save(evt);

#if CONFIG_EVENT_WORKQUEUE_MONITOR
    static uint32_t last_time;
    uint32_t now = k_cycle_get_32();
    uint32_t delta = k_cyc_to_us_near32(now - last_time);
    last_time = now;

    if (delta > CONFIG_EVENT_MAX_LATENCY) {
        LOG_WRN("Event process delay: %d us", delta);
    }
#endif
}

/**
 * @brief initialize event work queue
 * 
 * @return int 
 */
void event_workq_init(void)
{
    k_work_queue_start(
        &event_workq,
        event_workq_stack,
        K_THREAD_STACK_SIZEOF(event_workq_stack),
        CONFIG_EVENT_WORKQUEUE_PRIORITY,
        NULL
    );

    for (int i = 0; i < WORK_ITEM_POOL_SIZE; i++) {
        k_work_init(&work_items[i].work, event_work_handler);
    }

    LOG_INF("Event work queue started (prio: %d)", CONFIG_EVENT_WORKQUEUE_PRIORITY);
}

void on_normal_event(const struct zbus_channel *chan)
{
#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    uint8_t idx = atomic_inc(&work_item_idx) % WORK_ITEM_POOL_SIZE;
    struct event_work_item *item = &work_items[idx];

    item->channel = chan;

    int ret = k_work_submit_to_queue(&event_workq, &item->work);
    if (ret < 0) {
        LOG_WRN("event work submit fail (%d), try to process directly", ret);
        // const struct event *evt = zbus_chan_msg(chan);
        // normal_event_process(evt);
    }
}
ZBUS_LISTENER_DEFINE(normal_listener, on_normal_event);

ZBUS_SUBSCRIBER_DEFINE(critical_sub, 5);

/**
 * @brief Link zbus producers and observers
 * 
 * @return int 0 for success, error otherwise
 */
int zbus_link_init(void)
{
    int ret = 0;

    ret = zbus_chan_add_obs(&normal_chan, &normal_listener, K_MSEC(10));
    if (ret) {
        LOG_ERR("Failed to add normal event listener");
        return ret;
    }

    return ret;
}

void critical_handler_thread(void)
{
    // printk("Thread ID: %p, Name: %s\n", k_current_get(), k_current_get()->name);
    zbus_chan_add_obs(&critical_chan, &critical_sub, K_MSEC(10));

    for (;;)
    {
        const struct zbus_channel *chan;
        struct event *evt;

        int ret = zbus_sub_wait(&critical_sub, &chan, K_FOREVER);
        if (ret == 0) {
            // if (zbus_chan_read(chan, &evt, K_NO_WAIT) == 0) {
            //     LOG_INF("critical zbus_chan_read, type = %d", evt.type);
            // }
            zbus_chan_claim(chan, K_NO_WAIT);
            evt = zbus_chan_msg(chan);
            time_t time_seconds = evt->timestamp;
            struct tm current_tm;
            gmtime_r(&time_seconds, &current_tm);

            LOG_INF("*[%04d-%02d-%02d %02d:%02d:%02d] type = %d, value = 0x%0x",
                        current_tm.tm_year + 1900,
                        current_tm.tm_mon + 1,
                        current_tm.tm_mday,
                        current_tm.tm_hour,
                        current_tm.tm_min,
                        current_tm.tm_sec,
                        evt->type, evt->data);

            switch (evt->type) {
                case EVENT_CRIT_PROBE_OVERCURR:
                    intan_pwr_disable();
                    fpga_pwr_disable();
                    nvs_event_save(evt);
                    event_ble_notify(evt);
                    break;
                case EVENT_CRIT_BATTERY_UNDERVOLTAGE:
                    intan_pwr_disable();
                    fpga_pwr_disable();
                    nvs_event_save(evt);
                    // Tell STM32
                    event_ble_notify(evt);
                    break;
                case EVENT_CRIT_BATT_TEMP_OVERHEAT:
#if 0
                    intan_pwr_disable();
                    fpga_pwr_disable();
                    nvs_event_save(evt);
                    event_ble_notify(evt);
#endif
                    break;
                case EVENT_CRIT_CASE_TEMP_OVERHEAT:
                    intan_pwr_disable();
                    fpga_pwr_disable();
                    nvs_event_save(evt);
                    event_ble_notify(evt);
                    break;
                case EVENT_CRIT_DISCHARGE_OVERCURR:
                    break;
                default: break;
            }
            zbus_chan_finish(chan);
        }
    }
}

K_THREAD_DEFINE(critical_event_thread_id, CRITICAL_THREAD_STACK_SIZE, critical_handler_thread, NULL, NULL,
        NULL, CRITICAL_THREAD_PRIORITY, 0, 0);
