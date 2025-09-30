/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : FPGA interface
*  修改记录:
*
******************************************************************************/
#include "bsp_ice40.h"
#include "bsp_gpio.h"
#include "../public_api.h"
#include <zephyr/sys/printk.h>

#define NUM_BUFFERS 2
#define BUFFER_SIZE 164

__aligned(4) uint8_t rx_buf[NUM_BUFFERS][BUFFER_SIZE];
atomic_t rx_idx = ATOMIC_INIT(0);

static void spi_complete_cb(const struct device *dev, int result, void *user_data)
{
    // printk("spi_complete_cb\n");
    gpio_pin_set_dt(&ice_cs, 0);
    intan_pwr_disable();
    struct msgq_sample_data_t data;

    data.len = (chans_select * 20 + 7) / 8 + 4;
    data.seq = sample_frame_seq;
    sample_frame_seq += 2;
    memcpy(data.data, rx_buf[0] + 3, data.len);
    if (k_msgq_put(&msgq_ble_tx, &data, K_NO_WAIT) != 0)
    {
        printk("msgq is full! \n");
    }
}

int fpga_read_async(uint8_t addr, uint8_t len)
{
    // printk("fpga_read_async: %d\n", len);
    int err = 0;
    int next_rx = atomic_get(&rx_idx);

    struct spi_config config = fpga_dev.config;
    config.cs = (struct spi_cs_control) {
        .gpio = {
            .port = NULL,
            .pin = 0,
            .dt_flags = 0,
        },
        .delay = 0,
    };

    uint8_t tx_buffer[TX_BUF_SIZE] = {FPGA_R, addr, 0x00,};
    for (int i = 0; i < len; i++)
    {
        tx_buffer[i+3] = i+1;
    }

    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = len + 3,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    struct spi_buf rx_spi_buf = {
        .buf = rx_buf[next_rx],
        .len = len + 3,
    };
    struct spi_buf_set rx_spi_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1,
    };

    gpio_pin_set_dt(&ice_cs, 1);
    // k_busy_wait(10);
    err = spi_transceive_cb(fpga_dev.bus,
                            &config,
                            &tx_spi_buf_set,
                            &rx_spi_buf_set,
                            spi_complete_cb,
                            NULL);
    if (err < 0)
    {
        gpio_pin_set_dt(&ice_cs, 0);
        printk("spi_transceive_cb failed, err: %d\n", err);
    }

    return err;
}

/**
 * @brief read fpga
 *
 * @param addr
 * @param data
 * @param len
 * @return int
 */
int fpga_read(uint8_t addr, uint8_t *data, uint8_t len)
{
    int err = 0;

    uint8_t tx_buffer[TX_BUF_SIZE] = {FPGA_R, addr, 0x00,};
    for (int i = 0; i < len; i++)
    {
        tx_buffer[i+3] = i+1;
    }
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = len + 3,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };
    uint8_t rx_buffer[RX_BUF_SIZE];
    struct spi_buf rx_spi_buf = {
        .buf = rx_buffer,
        .len = len + 3,
    };
    struct spi_buf_set rx_spi_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1,
    };

    err = spi_transceive_dt(&fpga_dev, &tx_spi_buf_set, &rx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_transceive_dt failed, err: %d\n", err);
    }
    else
    {
        memmove(data, rx_buffer + 3, len);
        // printk("[%d]: %02x %02x %02x %02x %02x\n", len, data[0], data[1], data[2], data[3], data[4]);
    }

    return err;
}

/**
 * @brief write fpga
 *
 * @param addr
 * @param data
 * @param len
 * @return int
 */
int fpga_write(uint8_t addr, uint8_t *data, uint8_t len)
{
    int err = 0;

    uint8_t tx_buffer[TX_BUF_SIZE] = {FPGA_W, addr, };
    memmove(tx_buffer + 2, data, len);
    struct spi_buf tx_spi_buf = {
        .buf = tx_buffer,
        .len = len + 2,
    };
    struct spi_buf_set tx_spi_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1,
    };

    err = spi_write_dt(&fpga_dev, &tx_spi_buf_set);
    if (err < 0)
    {
        printk("spi_write_dt failed, err: %d\n", err);
    }

    return err;
}