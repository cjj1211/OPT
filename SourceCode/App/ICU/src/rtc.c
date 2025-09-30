/*******************************************************************************
 * @file  rtc.c
 * @brief RTC
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include "rtc.h"
#include "public_api.h"
#include <ctype.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/timeutil.h>

#define LOG_MODULE_NAME OPT_RTC
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);


const struct device *rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));
// static uint64_t time_offset = 1746720000ULL * 32768; // UTC 2025-05-09 00:00:00
static uint64_t time_offset = 0;
static volatile uint32_t overflow_count = 0;
static struct k_mutex time_offset_mutex;

void alarm_handler(const struct device *dev, uint8_t id, uint32_t ticks, void *user_data)
{
    LOG_INF("Alarm trigger");
}

void top_callback(const struct device *dev, void *user_data)
{
    overflow_count++;
    LOG_INF("Overflow count: %u", overflow_count);
}

void rtc_init(void)
{
    if (!device_is_ready(rtc_dev))
    {
        LOG_ERR("RTC0 device is not ready");
        return ;
    }

    struct counter_top_cfg top_cfg = {
        .ticks = counter_get_max_top_value(rtc_dev),
        .callback = top_callback,
        .user_data = NULL,
        .flags = 0,
    };
    int err = counter_set_top_value(rtc_dev, &top_cfg);
    if (err != 0)
    {
        LOG_ERR("Failed to set top value: %d", err);
        return ;
    }

    // struct counter_alarm_cfg alarm_cfg = {
    //     .flags = COUNTER_ALARM_CFG_ABSOLUTE,
    //     .ticks = counter_get_max_top_value(rtc_dev),
    //     .callback = alarm_handler,
    //     .user_data = NULL,
    // };
    // int err = counter_set_channel_alarm(rtc_dev, 0, &alarm_cfg);
    // if (err != 0)
    // {
    //     LOG_ERR("Failed to set alarm: %d", err);
    //     return ;
    // }

    //!< Start Counter
    err = counter_start(rtc_dev);
    if (err != 0)
    {
        LOG_ERR("Failed to start counter: %d", err);
        return ;
    }

    k_mutex_init(&time_offset_mutex);
    // LOG_INF("Max Top Value: %d, %d", counter_get_max_top_value(rtc_dev), counter_get_frequency(rtc_dev));
    LOG_INF("RTC0 initialized with external 32.768 crystal, prescaler = %d", DT_PROP(DT_NODELABEL(rtc0), prescaler));
}

uint32_t rtc_calibrated_time_get(const struct device *rtc_dev)
{
    uint32_t ticks = 0;
    uint64_t local_offset;
    uint32_t freq = counter_get_frequency(rtc_dev);
    uint32_t max_top_value = counter_get_max_top_value(rtc_dev);

    counter_get_value(rtc_dev, &ticks);

    k_mutex_lock(&time_offset_mutex, K_FOREVER);
    local_offset = time_offset;
    k_mutex_unlock(&time_offset_mutex);

    return (ticks + overflow_count * max_top_value + local_offset) / freq + TIMEZONE_OFFSET;
    // return (ticks + overflow_count * max_top_value) / freq + 1746720000 + TIMEZONE_OFFSET;
}

void rtc_calibrated_time_set(const struct device *rtc_dev, uint32_t seconds)
{
    uint32_t freq = counter_get_frequency(rtc_dev);
    uint32_t current_ticks;

    counter_get_value(rtc_dev, &current_ticks);

    k_mutex_lock(&time_offset_mutex, K_FOREVER);
    time_offset = ((uint64_t)seconds * freq) - current_ticks;
    k_mutex_unlock(&time_offset_mutex);
}

void rtc_time_print(void)
{
    // uint32_t timestamp = 0;
    // counter_get_value(rtc_dev, &timestamp);
    // uint64_t ustime = counter_ticks_to_us(rtc_dev, timestamp);
    // LOG_INF("Current RTC time: %d, %lld(%d)", timestamp, 
    //                                           ustime / USEC_PER_SEC,
    //                                           timestamp / counter_get_frequency(rtc_dev));
    // time_t time_seconds = ustime / USEC_PER_SEC;
    time_t time_seconds = rtc_calibrated_time_get(rtc_dev);
    struct tm current_tm;
    gmtime_r(&time_seconds, &current_tm);

    LOG_INF("Current time: %04d-%02d-%02d %02d:%02d:%02d",
            current_tm.tm_year + 1900,
            current_tm.tm_mon + 1,
            current_tm.tm_mday,
            current_tm.tm_hour,
            current_tm.tm_min,
            current_tm.tm_sec);
}
