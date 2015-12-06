// This file shouldn't exist. It has STL.
#include <string>


namespace Common {
    std::string StringFromFormat(const char* format, ...);
    // Cheap!
    bool CharArrayFromFormatV(char* out, int outsize, const char* format, va_list args);
    unsigned int CountLeadingZeros(unsigned int num);
	FILE* fopen_mkdir(const char* name, const char* mode);
	char * U16ToASCII(char* data);
}

