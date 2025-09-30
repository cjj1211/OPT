/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
*  作者 : 宋廷宇 tingyu.song@neuroxess.com
*  描述 : 暴露main.c中的资源给其他模块使用
*  修改记录: 
*  
******************************************************************************/

#ifndef __EXTERN_RESOURCES_H__
#define __EXTERN_RESOURCES_H__

#include "cstdint"

extern uint16_t  m_conn_handle;

ble_nus_t* getBLE_NUS(void);

#endif // !__EXTERN_RESOURCES_H__

