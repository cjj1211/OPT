#include "FpgaInterface.h"

static volatile bool m_finished = false;

#define WAIT_FOR_PERIPH()   \
    do                      \
    {                       \
        while (!m_finished) \
        {                   \
        }                   \
        m_finished = false; \
    } while (0)

void spi_handler()//nrf_drv_qspi_evt_t event, void *p_context)
{
    
}
void spi_init()
{

  

}

void qspi_uninit()
{
   // nrf_drv_qspi_uninit();
}


uint32_t read_data(uint32_t addr, uint8_t *data, uint32_t length)
{
   
    m_finished = false;
   // err_code = nrf_drv_qspi_read(data, length, addr);
    WAIT_FOR_PERIPH();
    return  0x01;
}

uint32_t write_data(uint32_t addr, uint8_t *data, uint32_t length)
{
    uint32_t err_code = 0x01;//NRFX_SUCCESS;
    m_finished = false;
  //  err_code = nrf_drv_qspi_write(data, length, addr);
   ;
    WAIT_FOR_PERIPH();
    return err_code;
}
