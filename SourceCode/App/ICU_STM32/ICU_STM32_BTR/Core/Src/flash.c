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

    while(__LL_FLASH_GET_FLAG(FLASH_SR_BSY)) 
    { 
       if((LL_GetTick()-tickstart) > Timeout)
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

void FLASH_PageErase(uint32_t PageAddress)
{
    /* Set the ERASE bit */
    SET_BIT(FLASH->PECR, FLASH_PECR_ERASE);

    /* Set PROG bit */
    SET_BIT(FLASH->PECR, FLASH_PECR_PROG);

    /* Write 00000000h to the first word of the program page to erase */
    *(__IO uint32_t *)(uint32_t)(PageAddress & ~(FLASH_PAGE_SIZE - 1)) = 0x00000000;
}

void LL_FLASHEx_Erase(uint32_t page_address)
{
    uint32_t address = 0U;
  
    /* Wait for last operation to be completed */
    FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    FLASH_PageErase(page_address);

    /* Wait for last operation to be completed */
    FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    /* If the erase operation is completed, disable the ERASE Bit */
    CLEAR_BIT(FLASH->PECR, FLASH_PECR_PROG);
    CLEAR_BIT(FLASH->PECR, FLASH_PECR_ERASE);
}

void LL_FLASH_Program(uint32_t Address, uint32_t Data)
{
    /* Wait for last operation to be completed */
    FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    /*Program word (32-bit) at a specified address.*/
    *(__IO uint32_t *)Address = Data;

    /* Wait for last operation to be completed */
    FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
}

/**
 * @brief Read word at specified address
 */
static uint32_t STM32_Flash_ReadWord (uint32_t addr)
{
    return *(volatile uint32_t*)addr;
}

/**
 * @brief Get page number
 */
int16_t STM32_Flash_GetPage(uint32_t addr)
{
    if (addr < STM32_FLASH_BASE || addr >= STM32_FLASH_BASE + 0x8000)
    {
        return -1;
    }
    else
    {
        return ((addr & 0x0000ffff) >> PAGE_MASK);
    }
}

/**
 * @brief write data into flash at speified address
 * @param[in] addr - write address [MUST addr % 4 == 0]
 */
bool STM32_Flash_Write (uint32_t addr, uint32_t * pBuf, uint32_t num)
{
    uint32_t SectorError = 0;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;

    if (addr < STM32_FLASH_BASE || addr % 4)
    {
        return false;
    }

    LL_FLASH_Unlock();
    addrx = addr;                   //!< start address
    endaddr = addr + num * 4;       //!< stop address

    if (endaddr < 0x08008000)
    {
        while (addrx < endaddr)
        {
            if (memcmp((void *)addr, (void *)pBuf, num * 4) == 0)
            {
                return true;
            }

            if (STM32_Flash_ReadWord(addrx) != 0x00000000)
            {
                LL_FLASHEx_Erase(STM32_FLASH_BASE + FLASH_PAGE_SIZE * STM32_Flash_GetPage(addrx));
            }
            else
            {
                addrx += 4;
            }
            FLASH_WaitForLastOperation(STM32_FLASH_WAITTIME);
        }
    }

    FLASH_WaitForLastOperation(STM32_FLASH_WAITTIME);
    while (addr < endaddr)
    {
        LL_FLASH_Program(addr, *pBuf);

        addr += 4;
        pBuf++;
    }

    LL_FLASH_Lock();

    return true;
}

/**
 * @brief read data from specified address
 */
void STM32_Flash_Read (uint32_t addr, uint32_t * pBuf, uint32_t num)
{
    for (int i = 0; i < num; i++)
    {
        pBuf[i] = STM32_Flash_ReadWord(addr);
        addr += 4;
    }
}

bool STM32_Flash_Erase(uint32_t start, uint32_t stop)
{
    bool ret = true;
    uint32_t SectorError = 0;

    uint8_t startPage = STM32_Flash_GetPage(start);
    uint8_t stopPage = STM32_Flash_GetPage(stop);

    LL_FLASH_Unlock();
    for (uint8_t i = startPage; i <= stopPage; i++)
    {
        LL_FLASHEx_Erase(STM32_FLASH_BASE + i * FLASH_PAGE_SIZE);
    }
    LL_FLASH_Lock();

    return ret;
}
