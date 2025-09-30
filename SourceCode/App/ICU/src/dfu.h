/*******************************************************************************
 * @file dfu.h
 * @brief DFU
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __DFU_H__
#define __DFU_H__

#include <stdint.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define STM32_DFU_PRIORITY 6
#define STM32_FW_MAX_SIZE  (24 * 1024) // 24 KB

typedef enum {
    DFU_BTR_ENTER     = 0,
    DFU_METAINFO_SEND = 1,
    DFU_BODY_SEND     = 2,
    DFU_APP_ENTER     = 3,
    DFU_SUCCESS       = 4,
    DFU_FAIL          = 5,
    DFU_FINISH        = 6,
} dfu_internal_state_t;

typedef enum {
    DFU_NONE  = 0,
    DFU_STM32 = 1,
    DFU_FPGA  = 2,
    DFU_UNK   = 3,
} dfu_type_t;

typedef enum {
    TRANSFER_STARTED = 0,
    TRANSFER_SUCCESS = 1,
    TRANSFER_FAILED  = 2,
    UPGRADE_FAILED   = 3,
    UPGRADE_SUCCESS  = 4,
    UPGRADE_STARTED  = 5,
} dfu_host_state_t;

struct msgq_uart_ack_t
{
    uint16_t len;
    uint16_t seq;
    uint8_t  data[160];
};

extern struct k_sem sem_stm32_dfu;
extern volatile bool g_stm32_dfu_enter;
extern volatile bool g_stm32_talk;
extern struct k_msgq msgq_dfu_uart_rx;
extern struct k_msgq msgq_uart_rx;
extern struct k_msgq msgq_sync_uart_rx;
extern volatile uint8_t dfu_data[STM32_FW_MAX_SIZE];
extern volatile dfu_type_t g_dfu_type;

#endif /** __DFU_H__ */
