/**
 * @file bsp_gpio.c
 * @author DCX (chunxiao.dai@neuroxess.com)
 * @brief
 * @version 0.1
 * @date 2025-01-27
 *
 * @copyright Copyright (c) 2025 NeuroXess
 *
 */
#include "bsp_gpio.h"
#include "../public_api.h"
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#define LOG_MODULE_NAME OPT_GPIO
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

//!< GPIO: control pin
const struct gpio_dt_spec ble_intan_on = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), ctrl_gpios, 0);
const struct gpio_dt_spec ble_fpga_on = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), ctrl_gpios, 1);
static const struct gpio_dt_spec ble_tp39 = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), tp_gpios, 0);
static const struct gpio_dt_spec fpga_trig = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), trig_gpios, 0);
static struct gpio_callback fpga_irq_cb;
const struct gpio_dt_spec ice_reset = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), ice_gpios, 0);

K_SEM_DEFINE(sem_fpga_read, 0, 1);
static void fpga_data_rdy(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // LOG_INF("FPGA data ready at %" PRIu32 "\n", k_cycle_get_32());
    if (atomic_load(&g_sampling_started)) {
        k_sem_give(&sem_fpga_read);
    }
    // gpio_pin_toggle_dt(&ble_intan_on);
}

void configure_gpio(void)
{
    int err;

    if (!gpio_is_ready_dt(&ble_intan_on))
    {
        LOG_ERR("BLE_INTAN_ON init fail");
        return ;
    }
    err = gpio_pin_configure_dt(&ble_intan_on, GPIO_OUTPUT_INACTIVE);
    if (err < 0)
    {
        LOG_ERR("BLE_INTAN_ON configure fail");
        return ;
    }

    if (!gpio_is_ready_dt(&ble_fpga_on))
    {
        LOG_ERR("BLE_FPGA_ON init fail");
        return ;
    }
    err = gpio_pin_configure_dt(&ble_fpga_on, GPIO_OUTPUT_INACTIVE);
    if (err < 0)
    {
        LOG_ERR("BLE_FPGA_ON configure fail");
        return ;
    }

    if (!gpio_is_ready_dt(&ice_reset))
    {
        LOG_ERR("ICE_Reset init fail");
        return ;
    }
    err = gpio_pin_configure_dt(&ice_reset, GPIO_OUTPUT_INACTIVE);
    if (err < 0)
    {
        LOG_ERR("ICE_Reset configure fail");
        return ;
    }

    if (!gpio_is_ready_dt(&ble_tp39))
    {
        LOG_ERR("BLE_TP39 init fail");
        return ;
    }
    err = gpio_pin_configure_dt(&ble_tp39, GPIO_OUTPUT_INACTIVE);
    if (err < 0)
    {
        LOG_ERR("BLE_TP39 configure fail");
        return ;
    }

    if (!gpio_is_ready_dt(&fpga_trig)) {
        LOG_ERR("Error: trig gpio %s is not ready",
               fpga_trig.port->name);
        return ;
    }
    err = gpio_pin_configure_dt(&fpga_trig, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n",
               err, fpga_trig.port->name, fpga_trig.pin);
        return ;
    }
    err = gpio_pin_interrupt_configure_dt(&fpga_trig,
                          GPIO_INT_EDGE_RISING);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
            err, fpga_trig.port->name, fpga_trig.pin);
        return ;
    }

    gpio_init_callback(&fpga_irq_cb, fpga_data_rdy, BIT(fpga_trig.pin));
    gpio_add_callback(fpga_trig.port, &fpga_irq_cb);
    LOG_INF("Set up trig at %s pin %d", fpga_trig.port->name, fpga_trig.pin);
}

//!< Power on intan and fpga
void fpga_pwr_enable(void)
{
    gpio_pin_set_dt(&ble_fpga_on, 1);
}
void intan_pwr_enable(void)
{
    gpio_pin_set_dt(&ble_intan_on, 1);
}

//!< Power off intan and fpga
void fpga_pwr_disable(void)
{
    gpio_pin_set_dt(&ble_fpga_on, 0);
}
void intan_pwr_disable(void)
{
    gpio_pin_set_dt(&ble_intan_on, 0);
}

void fpga_reset(void)
{
    gpio_pin_set_dt(&ice_reset, 1);
    k_sleep(K_MSEC(10));
    gpio_pin_set_dt(&ice_reset, 0);
}