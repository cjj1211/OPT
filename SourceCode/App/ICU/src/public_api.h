#ifndef __PUBLIC_API_H__
#define __PUBLIC_API_H__

#include <stdatomic.h>
#include <zephyr/drivers/adc.h>
#include "HardwareDefine.h"
#include "G3_Protocol.h"
#include "G3_ProtocolImplBLEPeripheral.h"

#define RD (0x00)
#define WR (0x01)

//!< define timer
#define TIMER0_MS 1000
#define BLE_HB_TIMER_MS 1000  //!< BLE Heartbeat timer
#define STM_HB_TIMER_MS 1000  //!< STM Heartbeat timer
#define AUTH_TIMER_MS (1000 * 30)
#if defined(CONFIG_BT_NUS_IMP_MEA_MOCK_TEST)
#define IMP_MEA_MS (1000 * 10)
#define IMP_MOCK_TEST_MS (5)
#else
#define IMP_MEA_MS (1000 * 5)
#endif
#define IMP_MEA_DELAY_STOP (100)

#define CAP_NUM  3
#define CHAN_NUM 64
#define IMPEDANCE_FRAME_BYTES  102
#define IMPEDANCE_FRAME_RAW_BYTES  100
#define IMPEDANCE_FRAME_COUNT  2

struct msgq_sample_data_t
{
    uint16_t seq;
    uint8_t  len;
    uint8_t  type;
    uint8_t  data[204];
};

extern uint8_t chans_select;
extern atomic_bool g_sampling_started;
extern atomic_bool g_imp_data_read;
extern atomic_bool g_ble_connected;
extern atomic_bool g_chrg_triggered;
extern atomic_char g_icu_status;
extern volatile uint16_t sample_frame_seq;
extern const struct spi_dt_spec fpga_dev;
extern const struct gpio_dt_spec ice_cs;
extern const struct gpio_dt_spec ice_reset;
extern const struct i2c_dt_spec thermal_dev;
extern const struct adc_dt_spec adc_channels[];
extern const struct device *uart;
extern struct k_sem sem_uart_tx_done;
extern struct k_sem sem_adc_rdy;
extern struct k_timer mock_test_timer;
extern struct k_timer sleep_timer;
extern struct k_timer stm_heartbeat_timer;
extern struct k_timer imp_measure_timer;
extern struct k_msgq msgq_ble_tx;
extern struct k_work notify_sleep_work;
extern struct k_work notify_sync_work;
extern struct k_work get_version_work;
extern const k_tid_t fpga_data_producer_thread_id;
extern const k_tid_t fpga_data_consumer_thread_id;

extern void update_le_param_custom(void);
extern void ble_adv_name_update(void);
extern IcuStatus getIcuStatus(void);
extern void swap_priorities(k_tid_t t1, k_tid_t t2);

#endif /* __PUBLIC_API_H__ */