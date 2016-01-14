#pragma once

#include <string>
#include <vector>
#include <stdint.h>

class CRC32
{
public:
	CRC32();

	CRC32(const std::string &item);

	CRC32(const std::vector<char> &item);

	virtual ~CRC32();

	void Clear();

	void Add(unsigned char item);

	void Add(const std::string &item);

	void Add(const std::vector<char> &item);

public:
	operator unsigned long() const
	{
		return GetChecksum();
	}

	uint32_t GetChecksum() const;

protected:
	uint32_t Table(unsigned int idx) const;

private:
	uint32_t checksum_;
};