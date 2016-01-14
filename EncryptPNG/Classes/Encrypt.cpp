#include "Encrypt.h"

#include "CRC32.h"
#include "Files.h"
#include <fstream>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

/**
 * 写入单个文件数据
 * @param filename 文件名称
 * @param outstream 输出文件流
 * @return 数据块信息
 */
std::stringstream WriteFileData(const std::string &filename, std::ofstream &outstream)
{
	std::ifstream file;
	std::stringstream block_info;
	file.open(filename, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "打开" << filename << " 失败！" << std::endl;
		return block_info;
	}

	// 读取文件头
	auto head = ReadSome<HEAD_SIZE>(file);

	// 读取数据块
	while (true)
	{
		Block block;

		// 获取数据块长度
		auto lenght = ReadSome<4>(file);
		if (file.eof()) break;
		auto block_size = ntohl(*reinterpret_cast<uint32_t *>(&lenght[0]));

		// 获取数据块名称
		auto block_name = &(ReadSome<4>(file)[0]);

		// 获取数据块内容
		auto block_data = ReadLarge(file, block_size + CRC_SIZE);

		// 数据块信息
		block.size = htonl(block_size);
		block.pos = htonl((uint32_t)outstream.tellp());
		memcpy(block.name, &block_name[0], sizeof(block.name));

		// 根据数据类型进行处理
		std::string s_name(block.name, sizeof(block.name));
		if (strcmp(s_name.c_str(), "IHDR") == 0)
		{
			IHDRBlock ihdr;
			ihdr.block = block;
			memcpy(ihdr.data, block_data.str().c_str(), sizeof(ihdr.data));
			SteamCopy(block_info, &ihdr, sizeof(IHDRBlock));
		}
		else if (strcmp(s_name.c_str(), "IEND") == 0)
		{
			SteamCopy(block_info, &block, sizeof(Block));
		}
		else
		{
			SteamCopy(block_info, &block, sizeof(Block));
			StreamMove(outstream, block_data, block_size + CRC_SIZE);
		}
	}
	return block_info;
}

// 加密PNG图片
void EncryptPNG(const std::vector<std::string> &filelist, const aes_key &key)
{
	for (auto &filename : filelist)
	{
		// 写入文件数据
		std::string out_path = path::splitext(filename)[0] + ".muka";
		std::ofstream out_file(out_path, std::ios::binary);
		if (!out_file.is_open())
		{
			std::cerr << "创建" << filename << " 失败！" << std::endl;
			continue;
		}

		// 写入文件数据
		std::stringstream block_info = WriteFileData(filename, out_file);
		uint32_t block_size = uint32_t(block_info.tellp() - block_info.tellg());
		if (block_size == 0) continue;

		// 记录起始位置
		uint32_t block_start_pos = htonl((uint32_t)out_file.tellp());

		// 写入数据块信息大小
		block_size = htonl(block_size);
		SteamCopy(out_file, &block_size, sizeof(block_size));

		// 写入校验和
		uint32_t crc32 = htonl(CRC32(block_info.str()).GetChecksum());
		SteamCopy(block_info, &crc32, sizeof(crc32));

		// 数据块信息加密
		EncryptBlock(block_info, key);

		// 写入数据块信息
		StreamMove(out_file, block_info, uint32_t(block_info.tellp() - block_info.tellg()));

		// 写入数据块信息位置
		char *user_data = reinterpret_cast<char *>(&block_start_pos);
		for (unsigned int i = 0; i < sizeof(block_start_pos); ++i) out_file.put(user_data[i]);

		std::cout << "已生成：" << out_path.c_str() << std::endl;
	}
}