/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。                   
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  描述 : 定义了蓝牙MCU与FPGA之间的通信接口
*  修改记录: 
*  
******************************************************************************/

#ifndef __FPGA_INTERFACE_H__
#define __FPGA_INTERFACE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
//#include "nrf_drv_spi.h"
// #include "nrfx_gpiote.h"
// #include "app_timer.h"
// #define QSPI_STD_CMD_WRSR   0x01
// #define QSPI_STD_CMD_RSTEN  0x66
// #define QSPI_STD_CMD_RST    0x99
#define QSPI_PAGE_SIZE     220  //102        //读取数据长度
#define GPIO_PIN    NRF_GPIO_PIN_MAP(0, 02) //开始中断引脚
#define GPIO_STOP_PIN    NRF_GPIO_PIN_MAP(0, 03)//结束中断引脚 
#define GPIO_FPGA_PIN    NRF_GPIO_PIN_MAP(0, 07)//控制FPGA引脚 
#define GPIO_30W_SMPIN    NRF_GPIO_PIN_MAP(0, 28)//控制FPGA引脚 
#define GPIO_WRITE_ADDR    0x00 
void spi_init(void);
void spi_uninit(void);

// void qspi_handler(nrf_drv_spi_evt_t event, void * p_context);

 /**
  * @brief 向FPGA写入配置数据
  * 
  * @param epAddr 配置地址
  * @param data 配置数据
  * @param length 配置数据长度
  * @return uint32_t 错误码，0表示成功，其他值表示失败
  */
 uint32_t write_config(uint32_t epAddr, uint8_t *data, uint32_t length);

/**
 * @brief 
 * 
 * @param addr 配置地址
 * @param data 读取数据缓冲区
 * @param length 读取数据长度
 * @return uint32_t 错误码，如果返回值小于length，表示读取失败
 */
uint32_t read_config(uint32_t epAddr, uint8_t *data, uint32_t length);


/**
 * @brief 向FPGA写入数据
 * 
 * @param addr 数据地址
 * @param data 
 * @param length 
 * @return uint32_t 错误码，小于0表示写入失败的错误码
 */
uint32_t write_data(uint32_t addr, uint8_t *data, uint32_t length);

/**
 * @brief 从FPGA读取数据
 * 
 * @param addr 数据地址
 * @param data 读取数据缓冲区
 * @param length 读取数据长度
 * @return uint32_t 错误码，如果返回值小于length，表示读取失败
 */
uint32_t read_data(uint32_t addr, uint8_t *data, uint32_t length);





#endif // __FPGA_INTERFACE_H__

