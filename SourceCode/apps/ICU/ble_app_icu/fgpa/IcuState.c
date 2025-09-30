#include "IcuState.h"
#include "HardwareDefine.h"
#include "FpgaInterface.h"
#include <cstdint>
#include <stdbool.h>
#include <string.h>
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#define MAX_WRITE_LENGTH 64
#define writeAddr   0x4B   //I2C 写入地址 


// Define I2C address of BQ27220
#define BQ27220_ADDRESS 0x55
static IcuStatus icuStatus = ICU_OFFLINE;
/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = 11,
       .sda                = 12,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_twi_enable(&m_twi);
	
}
 
// Function to read TimeToEmpty() from BQ27220
void read_time_to_empty(uint8_t *data) {
    ret_code_t err_code;
    // Send command to read low byte
    uint8_t cmd = 0x16; // Command to read low byte of TimeToEmpty
    err_code = nrf_drv_twi_tx(&m_twi, BQ27220_ADDRESS, &cmd, 1, false);
	  if(err_code!=NRF_SUCCESS)
		{
		    NRF_LOG_DEBUG("ERROR: 0x%08X", err_code);
		}

    err_code = nrf_drv_twi_rx(&m_twi, BQ27220_ADDRESS, &data[0], 1);
		
		if(err_code!=NRF_SUCCESS)
		{
		    NRF_LOG_DEBUG("ERROR: 0x%08X", err_code);
		}
    // Send command to read high byte
    cmd = 0x17; // Command to read high byte of TimeToEmpty
    err_code = nrf_drv_twi_tx(&m_twi, BQ27220_ADDRESS, &cmd, 1, false);
if(err_code!=NRF_SUCCESS)
		{
		    NRF_LOG_DEBUG("ERROR: 0x%08X", err_code);
		}
	
    err_code = nrf_drv_twi_rx(&m_twi, BQ27220_ADDRESS, &data[1], 1);
		if(err_code!=NRF_SUCCESS)
		{
		    NRF_LOG_INFO("ERROR: 0x%08X", err_code);
		}
}

// 读
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi,dev_id,&reg_addr,1,false);
    
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_twi_rx(&m_twi,dev_id,reg_data,len);
    
    APP_ERROR_CHECK(err_code);
    
    return err_code;
}
// 写
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    ret_code_t err_code;
    
    uint8_t write_data[MAX_WRITE_LENGTH];
    
    if(len>MAX_WRITE_LENGTH-1)
    {
        err_code = 1;
        return err_code;
    }
    
    write_data[0] = reg_addr;
    
    memcpy(&write_data[1],reg_data,len);
    
    err_code = nrf_drv_twi_tx(&m_twi,dev_id,write_data,len+1,false);
    
    APP_ERROR_CHECK(err_code);
    
    return err_code;
}
 
uint8_t IIC_Write_1Byte(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data)
{
    ret_code_t err_code = user_i2c_write(SlaveAddress, REG_Address, &REG_data, 1);
    
    return err_code;
}
 
uint8_t IIC_Read_1Byte(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t *REG_data)
{
    ret_code_t err_code = user_i2c_read(SlaveAddress, REG_Address, REG_data, 1);
    
    return err_code;
}
 
 
uint8_t IIC_Write_nByte(uint8_t SlaveAddress, uint8_t REG_Address, uint8_t len, uint8_t *buf)
{
    ret_code_t err_code = user_i2c_write(SlaveAddress, REG_Address, buf, len);
    
    return err_code;
}
 
uint8_t IIC_Read_nByte(uint8_t SlaveAddress, uint8_t REG_Address, uint8_t len, uint8_t *buf)
{
    ret_code_t err_code = user_i2c_read(SlaveAddress, REG_Address, buf, len);
    
    return err_code;
}


void initICUState(void){
    //TODO 这里是测试代码，等FPGA正式代码有后，应该发送命令去获取相关状态
//    uint8_t init_data[5] = {0x55,0xAA,0x12,0x34};
//    uint8_t buffer_rx[5] = {0};
//    uint32_t err_code;
//    uint32_t initDataAddr = 0x00000002;
//    int data_len = 4;
//    err_code = read_data(initDataAddr, buffer_rx, data_len );
//    if(memcmp(buffer_rx,init_data,data_len) == 0){
//        icuStatus = ICU_ONLINE;
//    }else{
//        icuStatus = ICU_OFFLINE;
//    }
}

IcuStatus getIcuStatus(void){
    return icuStatus;
}

