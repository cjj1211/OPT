#include "FpgaInterface.h"
#include "nrf_log.h"
#include "nrf_drv_qspi.h"
#include <stdint.h>
#include "nrf_delay.h"
#include <string.h>
#include "nrf.h"
#include "bsp_btn_ble.h"
#include "nrfx_gpiote.h"
#include <cstdint>
#include "nrf_drv_gpiote.h"
#include "nrfx_qspi.h"
#include "nrf_mtx.h"
static volatile bool m_finished = false;

#define WAIT_FOR_PERIPH()   \
    do                      \
    {                       \
        while (!m_finished) \
        {                   \
        }                   \
        m_finished = false; \
    } while (0)

void qspi_handler(nrf_drv_qspi_evt_t event, void *p_context)
{
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(p_context);
    m_finished = true;
}
void qspi_init()
{
    uint32_t err_code;
    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;
    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("QSPI started.");
}

void qspi_uninit()
{
    nrf_drv_qspi_uninit();
}

void test_fpga()
{
    uint8_t init_data[QSPI_PAGE_SIZE] = {0x55, 0xAA, 0x12, 0x34};
    uint8_t buffer_rx[QSPI_PAGE_SIZE] = {0};
    uint8_t buffer_tx[QSPI_PAGE_SIZE] = {0x66, 0xBB, 0x56, 0x78};
    uint32_t initDataAddr = 0x00000002;
    uint32_t rwAddr = 0x00000005;
    uint32_t err_code;
    m_finished = false;
    int data_len = 4;
    err_code = read_data(initDataAddr, buffer_rx, data_len);
    APP_ERROR_CHECK(err_code);
    if (memcmp(init_data, buffer_rx, data_len) != 0)
    {
        NRF_LOG_DEBUG("recieved init data NOT required!");
    }
    nrf_delay_ms(10);
    NRF_LOG_INFO("Data read successfully!");
    err_code = write_data(rwAddr, buffer_tx, data_len);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Data write successfully!");
    nrf_delay_ms(20);
    err_code = read_data(rwAddr, buffer_rx, data_len);
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(10);
    NRF_LOG_INFO("Data read successfully!");
    NRF_LOG_INFO("Compare...");
    if (memcmp(buffer_tx, buffer_rx, data_len) == 0)
    {
        NRF_LOG_INFO("Data consistent");
    }
    else
    {
        NRF_LOG_INFO("Data inconsistent");
    }
}

uint32_t read_data(uint32_t addr, uint8_t *data, uint32_t length)
{
    uint32_t err_code = NRFX_SUCCESS;
    m_finished = false;
    err_code = nrf_drv_qspi_read(data, length, addr);
    WAIT_FOR_PERIPH();
    return err_code;
}

uint32_t write_data(uint32_t addr, uint8_t *data, uint32_t length)
{
    uint32_t err_code = NRFX_SUCCESS;
    m_finished = false;
    err_code = nrf_drv_qspi_write(data, length, addr);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    return err_code;
}
