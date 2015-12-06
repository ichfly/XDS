
#include "../Common.h"

static u64 LowPathRead64(uint8_t p[8])
{
	u64 temp = p[4] | p[5] << 8 | p[6] << 16 | p[7] << 24 | (u64)(p[0]) << 32 | (u64)(p[1]) << 40 | (u64)(p[2]) << 48 | (u64)(p[3]) << 56;
	return temp;
}

class LowPath {
public:
    LowPath(u32 type, u32 size, u32 desc, u8 *ptr);
	LowPath(LowPath &pat);
    ~LowPath();

    enum {
        PATH_INVALID,
        PATH_EMPTY,
        PATH_BINARY,
        PATH_CHAR,
        PATH_WCHAR
    };

    const char* TypeToString()
    {
        // Convert type to string representation.
        switch(m_type) {
            case PATH_INVALID:
                return "INVALID";
            case PATH_EMPTY:
                return "EMPTY";
            case PATH_BINARY:
                return "BINARY";
            case PATH_CHAR:
                return "CHAR";
            case PATH_WCHAR:
                return "WCHAR";
            default:
                return "UNKNOWN";
        }
    }

    const std::string GetPath()
    {
        static const char* hex_digits = "0123456789abcdef";
        u32 i;

        std::string str;

        str.resize(512); //TODO: fix this

        switch(m_type) {
            case PATH_BINARY:
                // Dump binary paths in hex.
                for(i = 0; i < m_size; i++) {
                    u8 b = m_ptr[i];
                    str[2 * i] = hex_digits[(b >> 4) & 0xF];
                    str[2 * i + 1] = hex_digits[b & 0xF];
                }
                return str;

            case PATH_CHAR:
                return std::string((char*)m_ptr);

            case PATH_WCHAR:
                for(i = 0; i < m_size; i++) {
                    // TODO: Do this conversion properly
                    // Convert unicode -> ascii.
                    // If it fails, use question marks.
                    if(m_ptr[(2 * i) + 1])
                        str[i] = '?';
                    else
                        str[i] = m_ptr[2 * i] & 0xFF;
                }
                return str;

            default:
                return "";
        }
    }

    const u32 GetSize()
    {
        return m_size;
    }

    const u64 GetHandle()
    {
		return LowPathRead64(m_ptr);
    }
	const u8* getraw()
	{
		return m_ptr;
	}
private:
    u32 m_type;
    u32 m_size;
    u32 m_desc;
    u8 *m_ptr;
};
