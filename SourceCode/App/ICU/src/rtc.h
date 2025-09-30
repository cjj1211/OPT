/*******************************************************************************
 * @file rtc.h
 * @brief RTC
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __RTC_H__
#define __RTC_H__

#include <stdint.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define TIMEZONE_OFFSET (8 * 3600) // UTC+8

extern const struct device *rtc_dev;

extern void rtc_init(void);
extern uint32_t rtc_calibrated_time_get(const struct device *rtc_dev);
extern void rtc_calibrated_time_set(const struct device *rtc_dev, uint32_t seconds);
extern void rtc_time_print(void); 

#endif /** __RTC_H__ */
