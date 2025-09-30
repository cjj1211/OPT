/*******************************************************************************
 * @file    flash.h
 * @brief   This file contains all the function prototypes for
 *          the bsp_flash.c file
 ******************************************************************************/

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/*******************************************************************************
 * @defgroup Macros
 ******************************************************************************/
#define LL_IS_BIT_SET(REG, BIT)         (((REG) & (BIT)) == (BIT))
#define __LL_FLASH_GET_FLAG(__FLAG__)   (((FLASH->SR) & (__FLAG__)) == (__FLAG__))
#define __LL_FLASH_CLEAR_FLAG(__FLAG__) ((FLASH->SR) = (__FLAG__))

#define FLASH_PEKEY1               (0x89ABCDEFU) /*!< Flash program erase key1 */
#define FLASH_PEKEY2               (0x02030405U) /*!< Flash program erase key: used with FLASH_PEKEY2
                                                     to unlock the write access to the FLASH_PECR register and
                                                     data EEPROM */
#define FLASH_PRGKEY1              (0x8C9DAEBFU) /*!< Flash program memory key1 */
#define FLASH_PRGKEY2              (0x13141516U) /*!< Flash program memory key2: used with FLASH_PRGKEY2
                                                     to unlock the program memory */
#define FLASH_TIMEOUT_VALUE      (50000U) /* 50 s */

//!< Embedded flash memory
#define STM32_FLASH_BASE         0x08000000
#define STM32_FLASH_WAITTIME     50000
#define PAGE_MASK               (7u)
#define FLASH_PAGE_SIZE          (128U)  /*!< FLASH Page Size in bytes */


/*******************************************************************************
 * @defgroup Exported variables
 * ****************************************************************************/

/*******************************************************************************
 * @defgroup Function prototypes
 * ****************************************************************************/

//!< STM32 embedded flash related access function

void STM32_EEPROM_Read(uint32_t addr, uint8_t *data, uint8_t len);
void STM32_EEPROM_Write(uint32_t addr, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /** __BSP_FLASH_H__ */
