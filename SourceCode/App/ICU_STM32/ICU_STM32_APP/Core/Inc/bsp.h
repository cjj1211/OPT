/**
  * @file bsp.h
  * @author DCX (chunxiao.dai@neuroxess.com)
  * @brief general BSP API
  * @version 0.1
  * @date 2025-03-06
  *
  * @copyright Copyright (C) 2025 NeuroXess
  *
  */

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t NX_SysTick_Config(uint32_t ticks);
extern void delay_us(uint16_t delay);
extern void delay_ms(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
