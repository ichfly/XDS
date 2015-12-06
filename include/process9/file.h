
class P9File
{
public:
	P9File(Process9* owner, LowPath lowpath, LowPath highpath, u32 achivetype);
	P9File(Process9* owner, LowPath lowpath, LowPath highpath, FILE* fs, u64 offset, u64 size, u8 *hash, u32 achivetype);
	~P9File();

	virtual u64 getsize();
	virtual s32 setsize(u64 size);
	virtual s32 read(u8 *buffer, u32 size, u64 file_offset, u32 &out_sizeread);
	virtual s32 write(u8 *buffer, u32 size, u64 file_offset, u32 &out_sizewritten);
	virtual u8* GetHashPtr();

	enum {
		OPEN_READ = 1,
		OPEN_WRITE = 2,
		OPEN_CREATE = 4
	};

	static const char* FlagsToMode(u32 flags, char *buf_out)
	{
		switch (flags)
		{
			case 1: //R
				strcpy(buf_out, "rb");
				break;
			case 2: //W
			case 3: //RW
				strcpy(buf_out, "rb+");
				break;
			case 4: //C
			case 6: //W+C
				strcpy(buf_out, "wb+");
				break;
			default:
				buf_out[0] = '\0';
				break;
		}
		return buf_out;
	}

	static const char* FlagsToString(u32 flags, char* buf_out)
	{
		buf_out[0] = '\0';

		// Convert flags to string representation.
		if (flags & OPEN_READ)
			strcat(buf_out, "R");
		if (flags & OPEN_WRITE)
			strcat(buf_out, "W");
		if (flags & OPEN_CREATE)
			strcat(buf_out, "C");

		return buf_out;
	}

private:
	LowPath m_lowpath;
	LowPath m_highpath;
	char m_realpath[0x400];
	u64 m_offset;
	u64 m_size;
	Process9* m_owner;
	FILE* m_fs;
	u8 *m_hash;
	u32 m_achivetype;
};


