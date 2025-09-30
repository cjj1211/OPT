/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
*  作者 : 戴春晓 chunxiao.dai@neuroxess.com
*  描述 : TI TMP119 driver
*  修改记录:
*
******************************************************************************/

#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "bsp_tmp119.h"

LOG_MODULE_REGISTER(tmp119);

static int tmp119_reg_read(const struct i2c_dt_spec *dev, uint8_t reg, uint16_t *val)
{
    if (i2c_burst_read_dt(dev, reg, (uint8_t *)val, 2) < 0) {
        return -EIO;
    }

    *val = sys_be16_to_cpu(*val);

    return 0;
}

__unused static int tmp119_reg_write(const struct i2c_dt_spec *dev, uint8_t reg, uint16_t val)
{
    uint8_t tx_buf[3] = {reg, val >> 8, val & 0xFF};

    return i2c_write_dt(dev, tx_buf, sizeof(tx_buf));
}

/**
 * @brief Check the Device ID
 *
 * @param[in]   dev  Pointer to the device structure
 * @param[in]   id   Pointer to the variable for storing the device id
 *
 * @retval 0 on success
 * @retval -EIO Otherwise
 */
static inline int tmp119_device_id_check(const struct i2c_dt_spec *dev, uint16_t *id)
{
    if (tmp119_reg_read(dev, TMP119_REG_DEVICE_ID, id) != 0) {
        LOG_ERR("%s: Failed to get Device ID register!",
            dev->bus->name);
        return -EIO;
    }

    if (*id != TMP119_DEVICE_ID) {
        LOG_ERR("%s: Failed to match the device IDs!",
            dev->bus->name);
        return -EINVAL;
    }

    return 0;
}

int tmp119_init(const struct i2c_dt_spec *dev)
{
    int rc;
    uint16_t id;

    if (!device_is_ready(dev->bus))
    {
        LOG_ERR("Device %s is not ready\n", dev->bus->name);
        return -EINVAL;
    }
    LOG_INF("Device is %p, name is %s\n", dev->bus, dev->bus->name);

    /* Check the Device ID */
    rc = tmp119_device_id_check(dev, &id);
    if (rc < 0) {
        return rc;
    }

    LOG_INF("Got device ID: %x", id);

    return 0;
}

int tmp119_sample_fetch(const struct i2c_dt_spec *dev, struct tmp119_data *drv_data)
{
    uint16_t value;
    uint16_t cfg_reg = 0;
    int rc;

    /* clear sensor values */
    drv_data->sample = 0U;

    /* Make sure that a data is available */
    rc = tmp119_reg_read(dev, TMP119_REG_CFGR, &cfg_reg);
    if (rc < 0) {
        LOG_ERR("%s, Failed to read from CFGR register",
            dev->bus->name);
        return rc;
    }

    if ((cfg_reg & TMP119_CFGR_DATA_READY) == 0) {
        LOG_DBG("%s: no data ready", dev->bus->name);
        return -EBUSY;
    }

    /* Get the most recent temperature measurement */
    rc = tmp119_reg_read(dev, TMP119_REG_TEMP, &value);
    if (rc < 0) {
        LOG_ERR("%s: Failed to read from TEMP register!",
            dev->bus->name);
        return rc;
    }

    /* store measurements to the driver */
    drv_data->sample = (int16_t)value;

    return 0;
}

int tmp119_channel_get(const struct device *dev, struct sensor_value *val, struct tmp119_data *drv_data)
{
    int32_t tmp;

    /*
     * See datasheet "Temperature Results and Limits" section for more
     * details on processing sample data.
     */
    tmp = ((int16_t)drv_data->sample * (int32_t)TMP119_RESOLUTION) / 10;
    val->val1 = tmp / 1000000; /* uCelsius */
    val->val2 = tmp % 1000000;

    return 0;
}