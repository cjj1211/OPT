#ifndef G3_FRAME_DECODER_H
#define G3_FRAME_DECODER_H
#include "G3_Protocol.h"
#include <stdio.h>
uint8_t decodeBuffer[WIRELESS_BUF_SIZE] = { 0x00 };
int decodeBufferLen = 0;

int getFrameLen(const Frame* frame)
{
    return FRAME_HEADER_LEN + frame->dataLen + DATA_CRC_LEN;
}

void decodeFrame(char* inData, int len, void(*processFunc)(Frame*))
{
    // 解码缓存为空，则输入数据必须要包含帧头，从帧头开始填入数据
    if(decodeBufferLen == 0) 
    {
        Frame* frame = NULL;
        int index = 0;
        for (; index < len-1; ++index)
        {
            if(*(uint16_t*)(inData + index) == HEADER)
            {
                frame = (Frame*)(inData + index);
                break;
            }
        }

        // 找到帧头
        if(frame!= NULL)
        {
            if (len - index < getFrameLen(frame)) // 但数据长度不够，将数据拷贝到缓存
            {
                for (int i = 0; i + index < len; ++i)
                {
                    ++decodeBufferLen;
                    decodeBuffer[decodeBufferLen] = inData[index + i];
                }
            }
            else // 数据中至少有1帧
            {
                processFunc(frame);
                if (len > getFrameLen(frame) + index)
                {
                    decodeFrame(inData + index + getFrameLen(frame), len - (getFrameLen(frame) + index), processFunc);
                }
            }
        }
        else// 无效数据
        {
            return;
        }
    }
    else
    {
        // 新数据 + 缓存中数据 长度 小于 帧头 --》 直接将数据填入到缓存更新尾指针位置后返回
        if(len + decodeBufferLen < FRAME_HEADER_LEN)
        {
            for (int i = 0; i  < len; ++i)
            {
                ++decodeBufferLen;
                decodeBuffer[decodeBufferLen] = inData[i];
            }
        }
        else // 缓存中数据长度 大于等于 帧头 
        {
            // 先将buffer中帧头填满
            const int oldBufferLen = decodeBufferLen;
            int index = 0; // 帧头结尾对应的位置
            if (decodeBufferLen < FRAME_HEADER_LEN)
            {
                for (; index < FRAME_HEADER_LEN - oldBufferLen; ++index)
                {
                    ++decodeBufferLen;
                    decodeBuffer[decodeBufferLen] = inData[index];
                }
            }
            
            Frame* frame = (Frame*)decodeBuffer;
            if(len < frame->dataLen + index) // 所有数据加起来不足一帧
            {
                for(int i = 0; i < len- index; ++i)
                {
                    ++decodeBufferLen;
                    decodeBuffer[decodeBufferLen] = inData[i];
                }
            }
            else // 所有数据加起来刚好一帧或大于一帧
            {
                for (; decodeBufferLen < getFrameLen(frame); ++index)
                {
                    ++decodeBufferLen;
                    decodeBuffer[decodeBufferLen] = inData[index];
                }
                processFunc(frame);
                decodeBufferLen = 0;

                if(index < len)
                {
                    decodeFrame(inData + index, len - index, processFunc); 
                }
            }
        }
    }
}

#endif
