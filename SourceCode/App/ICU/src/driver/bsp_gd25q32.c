/******************************************************************************
*  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : SPI Flash interface
*  修改记录:
*
******************************************************************************/
#include "bsp_gd25q32.h"
#include "../public_api.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>


/**
 * @brief read identification
 *
 */
int gd25q32_id_read(uint8_t *id, uint8_t len)
{
    int err = 0;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[4] = {CMD_RDID, 0x00, 0x00, 0x00};
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 4,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };
    uint8_t rx_buffer[4];
    struct spi_buf rx_spi_buf = {
        .buf = rx_buffer,
        .len = 4,
    };
    struct spi_buf_set rx_spi_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1,
    };

    err = spi_transceive(fpga_dev.bus, &config, &tx_spi_buf_set, &rx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_transceive failed, err: %d\n", err);
    }
    else
    {
        memmove(id, rx_buffer + 1, len);
    }

    return err;
}

/**
 * @brief read status register
 *
 */
int gd25q32_status_register_read(uint8_t command)
{
    int err = 0;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[2] = {0x00};
    tx_buffer[0] = command;
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 2,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };
    uint8_t rx_buffer[2];
    struct spi_buf rx_spi_buf = {
        .buf = rx_buffer,
        .len = 2,
    };
    struct spi_buf_set rx_spi_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1,
    };

    err = spi_transceive(fpga_dev.bus, &config, &tx_spi_buf_set, &rx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_transceive failed, err: %d\n", err);
        return err;
    }
    else
    {
        return rx_buffer[1];
    }
}

/**
 * @brief wait WIP
 *
 */
bool gd25q32_busy_wait(void)
{
    uint32_t start_time = k_uptime_get_32();

    while ((gd25q32_status_register_read(CMD_RDSR_LSB) & 0x01) == 0x01)
    {
        if (k_uptime_get() - start_time >= 100)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief write enable
 *
 */
int gd25q32_write_enable(void)
{
    int err = 0;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[1] = {CMD_WREN};
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 1,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    err = spi_write(fpga_dev.bus, &config, &tx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_write failed, err: %d\n", err);
    }
    return err;
}

/**
 * @brief chip erase
 *
 */
int gd25q32_chip_erase(void)
{
    int err = 0;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[1] = {CMD_CE};
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 1,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    gd25q32_write_enable();
    err = spi_write(fpga_dev.bus, &config, &tx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_write failed, err: %d\n", err);
    }
    return err;
}

/**
 * @brief sector erase
 *
 */
int gd25q32_sector_erase(uint16_t sector_num)
{
    int err = 0;
    uint32_t addr = SECTOR_SIZE * sector_num;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[4] = {CMD_SE};
    tx_buffer[1] = (uint8_t)((addr & 0x00ff0000) >> 16);
    tx_buffer[2] = (uint8_t)((addr & 0x0000ff00) >> 8);
    tx_buffer[3] = (uint8_t)(addr);
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 4,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    gd25q32_write_enable();
    err = spi_write(fpga_dev.bus, &config, &tx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_write failed, err: %d\n", err);
    }
    gd25q32_busy_wait();
    return err;
}

/**
 * @brief Wirte data into one page
 *
 * @param[in] addr  write address(24bits address)
 * @param[in] p_buf the data need to write
 * @param[in] len   number of bytes (Maximum 256)
 *                  NOT exceed the remain bytes in the page
 */
static int gd25q32_wirte_page(uint32_t addr, uint8_t *p_buf, uint16_t len)
{
    int err = 0;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[GD_MAX_BUF_SIZE] = {CMD_PP};
    tx_buffer[1] = (uint8_t)((addr & 0x00ff0000) >> 16);
    tx_buffer[2] = (uint8_t)((addr & 0x0000ff00) >> 8);
    tx_buffer[3] = (uint8_t)(addr);
    for (int i = 0; i< len; i++)
    {
        tx_buffer[4+i] = p_buf[i];
    }
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = 4 + len,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    gd25q32_write_enable();
    err = spi_write(fpga_dev.bus, &config, &tx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_write failed, err: %d\n", err);
    }
    // printk("[%d]%s: %d", k_uptime_get_32(), __FILE__, __LINE__);
    gd25q32_busy_wait();
    // printk("[%d]%s: %d", k_uptime_get_32(), __FILE__, __LINE__);
    return err;
}

/**
 * @brief Write flash without check, with auto page change function
 * @note Ensure that all the data in the address range is 0xFF, otherwise
 *       write would fail.
 *
 * @param addr
 * @param p_buf
 * @param len
 * @return int
 */
int gd25q32_write_nocheck(uint32_t addr, uint8_t *p_buf, uint16_t len)
{
    uint16_t remain_bytes_in_page = PAGE_SIZE - addr % PAGE_SIZE;
    uint16_t write_bytes_in_page = (remain_bytes_in_page > len) ? len : remain_bytes_in_page;

    while (1)
    {
        gd25q32_wirte_page(addr, p_buf, write_bytes_in_page);
        if (len == write_bytes_in_page)
        {
            break;
        }
        else
        {
            p_buf += write_bytes_in_page;
            addr += write_bytes_in_page;
            len -= write_bytes_in_page;
            write_bytes_in_page = (len > PAGE_SIZE) ? PAGE_SIZE : len;
        }
    }
}

/**
 * @brief Write specfied length data in speified address with erase operation
 *
 */
uint8_t g_gd32q_buf[SECTOR_SIZE] = {0};
int gd25q32_flash_write(uint32_t addr, uint8_t *p_buf, uint16_t len)
{
    int err = 0;
    uint32_t sec_pos = 0;
    uint16_t sec_off = 0;
    uint16_t sec_remain = 0;
    uint8_t *buffer = g_gd32q_buf;

    sec_pos = addr / SECTOR_SIZE;
    sec_off = addr % SECTOR_SIZE;
    sec_remain = SECTOR_SIZE - sec_off;
    if (len <= sec_remain)
    {
        sec_remain = len;
    }

    while (1)
    {
        uint16_t index = 0;
        gd25q32_flash_read(sec_pos * SECTOR_SIZE, buffer, SECTOR_SIZE);
        for (index = 0; index < sec_remain; index++)
        {
            if (buffer[sec_off + index] != 0xFF)
            {
                break;
            }
        }
        if (index < sec_remain)
        {
            gd25q32_sector_erase(sec_pos);
            for (index = 0; index < sec_remain; index++)
            {
                buffer[sec_off + index] = p_buf[index];
            }
            gd25q32_write_nocheck(sec_pos * SECTOR_SIZE, buffer, SECTOR_SIZE);
        }
        else
        {
            gd25q32_write_nocheck(addr, p_buf, sec_remain);
        }

        if (len == sec_remain)
        {
            break; //!< write finish
        }
        else
        {
            sec_pos++;
            sec_off = 0;

            p_buf += sec_remain;
            addr += sec_remain;
            len -= sec_remain;
            sec_remain = (len > SECTOR_SIZE) ? SECTOR_SIZE : len;
        }
    }

    return err;
}

/**
 * @brief read flash
 *
 * @param addr
 * @param data
 * @param len
 * @return int
 */
int gd25q32_flash_read(uint32_t addr, uint8_t *rx_data, uint16_t rx_len)
{
    int err = 0;
    uint16_t read_bytes = (rx_len > PAGE_SIZE) ? PAGE_SIZE : rx_len;

    struct spi_config config = fpga_dev.config;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;

    uint8_t tx_buffer[GD_MAX_BUF_SIZE] = {CMD_READ};
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = read_bytes + 4,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };
    uint8_t rx_buffer[GD_MAX_BUF_SIZE];
    struct spi_buf rx_spi_buf = {
        .buf = rx_buffer,
        .len = read_bytes + 4,
    };
    struct spi_buf_set rx_spi_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1,
    };

    do {
        tx_buffer[1] = (uint8_t)((addr & 0x00ff0000) >> 16);
        tx_buffer[2] = (uint8_t)((addr & 0x0000ff00) >> 8);
        tx_buffer[3] = (uint8_t)(addr);
        tx_spi_buf.len = read_bytes + 4;
        rx_spi_buf.len = read_bytes + 4;
        err = spi_transceive(fpga_dev.bus, &config, &tx_spi_buf_set, &rx_spi_buf_set);
        if (err < 0)
        {
            printk("spi_transceive failed, err: %d\n", err);
            break;
        }
        else
        {
            memmove(rx_data, rx_buffer + 4, read_bytes);
            addr += read_bytes;
            rx_data += read_bytes;
            rx_len -= read_bytes;
            read_bytes = (rx_len > PAGE_SIZE) ? PAGE_SIZE : rx_len;
        }
    } while (read_bytes != 0);

    return err;
}