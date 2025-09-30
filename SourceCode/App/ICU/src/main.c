/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */
#include "uart_async_adapter.h"
#include "public_api.h"
#include "nx_crc.h"
#include "dfu.h"
#include "rtc.h"
#include "info.h"
#include "sensor.h"
#include "events.h"

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>
#include <zephyr/usb/usb_device.h>
#include <hal/nrf_clock.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <zephyr/settings/settings.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include <stdio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <stdatomic.h>
#include "driver/bsp_gpio.h"
#include "driver/bsp_ice40.h"
#include "driver/bsp_tmp119.h"
#include <G3_Protocol.h>
#include "G3_ProtocolInterface.h"
#include "G3_ProtocolImplBLEPeripheral.h"
#include "G3_ProtocolImplUart.h"
#include "HardwareDefine.h"


#define LOG_MODULE_NAME OPT_NUS
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7
#define FPGA_DATA_PRODUCER_PRIORITY 6
#define FPGA_DATA_CONSUMER_PRIORITY 5

#define INTERVAL_MIN    0x06  //!< 6 units, 7.5ms, only used to setup connection
#define INTERVAL_MAX    0x06  //!< 6 units, 7.5ms, only used to setup connection

#define RUN_IDLE_INTERVAL 1000

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

static K_SEM_DEFINE(sem_ble_init_ok, 0, 1);
K_SEM_DEFINE(sem_uart_tx_done, 0, 1);
K_SEM_DEFINE(sem_adc_rdy, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;
static struct bt_gatt_exchange_params exchange_params;
static struct bt_le_conn_param *conn_param =
    BT_LE_CONN_PARAM(INTERVAL_MIN, INTERVAL_MAX, 0, 400);

//!< ADC
#define DT_SPEC_AND_COMMA(node_id, prop, idx)  ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                 DT_SPEC_AND_COMMA)
};
//!< watchdog
static const struct device *wdt_dev = DEVICE_DT_GET(DT_NODELABEL(wdt0));
int wdt_channel_id;
//!< FPGA
#define SPIOP SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_LSB
const struct spi_dt_spec fpga_dev = SPI_DT_SPEC_GET(DT_NODELABEL(ice40), SPIOP, 0);
const struct gpio_dt_spec ice_cs = GPIO_DT_SPEC_GET(DT_NODELABEL(spi4), cs_gpios);

const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(nordic_nus_uart));
static struct k_work_delayable uart_work;

struct uart_data_t
{
    void *fifo_reserved;
    uint8_t data[UART_BUF_SIZE];
    uint16_t len;
};

struct ble_data_t
{
    void *fifo_reserved;
    uint8_t data[MAX_BLE_FRAME_SIZE];
    uint16_t len;
};

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_ble_tx_data);

K_MSGQ_DEFINE(msgq_ble_tx, sizeof(struct msgq_sample_data_t), 500, 4);
struct msgq_ble_data_t
{
    uint16_t len;
    uint16_t seq;
    uint8_t  data[244];
};
K_MSGQ_DEFINE(msgq_ble_rx, sizeof(struct msgq_ble_data_t), 5, 4);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

#if CONFIG_BT_NUS_UART_ASYNC_ADAPTER
UART_ASYNC_ADAPTER_INST_DEFINE(async_adapter);
#else
static const struct device *const async_adapter;
#endif


atomic_bool g_sampling_started = ATOMIC_VAR_INIT(false);
atomic_bool g_imp_data_read = ATOMIC_VAR_INIT(false);
atomic_bool g_ble_connected = ATOMIC_VAR_INIT(false);
atomic_bool g_host_auth = ATOMIC_VAR_INIT(false);
atomic_bool g_chrg_triggered = ATOMIC_VAR_INIT(false);
atomic_char g_icu_status = ATOMIC_VAR_INIT(ICU_IDLE);
volatile uint16_t sample_frame_seq = 0;
static uint8_t fpga_data[SPI_MAX_FRAME_SIZE] = {0};
static uint8_t imp_matrix[CAP_NUM][CHAN_NUM] = {0};
static uint8_t imp_measure_count = 0;

void swap_priorities(k_tid_t t1, k_tid_t t2)
{
    int prio_t1 = k_thread_priority_get(t1);
    int prio_t2 = k_thread_priority_get(t2);

    k_sched_lock();
    k_thread_priority_set(t1, prio_t2);
    k_thread_priority_set(t2, prio_t1);
    k_sched_unlock();

    printk("Priorities swapped: Producer now %d, Consumer now %d\n",
           k_thread_priority_get(t1), k_thread_priority_get(t2));
}

uint32_t nrf5340_get_cpuapp_frequency(void)
{
    nrf_clock_hfclk_t clk_src = nrf_clock_hf_src_get(NRF_CLOCK);

    uint32_t div = nrf_clock_hfclk_div_get(NRF_CLOCK);
    uint32_t div_factor = (div == 0) ? 1 : (1UL << div);
    LOG_DBG("clk_src / div: %d / %d\n", clk_src, div_factor);

    switch (clk_src) {
        case NRF_CLOCK_HFCLK_HIGH_ACCURACY:
            return 128000000 / div_factor;

        case NRF_CLOCK_HFCLK_LOW_ACCURACY:
            return 64000000 / div_factor;

        default:
            printk("Warning: unknown clock\n");
            return 0;
    }
}

IcuStatus getIcuStatus(void)
{
    if (atomic_load(&g_sampling_started))
    {
        atomic_store(&g_icu_status, ICU_SAMPLING);
        return ICU_SAMPLING;
    }
    else
    {
        return ICU_IDLE;
    }
}

void ble_adv_name_update(void)
{
    struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME | BT_LE_ADV_OPT_FORCE_NAME_IN_AD,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL
    );

    device_name_load();
    bt_le_adv_stop();
    int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
    }
}

static const char *phy2str(uint8_t phy)
{
    switch (phy) {
    case BT_GAP_LE_PHY_NONE: return "No phy is set";
    case BT_GAP_LE_PHY_1M: return "LE 1M";
    case BT_GAP_LE_PHY_2M: return "LE 2M";
    case BT_GAP_LE_PHY_CODED: return "LE Coded";
    default: return "Unknown";
    }
}

static void update_le_param(struct bt_conn *conn)
{
    int err;
    err = bt_conn_le_param_update(conn, conn_param);
    if (err) {
        LOG_ERR("Connection paramters update failed: %d", err);
    }
}

void update_le_param_custom(void)
{
    conn_param->interval_max = 12;
    conn_param->interval_min = 12;
    update_le_param(current_conn);
}

static void update_phy(struct bt_conn *conn)
{
    int err;
    const struct bt_conn_le_phy_param preferred_phy = {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };
    err = bt_conn_le_phy_update(conn, &preferred_phy);
    if (err) {
        LOG_ERR("PHY update failed %d", err);
    }
}

static void update_data_length(struct bt_conn *conn)
{
    int err;
    struct bt_conn_le_data_len_param my_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };
    err = bt_conn_le_data_len_update(conn, &my_data_len);
    if (err) {
        LOG_ERR("LE data length update failed: %d", err);
    }
}

static void exchange_func(struct bt_conn *conn, uint8_t att_err,
              struct bt_gatt_exchange_params *params)
{
    LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");
    if (!att_err) {
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;   // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
}

static void update_mtu(struct bt_conn *conn)
{
    int err;
    exchange_params.func = exchange_func;

    err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (err) {
        LOG_ERR("MTU exchange failed: %d", err);
    }
}

static void ble_heartbeat_work_handler(struct k_work* work)
{
    static int seq = 0;

    LOG_DBG("BLE Send Heatbeat: %d", seq);

    Frame frmHeartBeat;
    frmHeartBeat.header = HEADER;
    frmHeartBeat.seq = seq++;
    frmHeartBeat.type = FRAME_HEARTBEAT;
    frmHeartBeat.dataLen = 0x0000;

    // if (current_conn && !g_sampling_started)
    // {
    //     // LOG_INF("Send Heatbeat over BLE");
    //     // ProtocolErrorCode ret = sendFrame(&frmHeartBeat);
    //     struct ble_data_t *tx = k_malloc(sizeof(*tx));
    //     tx->len = FRAME_HEADER_LEN;
    //     memcpy(tx->data, &frmHeartBeat, tx->len);
    //     tx->data[tx->len] = 0;
    //     tx->len++;
    //     k_fifo_put(&fifo_ble_tx_data, tx);
    // }
}

static void stm_heartbeat_work_handler(struct k_work* work)
{
    static int seq = 0;
    struct stm32_sensor_data sync_value;
    struct msgq_uart_ack_t ele;

    LOG_DBG("STM32 Send Heatbeat: %d", seq);

    g_stm32_talk = true;

    uint8_t uart_data[COMMON_FRAME_SIZE] = {0};
    Frame *p_uartframe = (Frame *)uart_data;
    p_uartframe->header = HEADER;
    p_uartframe->seq = seq++;
    p_uartframe->type = FRAME_HEARTBEAT;
    p_uartframe->dataLen = 0x00;
    sendUartFrame(p_uartframe);

    //!< The actual measurement requires approximately 60 milliseconds
    if (k_msgq_get(&msgq_uart_rx, &ele, K_MSEC(100)) == 0) {
        LOG_HEXDUMP_DBG(ele.data, ele.len, "STM_HB");
        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
        uint8_t type = ele.data[4];
        if ((ack_head == HEADER) && (type == FRAME_SENSOR_GET)) {
            sync_value.shell_temp = (ele.data[8] << 8) | ele.data[9];
            sync_value.batt_curr = (ele.data[10] << 8) | ele.data[11];
            sync_value.batt_volt = (ele.data[12] << 8) | ele.data[13];
            sync_value.ptx_volt = (ele.data[14] << 8) | ele.data[15];
            smart_buffer_write(&sync_value);
            LOG_DBG("from stm32: %d, %d, %d, %d", sync_value.shell_temp,
                                                  sync_value.batt_curr,
                                                  sync_value.batt_volt,
                                                  sync_value.ptx_volt);
        }
    }

    g_stm32_talk = false;
}

static void notify_ble_status_work_handler(struct k_work* work)
{
    g_stm32_talk = true;
    struct msgq_uart_ack_t ele;
    uint8_t uart_data[COMMON_FRAME_SIZE] = {0};
    Frame *p_uartframe = (Frame *)uart_data;
    p_uartframe->header = HEADER;
    p_uartframe->type = FRAME_BLE_STATUS;
    p_uartframe->dataLen = 0x01;
    p_uartframe->data[0] = 0x00;
    if (atomic_load(&g_ble_connected)) {
        p_uartframe->data[0] = 0x01;
    }
    sendUartFrame(p_uartframe);

    //!< The actual measurement requires approximately 60 milliseconds
    if (k_msgq_get(&msgq_uart_rx, &ele, K_MSEC(100)) == 0) {
    }
    g_stm32_talk = false;
}

static void notify_sync_limits_work_handler(struct k_work* work)
{
    g_stm32_talk = true;
    struct msgq_uart_ack_t ele;
    uint8_t uart_data[COMMON_FRAME_SIZE] = {0};
    Frame *p_uartframe = (Frame *)uart_data;
    p_uartframe->header = HEADER;
    p_uartframe->type = FRAME_SYNC_LIMIT;
    p_uartframe->dataLen = 0x06;

    int16_t discharge_curr = sensor_limit_get(LIMIT_BAT_DISCAHRGE_CURR);
    p_uartframe->data[0] = discharge_curr;
    p_uartframe->data[1] = discharge_curr >> 8;
    int16_t temp_soft = sensor_limit_get(LIMIT_CASE_TEMP_SOFT);
    p_uartframe->data[2] = temp_soft;
    p_uartframe->data[3] = temp_soft >> 8;
    int16_t temp_hard = sensor_limit_get(LIMIT_CASE_TEMP_HARD);
    p_uartframe->data[4] = temp_hard;
    p_uartframe->data[5] = temp_hard >> 8;

    sendUartFrame(p_uartframe);

    //!< The actual measurement requires approximately 60 milliseconds
    if (k_msgq_get(&msgq_uart_rx, &ele, K_MSEC(100)) == 0) {
    }
    g_stm32_talk = false;
}

static void notify_sleep_work_handler(struct k_work* work)
{
    g_stm32_talk = true;
    struct msgq_uart_ack_t ele;
    uint8_t data[COMMON_FRAME_SIZE] = {0};
    Frame *p_uartframe = (Frame *)data;
    p_uartframe->header = HEADER;
    p_uartframe->type = FRAME_CLOSEICU;
    p_uartframe->dataLen = 0;
    sendUartFrame(p_uartframe);

    //!< The actual measurement requires approximately 60 milliseconds
    if (k_msgq_get(&msgq_uart_rx, &ele, K_MSEC(100)) == 0) {
        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
        uint8_t type = ele.data[4];
        if ((ack_head == HEADER) && (type == FRAME_CLOSEICU)) {
        }
        else
        {
            k_work_submit(&notify_sleep_work);
        }
    }
    else
    {
        if (current_conn == NULL) {
            k_work_submit(&notify_sleep_work);
        }
    }
    g_stm32_talk = false;
}

static void recv_stm32_work_handler(struct k_work* work)
{
    struct msgq_uart_ack_t ele;

    if (k_msgq_get(&msgq_sync_uart_rx, &ele, K_MSEC(1000)) == 0) {
        LOG_HEXDUMP_DBG(ele.data, ele.len, "SYNC_STM32");
        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
        uint8_t type = ele.data[4];
        if ((ack_head == HEADER) && (type == FRAME_EVT_NOTICE))
        {
            //!< EVT_OVERTEMP
            if (ele.data[8] == 0x00) {
                struct event evt = {
                    .level = EVENT_CRITICAL,
                    .type = EVENT_CRIT_CASE_TEMP_OVERHEAT,
                    .timestamp = rtc_calibrated_time_get(rtc_dev),
                    .data = ele.data[9] | (ele.data[10] << 8),
                };
                zbus_chan_pub(&critical_chan, &evt, K_MSEC(10));
            }
            //!< EVT_CHRGSTAT
            if (ele.data[8] == 0x01) {
                if (ele.data[9] == 0x00) {
                    atomic_store(&g_chrg_triggered, false);
                } else {
                    atomic_store(&g_chrg_triggered, true);
                }
            }
        }
    }
}

static void get_version_work_handler(struct k_work* work)
{
    uint8_t data[COMMON_FRAME_SIZE] = {0};
    Frame *frmAck = (Frame *)data;
    frmAck->header = HEADER;
    frmAck->seq = getBleSendSeq();
    frmAck->type = FRAME_GET_VERSION;
    frmAck->dataLen = 0x05;
    frmAck->data[0] = 0x00;
    frmAck->data[1] = 0xff;
    frmAck->data[2] = 0xff;
    frmAck->data[3] = 0xff;
    frmAck->data[4] = 0xff;

    g_stm32_talk = true;
    struct msgq_uart_ack_t ele;
    uint8_t uart_data[COMMON_FRAME_SIZE] = {0};
    Frame *p_uartframe = (Frame *)uart_data;
    p_uartframe->header = HEADER;
    p_uartframe->type = FRAME_VERSION_GET;
    p_uartframe->dataLen = 0;
    sendUartFrame(p_uartframe);

    //!< The actual measurement requires approximately 60 milliseconds
    if (k_msgq_get(&msgq_uart_rx, &ele, K_MSEC(100)) == 0) {
        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
        uint8_t type = ele.data[4];
        if ((ack_head == HEADER) && (type == FRAME_VERSION_GET)) {
            if (ele.data[8] == 0) {
                frmAck->data[1] = 0x00;
                frmAck->data[2] = 0x00;
                frmAck->data[3] = 0x07;
                frmAck->data[4] = 0x1f;
            }
            else if (ele.data[8] == 1) {
                frmAck->data[1] = ele.data[9];
                frmAck->data[2] = ele.data[10];
                frmAck->data[3] = ele.data[11];
                frmAck->data[4] = ele.data[12];
            }
            else {
                frmAck->data[1] = 0xff;
                frmAck->data[2] = 0xff;
                frmAck->data[3] = 0xff;
                frmAck->data[4] = 0xff;
            }
        }
    }
    g_stm32_talk = false;

    sendFrame(frmAck);
}

#if defined(CONFIG_BT_NUS_HOST_AUTH_ENABLED)
static void host_auth_work_handler(struct k_work* work)
{
    if (!atomic_load(&g_host_auth)) {
        bt_conn_disconnect(current_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        LOG_INF("NOT recv auth command, disconnect");
    }
}
//!< application layer authentication
K_WORK_DEFINE(host_auth_work, host_auth_work_handler);
#endif

static void swap_prio_work_handler(struct k_work* work)
{
    swap_priorities(fpga_data_producer_thread_id, fpga_data_consumer_thread_id);
}

//!< Heartbeat
K_WORK_DEFINE(ble_heartbeat_work, ble_heartbeat_work_handler);
K_WORK_DEFINE(stm_heartbeat_work, stm_heartbeat_work_handler);
//!< Notify STM32 the connection status of BLE
K_WORK_DEFINE(notify_ble_status_work, notify_ble_status_work_handler);
//!< Notify STM32 to enter stop mode
K_WORK_DEFINE(notify_sleep_work, notify_sleep_work_handler);
//!< Notify STM32 to sync system limits
K_WORK_DEFINE(notify_sync_work, notify_sync_limits_work_handler);
//!< Synchronize the sensor data sent by STM32
K_WORK_DEFINE(recv_stm32_work, recv_stm32_work_handler);
//!< Get STM32 Firmware Version
K_WORK_DEFINE(get_version_work, get_version_work_handler);
//!< Swap thread priority
K_WORK_DEFINE(swap_prio_work, swap_prio_work_handler);


static void mock_test_timeout_handler(struct k_timer* timer)
{
    // uint32_t now = sys_clock_cycle_get_32();
    // LOG_INF("timer at %d ticks\n", now);
    k_sem_give(&sem_fpga_read);
}

static void ble_heartbeat_timeout_handler(struct k_timer* timer)
{
    // uint32_t now = sys_clock_cycle_get_32();
    // LOG_INF("heartbeat at %d ticks\n", now);
    k_work_submit(&ble_heartbeat_work);
}

static void stm_heartbeat_timeout_handler(struct k_timer* timer)
{
    k_work_submit(&stm_heartbeat_work);
}

static void sleep_timeout_handler(struct k_timer* timer)
{
    struct msgq_ble_data_t ble_data;
    ble_data.len = 8;
    ble_data.data[0] = 0x55;
    ble_data.data[1] = 0xAA;
    ble_data.data[2] = 0x00;
    ble_data.data[3] = 0x00;
    ble_data.data[4] = 0x25;
    ble_data.data[5] = 0x00;
    ble_data.data[6] = 0x00;
    ble_data.data[7] = 0x00;
    if (k_msgq_put(&msgq_ble_rx, &ble_data, K_NO_WAIT) != 0)
    {
        LOG_DBG("msgq_ble_rx is full!");
    }
}

static void imp_measure_timeout_handler(struct k_timer* timer)
{
    fpga_pwr_disable();
    intan_pwr_disable();

    // k_work_submit(&swap_prio_work);

    atomic_store(&g_sampling_started, false);
    atomic_store(&g_imp_data_read, false);

    if (imp_measure_count != CAP_NUM * CHAN_NUM)
    {
        LOG_ERR("Impedance measurement lost channel: %d", imp_measure_count);
    }

    memset((void*)imp_matrix, 0, CAP_NUM * CHAN_NUM);
    imp_measure_count = 0;

    LOG_INF("IMP MEA TIMEOUT");
}

K_TIMER_DEFINE(mock_test_timer, mock_test_timeout_handler, NULL);
K_TIMER_DEFINE(ble_heartbeat_timer, ble_heartbeat_timeout_handler, NULL);
K_TIMER_DEFINE(stm_heartbeat_timer, stm_heartbeat_timeout_handler, NULL);
//!< Timer for timeout to enter sleep mode
K_TIMER_DEFINE(sleep_timer, sleep_timeout_handler, NULL);
//!< Impedance measurement timeout timer
K_TIMER_DEFINE(imp_measure_timer, imp_measure_timeout_handler, NULL);

#if defined(CONFIG_BT_NUS_HOST_AUTH_ENABLED)
static void auth_timeout_handler(struct k_timer* timer)
{
    k_work_submit(&host_auth_work);
}
K_TIMER_DEFINE(auth_timer, auth_timeout_handler, NULL);

static void AuthFrameCheck(const uint8_t *p_data, uint16_t len)
{
    LOG_HEXDUMP_INF(p_data, len, "AuthFrame");
    Frame *frame = (Frame *)p_data;
    if (frame->header == HEADER && frame->type == FRAME_SEND_PASSWORD)
    {
        k_timer_stop(&auth_timer);
        atomic_store(&g_host_auth, true);
        k_timer_start(&ble_heartbeat_timer, K_MSEC(BLE_HB_TIMER_MS), K_MSEC(BLE_HB_TIMER_MS));
        LOG_INF("Auth success");
    }
}
#endif

static void wdt_callback(const struct device *dev, int channel_id)
{
    LOG_INF("Watchdog feed fail, reset");
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    ARG_UNUSED(dev);

    static size_t aborted_len;
    struct uart_data_t *buf;
    static uint8_t *aborted_buf;
    static bool disable_req;

    switch (evt->type) {
    case UART_TX_DONE:
        LOG_DBG("UART_TX_DONE");
        if (g_stm32_dfu_enter || g_stm32_talk)
        {
            k_sem_give(&sem_uart_tx_done);
            return;
        }

        if ((evt->data.tx.len == 0) ||
            (!evt->data.tx.buf)) {
            return;
        }

        if (aborted_buf) {
            buf = CONTAINER_OF(aborted_buf, struct uart_data_t,
                       data[0]);
            aborted_buf = NULL;
            aborted_len = 0;
        } else {
            buf = CONTAINER_OF(evt->data.tx.buf, struct uart_data_t,
                       data[0]);
        }

        k_free(buf);

        buf = k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
        if (!buf) {
            return;
        }

        if (uart_tx(uart, buf->data, buf->len, SYS_FOREVER_MS)) {
            LOG_WRN("Failed to send data over UART");
        }

        break;

    case UART_RX_RDY:
        LOG_DBG("UART_RX_RDY");
        buf = CONTAINER_OF(evt->data.rx.buf, struct uart_data_t, data[0]);
        buf->len += evt->data.rx.len;

        if (disable_req) {
            return;
        }

        disable_req = true;
        uart_rx_disable(uart);

        break;

    case UART_RX_DISABLED:
        LOG_DBG("UART_RX_DISABLED");
        disable_req = false;

        buf = k_malloc(sizeof(*buf));
        if (buf) {
            buf->len = 0;
        } else {
            LOG_WRN("Not able to allocate UART receive buffer");
            k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
            return;
        }

        uart_rx_enable(uart, buf->data, sizeof(buf->data),
                   UART_WAIT_FOR_RX);

        break;

    case UART_RX_BUF_REQUEST:
        LOG_DBG("UART_RX_BUF_REQUEST");
        buf = k_malloc(sizeof(*buf));
        if (buf) {
            buf->len = 0;
            uart_rx_buf_rsp(uart, buf->data, sizeof(buf->data));
        } else {
            LOG_WRN("Not able to allocate UART receive buffer");
        }

        break;

    case UART_RX_BUF_RELEASED:
        LOG_DBG("UART_RX_BUF_RELEASED");
        buf = CONTAINER_OF(evt->data.rx_buf.buf, struct uart_data_t,
                   data[0]);

        if (buf->len > 0) {
            LOG_HEXDUMP_DBG(buf->data, buf->len, "Recv_Uart");
            if (g_stm32_dfu_enter)
            {
                struct msgq_uart_ack_t uart_data;
                uart_data.len = buf->len;
                memcpy(uart_data.data, buf->data, buf->len);
                k_msgq_put(&msgq_dfu_uart_rx, &uart_data, K_NO_WAIT);
            }
            else if (g_stm32_talk)
            {
                struct msgq_uart_ack_t uart_data;
                uart_data.len = buf->len;
                memcpy(uart_data.data, buf->data, buf->len);
                k_msgq_put(&msgq_uart_rx, &uart_data, K_NO_WAIT);
            }
            else {
                struct msgq_uart_ack_t uart_data;
                uart_data.len = buf->len;
                memcpy(uart_data.data, buf->data, buf->len);
                k_msgq_put(&msgq_sync_uart_rx, &uart_data, K_NO_WAIT);
                k_work_submit(&recv_stm32_work);
            }
            k_free(buf);
        } else {
            k_free(buf);
        }

        break;

    case UART_TX_ABORTED:
        LOG_DBG("UART_TX_ABORTED");
        if (!aborted_buf) {
            aborted_buf = (uint8_t *)evt->data.tx.buf;
        }

        aborted_len += evt->data.tx.len;
        buf = CONTAINER_OF((void *)aborted_buf, struct uart_data_t,
                   data);

        uart_tx(uart, &buf->data[aborted_len],
            buf->len - aborted_len, SYS_FOREVER_MS);

        break;

    default:
        break;
    }
}

static void uart_work_handler(struct k_work *item)
{
    struct uart_data_t *buf;

    buf = k_malloc(sizeof(*buf));
    if (buf) {
        buf->len = 0;
    } else {
        LOG_WRN("Not able to allocate UART receive buffer");
        k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
        return;
    }

    uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);
}

static bool uart_test_async_api(const struct device *dev)
{
    const struct uart_driver_api *api =
            (const struct uart_driver_api *)dev->api;

    return (api->callback_set != NULL);
}

static int uart_init(void)
{
    int err;
    struct uart_data_t *rx;
    // struct uart_data_t *tx;

    if (!device_is_ready(uart)) {
        return -ENODEV;
    }

    if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
        err = usb_enable(NULL);
        if (err && (err != -EALREADY)) {
            LOG_ERR("Failed to enable USB");
            return err;
        }
    }

    rx = k_malloc(sizeof(*rx));
    if (rx) {
        rx->len = 0;
    } else {
        return -ENOMEM;
    }

    k_work_init_delayable(&uart_work, uart_work_handler);


    if (IS_ENABLED(CONFIG_BT_NUS_UART_ASYNC_ADAPTER) && !uart_test_async_api(uart)) {
        /* Implement API adapter */
        uart_async_adapter_init(async_adapter, uart);
        uart = async_adapter;
    }

    err = uart_callback_set(uart, uart_cb, NULL);
    if (err) {
        k_free(rx);
        LOG_ERR("Cannot initialize UART callback");
        return err;
    }

    if (IS_ENABLED(CONFIG_UART_LINE_CTRL)) {
        LOG_INF("Wait for DTR");
        while (true) {
            uint32_t dtr = 0;

            uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr);
            if (dtr) {
                break;
            }
            /* Give CPU resources to low priority threads. */
            k_sleep(K_MSEC(100));
        }
        LOG_INF("DTR set");
        err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DCD, 1);
        if (err) {
            LOG_WRN("Failed to set DCD, ret code %d", err);
        }
        err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DSR, 1);
        if (err) {
            LOG_WRN("Failed to set DSR, ret code %d", err);
        }
    }

    err = uart_rx_enable(uart, rx->data, sizeof(rx->data), UART_WAIT_FOR_RX);
    if (err) {
        LOG_ERR("Cannot enable uart reception (err: %d)", err);
        /* Free the rx buffer only because the tx buffer will be handled in the callback */
        k_free(rx);
    }

    return err;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    const bt_addr_le_t *bt_addr = bt_conn_get_dst(conn);
    bt_addr_le_to_str(bt_addr, addr, sizeof(addr));
    LOG_INF("Connected %s", addr);

    current_conn = bt_conn_ref(conn);

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
    err = bt_conn_set_security(current_conn, BT_SECURITY_L2);
    LOG_INF("Set Security Level: %d", err);
#endif
    get_current_conn(current_conn);
    update_phy(current_conn);
    update_data_length(current_conn);
    update_mtu(current_conn);
    struct bt_conn_info info;
    err = bt_conn_get_info(current_conn, &info);
    if (err) {
        LOG_ERR("bt_conn_get_info() returned %d", err);
    }
    else
    {
        LOG_INF("interval: %.2f ms, tx_len: %d", info.le.interval * 1.25, info.le.data_len->tx_max_len);
        if (info.le.interval != INTERVAL_MIN)
        {
            update_le_param(current_conn);
        }
    }

#if defined(CONFIG_BT_NUS_HOST_AUTH_ENABLED)
    k_timer_start(&auth_timer, K_MSEC(AUTH_TIMER_MS), K_MSEC(AUTH_TIMER_MS));
#else
    k_timer_start(&ble_heartbeat_timer, K_MSEC(BLE_HB_TIMER_MS), K_MSEC(BLE_HB_TIMER_MS));
#endif
    k_timer_stop(&sleep_timer);
    atomic_store(&g_ble_connected, true);
    k_work_submit(&notify_ble_status_work);

    struct event evt = {
        .level = EVENT_NORMAL,
        .type = EVENT_NORM_BLE_CONNECTED,
        .timestamp = rtc_calibrated_time_get(rtc_dev),
    };
    zbus_chan_pub(&normal_chan, &evt, K_MSEC(10));
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Disconnected: %s (reason %u)", addr, reason);

    // bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(current_conn));

    if (auth_conn) {
        bt_conn_unref(auth_conn);
        auth_conn = NULL;
    }

    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }

#if defined(CONFIG_BT_NUS_HOST_AUTH_ENABLED)
    k_timer_stop(&auth_timer);
    atomic_store(&g_host_auth, false);
#endif
    k_timer_stop(&ble_heartbeat_timer);
    atomic_store(&g_ble_connected, false);
    k_timer_start(&sleep_timer, K_MSEC(current_sleep_time), K_MSEC(current_sleep_time));
    k_work_submit(&notify_ble_status_work);

    // uint8_t fpga_data[] = {0x00, 0x00, 0x00, 0x00};
    // fpga_write(TrigInSpiStart, fpga_data, sizeof(fpga_data));
    // atomic_store(&g_sampling_started, false);

    // fpga_pwr_disable();
    intan_pwr_disable();

    // int data;
    // while (k_msgq_get(&msgq_ble_tx, &data, K_NO_WAIT) == 0);

    struct event evt = {
        .level = EVENT_NORMAL,
        .type = EVENT_NORM_BLE_DISCONNECTED,
        .timestamp = rtc_calibrated_time_get(rtc_dev),
        .data = reason,
    };
    zbus_chan_pub(&normal_chan, &evt, K_MSEC(10));
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
    LOG_INF("Connection parameters update request received. \n");
    LOG_INF("Minimum interval: %d, Maximum interval: %d\n", param->interval_min, param->interval_max);
    LOG_INF("Latency: %d, Timeout: %d\n", param->latency, param->timeout);
    return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    double connection_interval = interval * 1.25;
    LOG_INF("Connection parameters updated.\n"
            " interval: %.2f ms, latency: %d intervals, timeout: %d ms",
            connection_interval, latency, timeout * 10);
}

static void le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    LOG_INF("LE PHY updated: TX PHY %s, RX PHY %s",
            phy2str(param->tx_phy), phy2str(param->rx_phy));
}

static void le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
    LOG_INF("LE data len updated: TX (len: %d time: %d)"
            " RX (len: %d time: %d)", info->tx_max_len,
            info->tx_max_time, info->rx_max_len, info->rx_max_time);
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
                 enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err) {
        LOG_INF("Security changed: %s level %u", addr, level);
    } else {
        LOG_WRN("Security failed: %s level %u err %d", addr,
            level, err);
    }
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
    .le_param_req = le_param_req,
    .le_param_updated = le_param_updated,
    .le_phy_updated  = le_phy_updated,
    .le_data_len_updated = le_data_len_updated,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
    .security_changed = security_changed,
#endif
};

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Pairing completed: %s, bonded: %d", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    const bt_addr_le_t *bt_addr = bt_conn_get_dst(conn);
    bt_addr_le_to_str(bt_addr, addr, sizeof(addr));

    LOG_INF("Pairing failed conn: %s, reason %d", addr, reason);

    bt_unpair(BT_ID_DEFAULT, bt_addr);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = NULL,
    .passkey_confirm = NULL,
    .cancel = NULL,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed
};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
static struct bt_conn_auth_info_cb conn_auth_info_callbacks;

#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
              uint16_t len)
{
    char addr[BT_ADDR_LE_STR_LEN] = {0};

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
    struct msgq_ble_data_t ble_data;
    ble_data.len = len;
    memcpy(ble_data.data, data, len);
    LOG_HEXDUMP_DBG(ble_data.data, len, "bt_receive_cb");

    if (k_msgq_put(&msgq_ble_rx, &ble_data, K_NO_WAIT) != 0)
    {
        LOG_DBG("msgq_ble_rx is full!");
    }
    else
    {
        LOG_DBG("msgp_ble_rx push");
    }
}

static struct bt_nus_cb nus_cb = {
    .received = bt_receive_cb,
};

void error(void)
{
    while (true) {
        /* Spin for ever */
        k_sleep(K_MSEC(1000));
    }
}

const struct device *getDevice()
{
    const struct device *gpio_dev;
    gpio_dev = device_get_binding("GPIO_0");

    if (!gpio_dev)
    {
        LOG_INF("GPIO device not found!\n");
        return NULL;
    }
    else
    {
        LOG_INF("GPIO device has found!\n");
        return gpio_dev;
    }
}

int main(void)
{
    int err = 0;

    info_init();
    configure_gpio();
    initProtocol();
    override_buffer_init();
    err = uart_init();
    if (err) {
        error();
    }

    if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED)) {
        err = bt_conn_auth_cb_register(&conn_auth_callbacks);
        if (err) {
            printk("Failed to register authorization callbacks.\n");
            return 0;
        }

        err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
        if (err) {
            printk("Failed to register authorization info callbacks.\n");
            return 0;
        }
    }

    err = bt_enable(NULL);
    if (err) {
        error();
    }

    LOG_INF("Bluetooth initialized");

    k_sem_give(&sem_ble_init_ok);

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    // uint8_t mac[8];
    // get_device_id(mac);
    // LOG_INF("MAC address: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
    //         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);

    // print_bt_address();

    err = bt_nus_init(&nus_cb);
    if (err) {
        LOG_ERR("Failed to initialize UART service (err: %d)", err);
        return 0;
    }

    device_name_load();
    struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME | BT_LE_ADV_OPT_FORCE_NAME_IN_AD,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL
    );
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return 0;
    }

    /* Configure adc */
    if (!adc_is_ready_dt(&adc_channels[0]))
    {
        LOG_ERR("ADC controller device %s not ready\n", adc_channels[0].dev->name);
    }
    else
    {
        LOG_INF("adc_intan ready\n");
    }
    err = adc_channel_setup_dt(&adc_channels[0]);
    if (err < 0)
    {
        LOG_ERR("Could not setup channel 0 (%d)", err);
    }
    else
    {
        LOG_INF("adc_intan setup ok\n");
        k_sem_give(&sem_adc_rdy);
    }

    /* FPGA */
    err = spi_is_ready_dt(&fpga_dev);
    if (!err)
    {
        LOG_ERR("SPI device is not ready, err: %d", err);
    }
    else
    {
        LOG_INF("SPI device is ready");
    }

    /* Watchdog */
    if (!device_is_ready(wdt_dev))
    {
        LOG_ERR("Watchdog device %s is not ready", wdt_dev->name);
    }
    struct wdt_timeout_cfg wdt_config = {
        .window.min = 0,
        .window.max = 5000,
        .callback = wdt_callback,
        .flags = WDT_FLAG_RESET_SOC,
    };
    wdt_channel_id = wdt_install_timeout(wdt_dev, &wdt_config);
    if (wdt_channel_id < 0)
    {
        LOG_ERR("Watchdog install failed %d", wdt_channel_id);
    }
    wdt_setup(wdt_dev, WDT_OPT_PAUSE_HALTED_BY_DBG);

    LOG_DBG("msgq space: %d", k_msgq_num_free_get(&msgq_ble_tx));

    //!< Start sleep timer
    sleep_time_get();
    k_timer_start(&sleep_timer, K_MSEC(current_sleep_time), K_MSEC(current_sleep_time));
    //!< Start heartbeat timer
    sync_time_get();
    k_timer_start(&stm_heartbeat_timer, K_MSEC(current_sync_time), K_MSEC(current_sync_time));

    //!< RTC Init
    rtc_init();

    // nrf_clock_hfclk_div_set(NRF_CLOCK, NRF_CLOCK_HFCLK_DIV_2);
    LOG_INF("HFCLK Freq: %u MHz\n", nrf5340_get_cpuapp_frequency() / 1000000);
    system_limits_init();
    nvs_get_free_space();

    //!< ZBUS
    zbus_link_init();
    event_workq_init();

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        // rtc_time_print();

        wdt_feed(wdt_dev, wdt_channel_id);
        k_sleep(K_MSEC(RUN_IDLE_INTERVAL));
    }
}

void ble_write_thread(void)
{
    /* Don't go any further until BLE is initialized */
    k_sem_take(&sem_ble_init_ok, K_FOREVER);

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        /* Wait indefinitely for data to be sent over bluetooth */
        struct ble_data_t *buf = k_fifo_get(&fifo_ble_tx_data,
                                             K_FOREVER);

        if (bt_nus_send(NULL, buf->data, buf->len))
        {
            LOG_WRN("Failed to send data over BLE connection");
        }

        k_free(buf);
    }
}

K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
        NULL, PRIORITY, 0, 200);

void fpga_data_producer_thread(void)
{
    struct msgq_sample_data_t ele;

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        k_sem_take(&sem_fpga_read, K_FOREVER);
        // gpio_pin_toggle_dt(&ble_intan_on);

        if (!atomic_load(&g_imp_data_read))
        {
            // int64_t stamp = k_uptime_get_32();
            memset(fpga_data, 0, sizeof(fpga_data));
            uint8_t frame_size = (chans_select * 20 + 7) / 8 + 4;
            fpga_read(PipeOutData, fpga_data, frame_size);
            // int64_t delta = k_uptime_delta(&stamp);
            // LOG_INF("fpga read use time: %d ms", delta);
            ele.seq = sample_frame_seq;
            ele.type = FRAME_READ_BLOCK_DATA;
            ele.len = frame_size;
            ele.data[0] = 0x00;
            memcpy(ele.data + 1, fpga_data, ele.len);
            ele.len += 1;
            sample_frame_seq += 2;
            if (k_msgq_put(&msgq_ble_tx, &ele, K_NO_WAIT) != 0)
            {
                LOG_INF("msgq is full!");
            }
        }
        else
        {
            //!< Caution: IMPEDANCE_FRAME_BYTES <= sizeof(fpga_data)
#if defined(CONFIG_BT_NUS_IMP_MEA_MOCK_TEST)
            static uint8_t mock_cap = 0;
            static uint8_t mock_chan = 0;
            static uint8_t count = 0;
            fpga_data[0] = mock_chan;
            fpga_data[1] = mock_cap;
            memset(fpga_data + 2, mock_chan + count, IMPEDANCE_FRAME_RAW_BYTES);
            if (++count == 2)
            {
                LOG_DBG("Mock Read: %d %d", mock_cap, mock_chan);
                if (mock_chan < CHAN_NUM - 1) {
                    mock_chan++;
                }
                else {
                    if (mock_cap < CAP_NUM - 1) {
                        mock_cap++;
                        mock_chan = 0;
                    }
                    else {
                        k_timer_stop(&mock_test_timer);
                        k_sem_take(&sem_fpga_read, K_NO_WAIT);
                        mock_cap = 0;
                        mock_chan = 0;
                        LOG_INF("STOP MOCK");
                    }
                }
                count = 0;
            }
#else
            memset(fpga_data, 0, IMPEDANCE_FRAME_BYTES);
            fpga_read(PipeOutImpData, fpga_data, IMPEDANCE_FRAME_BYTES);
            //!< enable the LOG may cause the interrupt response to be untimely
            LOG_HEXDUMP_DBG(fpga_data, IMPEDANCE_FRAME_BYTES, "MOCK");
#endif
            uint8_t cap = fpga_data[0] >> 6;
            uint8_t chan = fpga_data[0] & 0x3F;
            if (cap < CAP_NUM && chan < CHAN_NUM) {
                if (imp_matrix[cap][chan] == 0)
                {
                    memcpy(ele.data, fpga_data, IMPEDANCE_FRAME_BYTES);
                    imp_matrix[cap][chan]++;
                }
                else
                {
                    memcpy(ele.data + IMPEDANCE_FRAME_BYTES + (imp_matrix[cap][chan] - 1) * IMPEDANCE_FRAME_RAW_BYTES,
                           fpga_data + 2,
                           IMPEDANCE_FRAME_RAW_BYTES);
                    imp_matrix[cap][chan]++;
                }

                if (imp_matrix[cap][chan] == IMPEDANCE_FRAME_COUNT)
                {
                    imp_measure_count++;
                    ele.seq = sample_frame_seq++;
                    ele.type = FRAME_GET_IMPEDANCES_VALUE;
                    ele.len = IMPEDANCE_FRAME_RAW_BYTES * IMPEDANCE_FRAME_COUNT + 2;
                    if (k_msgq_put(&msgq_ble_tx, &ele, K_NO_WAIT) != 0)
                    {
                        LOG_INF("msgq is full!");
                    }
                }

                if (imp_measure_count == CAP_NUM * CHAN_NUM)
                {
                    k_timer_start(&imp_measure_timer, K_MSEC(IMP_MEA_DELAY_STOP), K_NO_WAIT);
                }
                else
                {
                    k_timer_start(&imp_measure_timer, K_MSEC(IMP_MEA_MS), K_NO_WAIT);
                }
            }
        }
    }
}

K_THREAD_DEFINE(fpga_data_producer_thread_id, STACKSIZE, fpga_data_producer_thread, NULL, NULL,
        NULL, FPGA_DATA_PRODUCER_PRIORITY, 0, 100);

void fpga_data_consumer_thread(void)
{
    struct msgq_sample_data_t ele;

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        k_msgq_get(&msgq_ble_tx, &ele, K_FOREVER);

        uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
        uint8_t len = 0;
        do {
            Frame *BlockPipeOut = (Frame *)(data + len);
            BlockPipeOut->header = HEADER;
            BlockPipeOut->seq = ele.seq;
            BlockPipeOut->type = ele.type;
            uint8_t *content = (uint8_t *)(data + FRAME_HEADER_LEN);
            memcpy(content, ele.data, ele.len);
            BlockPipeOut->dataLen = ele.len;
            content[BlockPipeOut->dataLen] = 0x00;
            len += BlockPipeOut->dataLen + FRAME_HEADER_LEN + 1;
        } while (len < 25 && k_msgq_get(&msgq_ble_tx, &ele, K_NO_WAIT) == 0);

#if defined(CONFIG_BT_NUS_IMP_MEA_MOCK_TEST)
        // LOG_HEXDUMP_INF(data, len, "SAMPLE");
        bt_nus_send(currentConn, data, len);
#else
        LOG_HEXDUMP_DBG(data, len, "SAMPLE");
        if (atomic_load(&g_ble_connected))
        {
            int32_t ret = bt_nus_send(currentConn, data, len);
            if (ret < 0)
            {
                LOG_INF("bt_nus_send: %d", ret);
            }
        }
#endif
    }
}

K_THREAD_DEFINE(fpga_data_consumer_thread_id, STACKSIZE * 4, fpga_data_consumer_thread, NULL, NULL,
        NULL, FPGA_DATA_CONSUMER_PRIORITY, 0, 100);

void ble_rx_data_parser_thread(void)
{
    struct msgq_ble_data_t data_item;

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        k_msgq_get(&msgq_ble_rx, &data_item, K_FOREVER);

#if defined(CONFIG_BT_NUS_HOST_AUTH_ENABLED)
        if (atomic_load(&g_host_auth)) {
            receiveFrame(data_item.data, data_item.len);
        }
        else {
            AuthFrameCheck(data_item.data, data_item.len);
        }
#else
        receiveFrame(data_item.data, data_item.len);
#endif
    }
}

K_THREAD_DEFINE(ble_rx_data_parser_thread_id, STACKSIZE * 2, ble_rx_data_parser_thread, NULL, NULL,
        NULL, PRIORITY, 0, 100);