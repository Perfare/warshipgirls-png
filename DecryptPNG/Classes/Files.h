#pragma once

#include <io.h>
#include <array>
#include <string>
#include <vector>
#include <direct.h>
#include <windows.h>

namespace path
{
	/**
	 * 获取当前目录
	 */
	static std::string curdir()
	{
		char exe_full_path[MAX_PATH];
		GetModuleFileNameA(NULL, exe_full_path, MAX_PATH);
		std::string current_path(exe_full_path);
		int pos = current_path.find_last_of('\\', current_path.length());
		return current_path.substr(0, pos);
	}

	/**
	 * 文件名分解
	 */
	static std::array<std::string, 2> splitext(const std::string &file_path)
	{
		auto pos = file_path.rfind('.');
		std::array<std::string, 2> text;
		if (std::string::npos != pos)
		{
			text[1] = file_path.substr(pos);
			text[0] = file_path.substr(0, pos);
		}
		else
		{
			text[0] = file_path;
		}
		return text;
	}

	/**
	 * 列出子目录下所有文件
	 */
	static std::vector<std::string> walk(const std::string &start_path)
	{
		_finddata_t file_info;
		std::vector<std::string> file_list;
		std::string find_path = start_path + "\\*";
		long handle = _findfirst(find_path.c_str(), &file_info);

		if (handle == -1L)
		{
			return file_list;
		}

		do
		{
			if (file_info.attrib & _A_SUBDIR)
			{
				if ((strcmp(file_info.name, ".") != 0) && (strcmp(file_info.name, "..") != 0))
				{
					std::string new_path = start_path + "\\" + file_info.name;
					for (auto filename : walk(new_path)) file_list.push_back(filename);
				}
			}
			else
			{
				std::string new_path = start_path + "\\";
				new_path += file_info.name;
				file_list.push_back(new_path);
			}
		} while (_findnext(handle, &file_info) == 0);

		_findclose(handle);

		return file_list;
	}
}