#include "Common.h"
#include "Util.h"

#include <stdarg.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <codecvt>
#else
#include <iconv.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

namespace Common {
	char * U16ToASCII(char* data)
	{
		char* data2 = data;
		while (*data2 != 0)
		{
			data2+= 2;
			data++;
			*data = *data2;
		}
		return data;
	}
    bool CharArrayFromFormatV(char* out, int outsize, const char* format, va_list args)
    {
        int writtenCount;

#ifdef _MSC_VER
        // You would think *printf are simple, right? Iterate on each character,
        // if it's a format specifier handle it properly, etc.
        //
        // Nooooo. Not according to the C standard.
        //
        // According to the C99 standard (7.19.6.1 "The fprintf function")
        //     The format shall be a multibyte character sequence
        //
        // Because some character encodings might have '%' signs in the middle of
        // a multibyte sequence (SJIS for example only specifies that the first
        // byte of a 2 byte sequence is "high", the second byte can be anything),
        // printf functions have to decode the multibyte sequences and try their
        // best to not screw up.
        //
        // Unfortunately, on Windows, the locale for most languages is not UTF-8
        // as we would need. Notably, for zh_TW, Windows chooses EUC-CN as the
        // locale, and completely fails when trying to decode UTF-8 as EUC-CN.
        //
        // On the other hand, the fix is simple: because we use UTF-8, no such
        // multibyte handling is required as we can simply assume that no '%' char
        // will be present in the middle of a multibyte sequence.
        //
        // This is why we lookup an ANSI (cp1252) locale here and use _vsnprintf_l.
        static _locale_t c_locale = nullptr;
        if (!c_locale)
            c_locale = _create_locale(LC_ALL, ".1252");
        writtenCount = _vsnprintf_l(out, outsize, format, c_locale, args);
#else
        writtenCount = vsnprintf(out, outsize, format, args);
#endif

        if (writtenCount > 0 && writtenCount < outsize)
        {
            out[writtenCount] = '\0';
            return true;
        }
        else
        {
            out[outsize - 1] = '\0';
            return false;
        }
    }

    std::string StringFromFormat(const char* format, ...)
    {
        va_list args;
        char *buf = nullptr;
#ifdef _WIN32
        int required = 0;

        va_start(args, format);
        required = _vscprintf(format, args);
        buf = new char[required + 1];
        CharArrayFromFormatV(buf, required + 1, format, args);
        va_end(args);

        std::string temp = buf;
        delete[] buf;
#else
        va_start(args, format);
        if (vasprintf(&buf, format, args) < 0)
            LOG("Unable to allocate memory for string");
        va_end(args);

        std::string temp = buf;
        free(buf);
#endif
        return temp;
    }

    unsigned int CountLeadingZeros(unsigned int num)
    {
        unsigned int x = 0x80000000;
        unsigned int zero_count = 0;

        for (int i = 0; i < 32; i++)
        { 
            if ((x & num) == 0)
            {
                zero_count++;
                x >>= 1;
                continue;
            }
            break;
        }

        return zero_count;
    }

	/* Opens the file, creating directories in its pathspec if necessary */
	FILE* fopen_mkdir(const char* name, const char* mode) {
		char* mname = strdup(name);
		int i;
		for (i = 0; mname[i] != '\0'; i++)
		{
			if (i>0 && (mname[i] == '\\' || mname[i] == '/')) {
				char slash = mname[i];
				mname[i] = '\0';

#ifdef _WIN32
				int ret = _mkdir(mname);
#else
				int ret = mkdir(p, 0777);
#endif
				if (ret >0 && ret != EEXIST)
				{
					break;
				}
				mname[i] = slash;
			}
		}
		free(mname);
		FILE* temp = fopen(name, mode);
		/*if (temp == NULL) //if file does not exist, create it
		{
			temp = fopen(name, "wb+"); //always use "wb+" to ensure it can read and write to the file
		}*/

		return temp;
	}
}
