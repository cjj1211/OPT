/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/**
 * @brief TMP119 Register Map
 * -----------------------------------------------------------
 * | ADDR | TYPE | RESET | REGISTER NAME                     |
 * -----------------------------------------------------------
 * | 00h  |  R   | 8000h | Temperature result register       |
 * -----------------------------------------------------------
 * | 01h  | R/W  | 0220h | Configuration register            |
 * -----------------------------------------------------------
 * | 02h  | R/W  | 6000h | Temperature high limit register   |
 * -----------------------------------------------------------
 * | 03h  | R/W  | 8000h | Temperature low limit register    |
 * -----------------------------------------------------------
 * | 04h  | R/W  | 0000h | EEPROM unlock register            |
 * -----------------------------------------------------------
 * | 05h  | R/W  | xxxxh | EEPROM1 register                  |
 * -----------------------------------------------------------
 * | 06h  | R/W  | xxxxh | EEPROM2 register                  |
 * -----------------------------------------------------------
 * | 07h  | R/W  | 0000h | Temperature offset register       |
 * -----------------------------------------------------------
 * | 08h  | R/W  | xxxxh | EEPROM3 register                  |
 * -----------------------------------------------------------
 * | 0Fh  |  R   | 0117h | Device ID register                |
 * -----------------------------------------------------------
 */
#define TMP119_REG_TEMP         0x00          //!< Temperature result register
#define TMP119_REG_CFGR         0x01          //!< Configuration register
#define TMP119_REG_HIGH         0x02          //!< High limit register
#define TMP119_REG_LOW          0x03          //!< Low limit register
#define TMP119_REG_UNLOCK       0x04          //!< EEPROM unlock register
#define TMP119_REG_OFFSET       0x07          //!< Temperature offset register
#define TMP119_REG_DEVICE_ID    0x0F          //!< Device ID register

#define TMP119_DEVICE_ID        0x2117        //!< TMP119 Device ID
#define CFGR_RESET_VALUE        0x0220        //!< Configuration register reset value
#define HIGH_LIMIT_RESET        0x0780        //!< 0x6000 / 128 * 10
#define LOW_LIMIT_RESET         0x0A00        //!< 0x8000 / 128 * 10
#define TMP119_CFGR_DATA_READY  (1 << 13)     //!< The data ready flag
#define TMP119_SD               (0x01)        //!< Shutdown
#define CFGR_MOD_MASK           (10)
#define CFGR_RESET_MASK         (1)

// TMP119 temperature resolution
#define TMP119_RESOLUTION       78125         //!< in tens of uCelsius
#define TMP119_RESOLUTION_DIV   10000000

#define TMP119_ADDR             (0x48 << 1)   //!< 7-bit address shifted left
#define I2C_TIMEOUT             100000        //!< Timeout counter value


// Error codes
typedef enum {
    I2C_OK = 0,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_NACK,
    I2C_ERROR_BUS,
    I2C_ERROR_ARBITRATION
} I2C_Status;


void MX_I2C1_Init(void);
void MX_I2C1_DeInit(void);

I2C_Status TMP119_WriteReg(uint8_t reg, uint16_t data);
I2C_Status TMP119_ReadReg(uint8_t reg, uint16_t *data);
I2C_Status TMP119_ReadTemperature(float *temp);
I2C_Status TMP119_Config(void);
I2C_Status TMP119_SetHighLimit(uint16_t high_limit);
I2C_Status TMP119_SetLowLimit(uint16_t low_limit);
I2C_Status TMP119_SoftReset(void);
I2C_Status TMP119_Shutdown(void);
I2C_Status TMP119_Wakeup(void);

I2C_Status I2C_Write(uint8_t addr, uint8_t *data, uint16_t size, uint8_t restart);
I2C_Status I2C_Read(uint8_t addr, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
