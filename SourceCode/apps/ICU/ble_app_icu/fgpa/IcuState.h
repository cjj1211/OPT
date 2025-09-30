/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  描述 : 管理ICU的状态
*  修改记录: 
*  
******************************************************************************/

#ifndef __ICU_STATE_H__
#define __ICU_STATE_H__

#include "HardwareDefine.h"
#include <cstdint>

/**
 * @brief 从FPGA读取状态，并获取本身的状态
 * 
 */
void initICUState(void);

/**
 * @brief 返回当前ICU的状态
 * 
 */
IcuStatus getIcuStatus(void);

void read_time_to_empty(uint8_t *data);

void twi_init (void);


#endif // __ICU_STATE_H__

