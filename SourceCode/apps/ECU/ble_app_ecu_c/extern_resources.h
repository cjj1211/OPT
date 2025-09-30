/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
*  作者 : 宋廷宇 tingyu.song@neuroxess.com
*  描述 : 暴露main.c中的资源给其他模块使用
*  修改记录: 
*  
******************************************************************************/

#ifndef __EXTERN_RESOURCES_H__
#define __EXTERN_RESOURCES_H__

#include "ble_nus_c.h"
#include "app_usbd_cdc_acm.h"

extern bool m_ble_connected;

/**
 * @brief 获取蓝牙发送服务
 * 
 * @return ble_nus_c_t* 
 */
ble_nus_c_t* getBLE_NUS_C(void);

/**
 * @brief 获取USB CDC ACM发送服务
 * 
 * @return app_usbd_cdc_acm_t* 
 */
const app_usbd_cdc_acm_t * getUSBCdcAcm(void);

#endif // __EXTERN_RESOURCES_H__

