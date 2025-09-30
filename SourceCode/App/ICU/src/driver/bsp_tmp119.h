/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。                   
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : TI TMP119 driver
*  修改记录: 
*  
******************************************************************************/

#ifndef __BSP_TMP119_H__
#define __BSP_TMP119_H__

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>

#define TMP119_REG_TEMP         0x0   //!< R: Temperature result register
#define TMP119_REG_CFGR         0x1   //!< R/W: Configuratiron register
#define TMP119_REG_HIGH_LIM     0x2   //!< R/W: Temperature high limit register
#define TMP119_REG_LOW_LIM      0x3   //!< R/W: Temperature low limit register
#define TMP119_REG_EEPROM_UL    0x4   //!< R/W: EEPROM unlock register
#define TMP119_REG_EEPROM1      0x5   //!< R/W: EEPROM1 register
#define TMP119_REG_EEPROM2      0x6   //!< R/W: EEPROM2 register
#define TMP119_REG_TEMP_OFFSET  0x7   //!< R/W: Temperature offset register
#define TMP119_REG_EEPROM3      0x8   //!< R/W: EEPROM3 register
#define TMP119_REG_DEVICE_ID    0xF   //!< R: Device ID register

#define TMP119_RESOLUTION       78125    /* in tens of uCelsius*/
#define TMP119_RESOLUTION_DIV   10000000

#define TMP119_DEVICE_ID        0x2117

#define TMP119_CFGR_DATA_READY  BIT(13)

struct tmp119_data {
    uint16_t sample;
    uint16_t id;
};

int tmp119_sample_fetch(const struct i2c_dt_spec *dev, struct tmp119_data *drv_data);
int tmp119_init(const struct i2c_dt_spec *dev);
int tmp119_channel_get(const struct device *dev, struct sensor_value *val, struct tmp119_data *drv_data);

#endif /*  __BSP_TMP119_H__ */
