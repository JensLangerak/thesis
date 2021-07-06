#pragma once

#include <zlib.h>
//#include "zlib-1.2.11/zlib.h"

#include <string>

namespace Pumpkin
{
class GzFileReader
{
public:
	GzFileReader(const std::string &file_location);
        ~GzFileReader();
	void PeekNextChar(char& c);
	GzFileReader& operator>>(char& c);
	GzFileReader& operator>>(std::string& s);
	GzFileReader& operator>>(int64_t& number);
	void SkipWhitespaces();
	void SkipLine();

	bool IsOk() const;	
	
private:
	
	
	bool IsWhiteSpace(char c);

	gzFile file_pointer_;
	char* buffer_;
	int current_buffer_size_;
	int position_in_buffer_;
	int MAX_BUFFER_SIZE;
	char c_;
	bool is_state_ok_;
};
}