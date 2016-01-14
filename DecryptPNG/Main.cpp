#include <iostream>
#include "Files.h"
#include "Decrypt.h"


int main()
{
	aes_key key = { 0 };
	std::cout << "请输入16位密钥：" << std::endl;
	std::cin.get((char *)&key[0], key.size());

	// 寻找所有muka文件
	std::vector<std::string> files;
	auto all_files = path::walk(path::curdir());
	for (auto filename : all_files)
	{
		if (path::splitext(filename)[1] == ".muka")
		{
			files.push_back(filename);
		}
	}
	DecryptPNG(files, key);
	
	return 0;
}