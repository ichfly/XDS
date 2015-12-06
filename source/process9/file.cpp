#include "Common.h"

#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

P9File::P9File(Process9* owner, LowPath lowpath, LowPath highpath, u32 achivetype) : m_owner(owner), m_lowpath(lowpath), m_highpath(highpath), m_achivetype(achivetype)
{

}
P9File::P9File(Process9* owner, LowPath lowpath, LowPath highpath, FILE* fs, u64 offset, u64 size, u8* hash, u32 achivetype) : m_owner(owner), m_lowpath(lowpath), m_highpath(highpath), m_fs(fs), m_offset(offset), m_size(size), m_hash(hash), m_achivetype(achivetype)
{

}
P9File::~P9File() {
	delete m_hash;
	if (m_fs)
		fclose(m_fs);
}

u64 P9File::getsize()
{
	return m_size;
}

u8* P9File::GetHashPtr()
{
	return m_hash;
}

s32 P9File::read(u8 *buffer,u32 size,u64 file_offset,u32 &out_sizeread)
{
	fseek64(m_fs, file_offset + m_offset, SEEK_SET);
	out_sizeread = fread(buffer, 1, size, m_fs);
	if ((m_achivetype == 0x1234567c || m_achivetype == 0x1234567b) && file_offset == 0 && size >= 0x100) //MAC AESEnginePatch
	{
		memset(buffer, 0x11, 0x100);
	}
	return 0;
}

s32 P9File::write(u8 *buffer, u32 size, u64 file_offset, u32 &out_sizewritten)
{
	fseek64(m_fs, file_offset + m_offset, SEEK_SET);
	out_sizewritten = fwrite(buffer, 1, size, m_fs);
	fflush(m_fs);
	return 0;
}
s32 P9File::setsize(u64 size)
{
	m_size = size;
	if (ftruncate(fileno(m_fs), size) == -1) {
		LOG("ftruncate failed.\n");
		return -1;
	}
	return 0;
}
