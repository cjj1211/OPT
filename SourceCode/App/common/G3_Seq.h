
/******************************************************************************
 *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *   描述 : 协议帧序号的处理模块
 *  修改记录:
 *
 ******************************************************************************/
#ifndef G3_SEQ_H
#define G3_SEQ_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Get the 全局统一的发送方帧序号
 *
 * @return uint16_t
 */

uint16_t getSendSeq(uint16_t *sendSeq);

/**
 * @brief 检查接收帧的序号是否正确，不正确表明发生了丢帧
 *
 * @param oldSeq 上一个帧序号
 * @param seq
 * @return true
 * @return false
 */
bool checkRecvSeqIsValid(uint16_t *oldSeq, uint16_t seq);

/**
 * @brief 重置发送帧序号
 *
 */
void resetSendSeq(uint16_t *sendSeq);

/**
 * @brief 重置接收帧序号
 *
 */
void resetRecvSeq(uint16_t *recvSeq);

#endif // G3_SEQ_H
