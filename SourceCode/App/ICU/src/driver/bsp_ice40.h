/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : FPGA interface
*  修改记录:
*
******************************************************************************/

#ifndef __BSP_ICE40_H__
#define __BSP_ICE40_H__

#include <stdint.h>
#include <zephyr/drivers/spi.h>
#include "HardwareDefine.h"

#define FPGA_W  0x03
#define FPGA_R  0x05

//!< 4 + 64 * 2 * 10 / 8 = 164
#define SPI_MAX_FRAME_SIZE 164
#define TX_BUF_SIZE (SPI_MAX_FRAME_SIZE + 4)
#define RX_BUF_SIZE (SPI_MAX_FRAME_SIZE + 4)

extern int fpga_write(uint8_t addr, uint8_t *data, uint8_t len);
extern int fpga_read(uint8_t addr, uint8_t *data, uint8_t len);
extern int fpga_read_async(uint8_t addr, uint8_t len);

#endif /* __BSP_ICE40_H__ */