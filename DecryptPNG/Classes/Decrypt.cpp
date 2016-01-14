#include "Decrypt.h"

#include "CRC32.h"
#include "Files.h"
#include <fstream>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


// 解密PNG图片
void DecryptPNG(const std::vector<std::string> &filelist, const aes_key &key)
{
	for (auto &filename : filelist)
	{
		std::ifstream in_file(filename, std::ios::binary | std::ios::ate);
		if (!in_file.is_open())
		{
			std::cerr << "打开" << filename << " 失败！" << std::endl;
			return;
		}

		// 读取数据块位置
		uint32_t end_pos = (uint32_t)in_file.tellg();
		in_file.seekg(end_pos - sizeof(uint32_t));
		uint32_t block_start_pos = ntohl(*reinterpret_cast<uint32_t *>(&(ReadSome<sizeof(uint32_t)>(in_file)[0])));
		in_file.seekg(block_start_pos);

		// 获取数据块大小
		const uint32_t block_size = ntohl(*reinterpret_cast<uint32_t *>(&ReadSome<sizeof(uint32_t)>(in_file)[0]));
		
		// 解密数据块信息
		auto block_info = ReadLarge(in_file, uint32_t(end_pos - block_start_pos - sizeof(uint32_t) * 2));
		DecryptBlock(block_info, key);

		// 验证校验和
		block_info.seekg(block_size);
		uint32_t crc32 = ntohl(*reinterpret_cast<uint32_t *>(&ReadSome<sizeof(uint32_t)>(block_info)[0]));
		if (crc32 != CRC32(block_info.str().substr(0, block_size)))
		{
			std::cerr << "校验和验证失败！" << std::endl;
			continue;
		}

		// 创建PNG文件
		std::ofstream out_file(path::splitext(filename)[0] + ".png", std::ios::binary);
		if (!out_file.is_open())
		{
			std::cerr << "创建" << path::splitext(filename)[1] << ".png" << " 失败！" << std::endl;
			continue;
		}

		// 写入文件头
		SteamCopy(out_file, HEAD_DATA, sizeof(HEAD_DATA));

		// 读取数据块
		block_info.seekg(0);
		uint64_t read_size = 0;
		while (true)
		{
			// 验证数据有效性
			if (block_info.tellg() >= block_size)
			{
				out_file.clear();
				std::cerr << "the %s file format error!" << std::endl;
				break;
			}

			// 读取数据块信息
			Block block;
			memcpy(&block, &ReadSome<sizeof(Block)>(block_info)[0], sizeof(Block));

			// 写入数据块长度
			SteamCopy(out_file, &block.size, sizeof(block.size));

			// 大小端转换
			block.pos = ntohl(block.pos);
			block.size = ntohl(block.size);

			// 写入数据块名称
			SteamCopy(out_file, &block.name, sizeof(block.name));

			// 写入数据块内容
			std::string s_name(block.name, sizeof(block.name));
			if (strcmp(s_name.c_str(), "IHDR") == 0)
			{
				IHDRBlock ihdr;
				memcpy(&ihdr, &block, sizeof(Block));
				memcpy(((char *)&ihdr) + sizeof(Block), &ReadSome<sizeof(IHDRBlock) - sizeof(Block)>(block_info)[0], sizeof(IHDRBlock) - sizeof(Block));
				SteamCopy(out_file, ihdr.data, sizeof(ihdr.data));
			}
			else if (strcmp(s_name.c_str(), "IEND") == 0)
			{
				SteamCopy(out_file, IEND_DATA, sizeof(IEND_DATA));
				std::cout << "成功解密：" << filename << std::endl;
				break;
			}
			else
			{
				in_file.seekg(read_size);
				StreamMove(out_file, in_file, block.size + CRC_SIZE);
				read_size += block.size + CRC_SIZE;
			}
		}
	}
}