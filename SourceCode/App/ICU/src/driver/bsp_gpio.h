#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

extern struct k_sem sem_fpga_read;
extern const struct gpio_dt_spec ble_fpga_on;
extern const struct gpio_dt_spec ble_intan_on;

void configure_gpio(void);
void fpga_pwr_enable(void);
void intan_pwr_enable(void);
void fpga_pwr_disable(void);
void intan_pwr_disable(void);
void fpga_reset(void);

#endif /* __BSP_GPIO_H__ */