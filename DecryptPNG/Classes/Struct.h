#pragma once

#include <cstdint>

static const uint32_t HEAD_SIZE = 8;

static const uint32_t CRC_SIZE = 4;

static const unsigned char HEAD_DATA[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

static const unsigned char IEND_DATA[] = { 0xae, 0x42, 0x60, 0x82 };

#pragma pack(push, 1)

struct Block
{
	char name[4];
	uint32_t pos;
	uint32_t size;
};

struct IHDRBlock
{
	Block block;
	char data[13 + CRC_SIZE];
};

#pragma pack(pop)