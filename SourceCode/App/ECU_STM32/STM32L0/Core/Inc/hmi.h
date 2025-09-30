/**
 * @file hmi.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-01-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdint.h>

typedef enum {
    LED_Y,
    LED_G,
    LED_R,
    LED_A,
} LED;

extern volatile uint16_t g_keep_alive;

extern void vout_close(void);
extern void led_on(LED led_entity);
extern void led_off(LED led_entity);
extern void beep_on(void);
extern void beep_off(void);
extern void power_down(void);
extern void wakeup_key_listen (void);