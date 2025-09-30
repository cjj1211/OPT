/*******************************************************************************
 * @file nx_crc.h
 * @brief calculate crc16/32 code function
 *
 * @copyright Copyright(C) 2025 NeuroXess, All rights reserved.
 ******************************************************************************/

#ifndef __NX_CRC_H__
#define __NX_CRC_H__

#include <stdint.h>
#include <string.h>

extern unsigned short nx_crc16 (void * crcSrc, unsigned int size);
extern uint32_t nx_crc32(volatile const uint8_t *data, size_t length);

#endif /** __NX_CRC_H__ */
