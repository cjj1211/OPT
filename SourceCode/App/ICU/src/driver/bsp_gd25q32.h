/******************************************************************************
*  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : SPI Flash interface
*  修改记录:
*
******************************************************************************/

#ifndef __BSP_GD25Q32_H__
#define __BSP_GD25Q32_H__

#include <stdint.h>

/**
 * @brief GD25Q32E
 *   ------------------------------------------------------------------------------
 *   | Each device has | Each block has | Each sector has | Each page has |        |
 *   ------------------------------------------------------------------------------
 *   |       4M        |     64/32K     |        4K       |      256      | Bytes  |
 *   ------------------------------------------------------------------------------
 *   |       16K       |     256/128    |        16       |       -       | pages  |
 *   ------------------------------------------------------------------------------
 *   |       1K        |      16/8      |        -        |       -       | sectos |
 *   ------------------------------------------------------------------------------
 *   |     64/128      |       -        |        -        |       -       | blocks |
 *   ------------------------------------------------------------------------------
 */

#define PAGE_SIZE    256
#define SECTOR_SIZE  4096
#define GD_MAX_BUF_SIZE (PAGE_SIZE + 4)

//!< Command list of GD25Q32
#define CMD_WREN       0x06  // Write enable
#define CMD_WRDI       0x04  // Write disable
#define CMD_RDSR_LSB   0x05  // Read status register
#define CMD_RDSR_MID   0x35
#define CMD_RDSR_MSB   0x15
#define CMD_WRSR_LSB   0x01  // Write status register
#define CMD_WRSR_MID   0x31
#define CMD_WRSR_MSB   0x11
#define CMD_READ       0x03  // Read data bytes
#define CMD_PP         0x02  // Page program
#define CMD_SE         0x20  // Sector erase
#define CMD_CE         0x60  // Chip erase
#define CMD_RDID       0x9F  // Read indentification

extern int gd25q32_id_read(uint8_t *id, uint8_t len);
extern int gd25q32_status_register_read(uint8_t command);
extern int gd25q32_write_enable(void);
extern int gd25q32_chip_erase(void);
extern int gd25q32_sector_erase(uint16_t sector_num);
extern int gd25q32_flash_write(uint32_t addr, uint8_t *data, uint16_t len);
extern int gd25q32_flash_read(uint32_t addr, uint8_t *rx_data, uint16_t rx_len);

#endif /* __BSP_GD25Q32_H__ */