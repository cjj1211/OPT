/*******************************************************************************
 * @file   flash.c
 * @brief  Access embedded flash memory
 * @details
 * @copyright
 *       Copyright(C) 2025 NeuroXess, All rights reserved
 *
 * @par Changelog
 *       - 2025.1.16 created by chunxiao.dai <chunxiao.dai@neuroxess.com>
 *
 ******************************************************************************/

#include "flash.h"
#include "gpio.h"
#include <string.h>

/*******************************************************************************
 * @defgroup Variables
 ******************************************************************************/


/*******************************************************************************
 * @defgroup Functions
 ******************************************************************************/
extern uint32_t LL_GetTick(void);

void LL_FLASH_Unlock(void)
{
    uint32_t primask_bit;

    /* Unlocking FLASH_PECR register access*/
    if(LL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PELOCK))
    {
        /* Disable interrupts to avoid any interruption during unlock sequence */
        primask_bit = __get_PRIMASK();
        __disable_irq();

        WRITE_REG(FLASH->PEKEYR, FLASH_PEKEY1);
        WRITE_REG(FLASH->PEKEYR, FLASH_PEKEY2);

        /* Re-enable the interrupts: restore previous priority mask */
        __set_PRIMASK(primask_bit);
    }

    if (LL_IS_BIT_SET(FLASH->PECR, FLASH_PECR_PRGLOCK))
    {
        /* Disable interrupts to avoid any interruption during unlock sequence */
        primask_bit = __get_PRIMASK();
        __disable_irq();

        /* Unlocking the program memory access */
        WRITE_REG(FLASH->PRGKEYR, FLASH_PRGKEY1);
        WRITE_REG(FLASH->PRGKEYR, FLASH_PRGKEY2);

        /* Re-enable the interrupts: restore previous priority mask */
        __set_PRIMASK(primask_bit);
    }
}

void LL_FLASH_Lock(void)
{
    /* Set the PRGLOCK Bit to lock the FLASH Registers access */
    SET_BIT(FLASH->PECR, FLASH_PECR_PRGLOCK);
    /* Set the PELOCK Bit to lock the PECR Register access */
    SET_BIT(FLASH->PECR, FLASH_PECR_PELOCK);
}

void FLASH_WaitForLastOperation(uint32_t Timeout)
{
    uint32_t tickstart = LL_GetTick();

    while (__LL_FLASH_GET_FLAG(FLASH_SR_BSY))
    {
       if((LL_GetTick() - tickstart) > Timeout)
       {
            return ;
       }
    }

    /* Check FLASH End of Operation flag  */
    if (__LL_FLASH_GET_FLAG(FLASH_SR_EOP))
    {
        /* Clear FLASH End of Operation pending bit */
        __LL_FLASH_CLEAR_FLAG(FLASH_SR_EOP);
    }
}

void STM32_EEPROM_Read(uint32_t addr, uint8_t *data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        data[i] = *((__IO uint8_t *)addr + i);
    }
}

void STM32_EEPROM_Write(uint32_t addr, uint8_t *data, uint8_t len)
{
    LL_FLASH_Unlock();

    for (int i = 0; i < len; i++) {
        *((__IO uint8_t *)addr + i) = data[i];
    }

    LL_FLASH_Lock();
}
