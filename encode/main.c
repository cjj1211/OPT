#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "chacha20.h"

static void print_hex(const char* label, const uint8_t* data, size_t len)
{
	printf("%-12s", label);
	for (size_t i = 0; i < len; i++)
	{
		printf("%02X ", data[i]);
	}
	printf("\n");
}

int main(void)
{
	uint8_t plain[] = { 0x55,0xaa,0x34,0x12,0x28,0x05,0x00,0x01,0x00,0x02,0x00,0x01,0x00 };
	uint8_t enc_frame[sizeof(plain) + 2];

	print_hex("plain:", plain, sizeof(plain));

	uint16_t counter = 0x1234;

	/********************** 加密过程 **********************/
	*(uint16_t*)enc_frame = counter;
	memcpy(enc_frame + 2, plain, sizeof(plain));			// 留两个字节给counter用
	ChaEncryptBuffer(enc_frame + 2, (uint16_t)sizeof(enc_frame) - 2, counter);
	print_hex("enc_frame:", enc_frame, sizeof(enc_frame));	// 加密后数据帧，比原文多两个字节的counter

	/********************** 解密过程 **********************/
	uint16_t counter_from_frame = *(uint16_t*)enc_frame;	// 从数据帧中取出counter
	ChaDecryptBuffer(enc_frame + 2, (uint16_t)sizeof(enc_frame) - 2, counter_from_frame);
	print_hex("decrypted:", enc_frame + 2, sizeof(enc_frame) - 2);

	if (memcmp(plain, enc_frame + 2, sizeof(plain)) == 0)
	{
		printf("PASS: decrypt(encrypt(plain)) == plain\n");
		return 0;
	}

	printf("FAIL: round-trip mismatch!\n");
	return 1;
}
