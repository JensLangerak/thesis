#include "gz_file_reader.h"
#include "runtime_assert.h"
#include "zlib-1.2.11/gzguts.h"
#include "zlib-1.2.11/zutil.h"

#include <iostream>

namespace Pumpkin
{
GzFileReader::GzFileReader(const std::string &file_location):
	file_pointer_(0),
	buffer_(0),
	current_buffer_size_(0),
	position_in_buffer_(0),
	is_state_ok_(true),
	MAX_BUFFER_SIZE(1024),
	c_(0)
{
	file_pointer_ = gzopen(file_location.c_str(), "r");
	if (!file_pointer_) { std::cout << "File " << file_location << " does not exist!\n"; exit(1); }
	buffer_ = new char[MAX_BUFFER_SIZE];
	runtime_assert(buffer_);	
}

GzFileReader& GzFileReader::operator>>(char& c)
{
	runtime_assert(is_state_ok_);

	PeekNextChar(c);
	position_in_buffer_++;

	return *this;
}

GzFileReader& GzFileReader::operator>>(std::string& s)
{
	s.clear();
	SkipWhitespaces();
	PeekNextChar(c_);
	while (!IsWhiteSpace(c_) && c_ != '\0') 
	{ 
		s += c_; 
		position_in_buffer_++;
		PeekNextChar(c_);
	}
	is_state_ok_ = (c_ != '\0');
	return *this;
}

GzFileReader& GzFileReader::operator>>(int64_t& number)
{
	number = 0;
	
	SkipWhitespaces();
	PeekNextChar(c_);
	
	int64_t sign = c_ == '-' ? -1: 1;
	if (sign == -1) 
	{ 
		position_in_buffer_++;
		PeekNextChar(c_);
	}

	do
	{
		number = number * 10 + (c_ - '0');
		position_in_buffer_++;
		PeekNextChar(c_);
	} while (c_ >= '0' && c_ <= '9' && c_ != '\0');

	number *= sign;
	is_state_ok_ = (c_ != '\0');
	return *this;
}

void GzFileReader::SkipLine()
{
	//skip the current character
	position_in_buffer_++;
	PeekNextChar(c_);
	//iterate until you find the next line character
	while (c_ != '\n' && c_ != '\0')
	{
		position_in_buffer_++;
		PeekNextChar(c_);
	}

	if (c_ == 13) //on Windows '\n' will be followed up with another character, so we need to skip that too
	{
		position_in_buffer_++;
		PeekNextChar(c_);
	}

	//now move the file pointer right after the new line character - note that there can be several new lines but here we only skip one such line
	//...but only if we did not reach the end of the file
	if (c_ != '\0') { position_in_buffer_++; }
}

bool GzFileReader::IsOk() const
{
	return is_state_ok_;
}

void GzFileReader::PeekNextChar(char &c)
{
	if (position_in_buffer_ == current_buffer_size_)
	{
		position_in_buffer_ = 0;
		current_buffer_size_ = gzread(file_pointer_, buffer_, MAX_BUFFER_SIZE);

		if (current_buffer_size_ == 0)
		{
			current_buffer_size_ = 0;
			is_state_ok_ = false;
			c = '\0';
			return;
		}
	}
	c = buffer_[position_in_buffer_];	
}

void GzFileReader::SkipWhitespaces()
{
	PeekNextChar(c_);
	while (IsWhiteSpace(c_)) 
	{ 
		position_in_buffer_++;
		PeekNextChar(c_);
	}
}

bool GzFileReader::IsWhiteSpace(char c)
{
	return (9 <= c && c <= 13) || c == 32;
}
GzFileReader::~GzFileReader() {
  delete[] buffer_;
  buffer_ = NULL;

  gz_statep state;

  /* get internal structure */
  if (file_pointer_ != NULL) {
    state = (gz_statep)file_pointer_;
    free( state->path);
    free( state->in);
    free( state->out);
    free(state->strm.state);
    free( state);
  }

}
}
