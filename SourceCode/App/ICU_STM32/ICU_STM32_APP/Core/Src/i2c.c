/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  *
  *  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
  *  作者 : 崔俊杰 junjie.cui@neuroxess.com
  *  描述 : I2C
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* I2C1 init function */
void MX_I2C1_Init(void)
{
    // Enable peripheral clocks
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;   // Enable I2C1 clock
    RCC->IOPENR  |= RCC_IOPENR_GPIOBEN;   // Enable GPIOB clock

    // Configure PB6 (SCL) and PB7 (SDA) as alternate function open-drain
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOB->MODER |= (2 << GPIO_MODER_MODE6_Pos) | (2 << GPIO_MODER_MODE7_Pos); // AF mode
    GPIOB->OTYPER |= GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7;                      // Open-drain
    GPIOB->PUPDR  |= (1 << GPIO_PUPDR_PUPD6_Pos) | (1 << GPIO_PUPDR_PUPD7_Pos);// Pull-up
    GPIOB->AFR[0] |= (1 << (6*4)) | (1 << (7*4));                              // AF1 (I2C1)

    // Configure I2C timing for 100 kHz (assuming 16MHz system clock)
    I2C1->CR1 &= ~I2C_CR1_PE;            // Disable I2C before configuration
    I2C1->TIMINGR = 0x00503D58;
    I2C1->CR1 |= I2C_CR1_PE;             // Enable I2C peripheral
}

void MX_I2C1_DeInit(void)
{
    I2C1->CR1 &= ~I2C_CR1_PE;
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
}

/**
  * @brief  Check and clear I2C error flags
  * @param  None
  * @retval I2C_Status: Error status
  */
static I2C_Status I2C_CheckErrors(void)
{
    uint32_t isr = I2C1->ISR;

    if (isr & I2C_ISR_NACKF) {
        I2C1->ICR |= I2C_ICR_NACKCF;     // Clear NACK flag
        // printf("NACK\n");
        return I2C_ERROR_NACK;
    }

    if (isr & I2C_ISR_BERR) {
        I2C1->ICR |= I2C_ICR_BERRCF;     // Clear bus error flag
        // printf("bus error\n");
        return I2C_ERROR_BUS;
    }

    if (isr & I2C_ISR_ARLO) {
        I2C1->ICR |= I2C_ICR_ARLOCF;     // Clear arbitration lost flag
        // printf("arbitration lost\n");
        return I2C_ERROR_ARBITRATION;
    }

    return I2C_OK;
}

/**
  * @brief  Write data to TMP119 register
  * @param  reg: Register address to write
  * @param  data: 16-bit data to write
  * @retval I2C_Status: Operation status
  */
I2C_Status TMP119_WriteReg(uint8_t reg, uint16_t data)
{
    uint32_t timeout = 0;

    // Wait for I2C bus to be ready
    while (I2C1->ISR & I2C_ISR_BUSY) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
    }

    // Configure transfer parameters (3 bytes: register address + 2 data bytes)
    I2C1->CR2 = TMP119_ADDR |           // Slave address
                (3 << 16)   |           // Number of bytes to transmit
                I2C_CR2_AUTOEND |       // Enable auto end mode
                I2C_CR2_START;          // Generate START condition

    // Send register address
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_TXIS)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->TXDR = reg;

    // Send data high byte
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_TXIS)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->TXDR = (data >> 8) & 0xFF;

    // Send data low byte
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_TXIS)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->TXDR = data & 0xFF;

    // Wait for STOP condition generation
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_STOPF)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->ICR |= I2C_ICR_STOPCF;

    return I2C_OK;
}

/**
  * @brief  Read data from TMP119 register
  * @param  reg: Register address to read
  * @param  data: Pointer to store read data
  * @retval I2C_Status: Operation status
  */
I2C_Status TMP119_ReadReg(uint8_t reg, uint16_t *data)
{
    uint32_t timeout = 0;

    // Write register address phase
    I2C1->CR2 = TMP119_ADDR |            // Slave address + write
                (1 << 16)    |           // Number of bytes to transmit
                I2C_CR2_START;           // Generate START

    // Wait for TXIS flag (transmit data register empty)
    while (!(I2C1->ISR & I2C_ISR_TXIS)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->TXDR = reg;                    // Send register address

    // Restart in read mode
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_TC)) {   // Wait for transfer complete
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }

    // Configure read transfer
    I2C1->CR2 = TMP119_ADDR | 1 |        // Slave address + read
                (2 << 16)    |           // Number of bytes to receive
                I2C_CR2_START |          // Generate repeated START
                I2C_CR2_RD_WRN |         // Read direction
                I2C_CR2_AUTOEND;         // Auto end after reception

    // Read data bytes
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_RXNE)) {// Wait for first byte
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    *data = I2C1->RXDR << 8;             // Read MSB first

    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_RXNE)) {// Wait for second byte
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    *data |= I2C1->RXDR;                 // Read LSB

    // Wait for STOP condition
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_STOPF)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->ICR |= I2C_ICR_STOPCF;         // Clear STOP flag

    return I2C_OK;
}

/**
  * @brief  Read temperature from TMP119
  * @param  temp: Pointer to store temperature value
  * @retval I2C_Status: Operation status
  */
I2C_Status TMP119_ReadTemperature(float *temp)
{
    uint16_t raw_data;
    I2C_Status status;

    // Read temperature register (0x00)
    if((status = TMP119_ReadReg(TMP119_REG_TEMP, &raw_data)) != I2C_OK) {
        return status;
    }

    // Convert raw data to temperature (12-bit resolution)
    *temp = (int16_t)raw_data * 0.0078125f; // LSB = 0.0078125°C
    return I2C_OK;
}

static I2C_Status TMP119_EE_Unlock(void)
{
    I2C_Status status;

    // Write EEPROM Unlock register (0x04)
    status = TMP119_WriteReg(TMP119_REG_UNLOCK, (1 << 15));
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

static I2C_Status TMP119_EE_Lock(void)
{
    I2C_Status status;

    // Write EEPROM Unlock register (0x04)
    status = TMP119_WriteReg(TMP119_REG_UNLOCK, 0);
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

I2C_Status TMP119_SoftReset(void)
{
    I2C_Status status;

    status = TMP119_WriteReg(TMP119_REG_CFGR, (1 << 1));
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

I2C_Status TMP119_Config(void)
{
    uint16_t cfg_val = 0;
    I2C_Status status;

    status = TMP119_ReadReg(TMP119_REG_CFGR, &cfg_val);
    if (status != I2C_OK) {
        return status;
    }
    if (CFGR_RESET_VALUE != cfg_val) {
        return TMP119_WriteReg(TMP119_REG_CFGR, CFGR_RESET_VALUE);
    }

    return I2C_OK;
}

/**
 * @brief Set high limit register 
 * 
 * @param high_limit 
 * @return I2C_Status 
 */
I2C_Status TMP119_SetHighLimit(uint16_t high_limit)
{
    uint16_t raw_data = 0;
    I2C_Status status;

    // Convert temperature to raw data
    raw_data = high_limit * 128 / 10;   // 128 LSB = 1°C

    // Write High_Limit register (0x02)
    status = TMP119_WriteReg(TMP119_REG_HIGH, raw_data);
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

/**
 * @brief Set low limit register
 * 
 * @param low_limit 
 * @return I2C_Status 
 */
I2C_Status TMP119_SetLowLimit(uint16_t low_limit)
{
    uint16_t raw_data = 0;
    I2C_Status status;

    // Convert temperature to raw data
    raw_data = low_limit * 128 / 10;   // 128 LSB = 1°C

    // Write High_Limit register (0x03)
    status = TMP119_WriteReg(TMP119_REG_LOW, raw_data);
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

I2C_Status TMP119_Shutdown(void)
{
    uint16_t reg_val = 0;
    I2C_Status status;

    status = TMP119_ReadReg(TMP119_REG_CFGR, &reg_val);
    if (status != I2C_OK) {
        return status;
    }

    reg_val |= (TMP119_SD << CFGR_MOD_MASK);
    status = TMP119_WriteReg(TMP119_REG_CFGR, reg_val);
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

I2C_Status TMP119_Wakeup(void)
{
    uint16_t reg_val = 0;
    I2C_Status status;

    status = TMP119_ReadReg(TMP119_REG_CFGR, &reg_val);
    if (status != I2C_OK) {
        return status;
    }

    reg_val |= (1 << CFGR_RESET_MASK);
    status = TMP119_WriteReg(TMP119_REG_CFGR, reg_val);
    if (status != I2C_OK) {
        return status;
    }

    return I2C_OK;
}

/* *************************  Registers Operation for TMP119 END ********************************** */

I2C_Status I2C_Read(uint8_t addr, uint8_t *data, uint16_t size)
{
    uint32_t timeout = 0;
    // config
    I2C1->CR2 = (addr << 1) | 1 |           // salve addr + w/r direction (LSB = 1)
                (size << 16)        |       // recv bytes
                I2C_CR2_START       |       // generate START flag
                I2C_CR2_RD_WRN     |        // w/r direction
                I2C_CR2_AUTOEND;            // auto stop

    // recv loop
    for (uint16_t i=0; i<size; i++){
        timeout = 0;
        // wait data register not empty
        while (!(I2C1->ISR & I2C_ISR_RXNE)) {
            if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
            I2C_CheckErrors();  // check error
        }
        data[i] = I2C1->RXDR;
    }

    // stop handle
    timeout = 0;
    while (!(I2C1->ISR & I2C_ISR_STOPF)) {
        if (++timeout > I2C_TIMEOUT) return I2C_ERROR_TIMEOUT;
        I2C_CheckErrors();
    }
    I2C1->ICR |= I2C_ICR_STOPCF;  // clear stop flag

    return I2C_OK;
}

I2C_Status I2C_Write(uint8_t addr, uint8_t *data, uint16_t size, uint8_t restart)
{
    uint32_t timeout = 0;

    // config
    I2C1->CR2 = (addr << 1)       |  // slave addr（7 bit addr）
               (size << 16)       |  // bytes to transfer
               I2C_CR2_START      |  // generate START flag
               (restart ? 0 : I2C_CR2_AUTOEND);  // according to para 'restart' setting autostop mode;

    // data transfer loop
    for (uint16_t i = 0; i < size; i++) {
        timeout = 0;
        while (!(I2C1->ISR & I2C_ISR_TXIS)) {
            if (++timeout > I2C_TIMEOUT) {
                return I2C_ERROR_TIMEOUT;
            }
            I2C_CheckErrors();
        }
        I2C1->TXDR = data[i];
    }

    // stop handle
    if (restart) {
        timeout = 0;
        // keep bus control and no stop
        while (!(I2C1->ISR & I2C_ISR_TC)) {
            if (++timeout > I2C_TIMEOUT) {
                return I2C_ERROR_TIMEOUT;
            }
            I2C_CheckErrors();
        }
    } else {
        timeout = 0;
        while (!(I2C1->ISR & I2C_ISR_STOPF)) {
            if (++timeout > I2C_TIMEOUT) {
                return I2C_ERROR_TIMEOUT;
            }
            I2C_CheckErrors();
        }
        I2C1->ICR |= I2C_ICR_STOPCF;  // clear stop flag
    }

    return I2C_OK;
}
