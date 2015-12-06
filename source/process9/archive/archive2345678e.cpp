#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

extern FILE* openapp(u32 titlehigh, u32 titlelow); //this is from Bootloader.cpp
extern s64 FindTableOffset(FILE* fd, char* name, u64 &out_size, u8* hash_out);
extern s64 FindRomFSOffset(FILE* fd, char* name, u64 &out_size, u8* hash_out);

#define strcpy_s(a,b,c) strncpy(a,c,b)

Archive2345678e::Archive2345678e(Process9* owner, LowPath *lowpath) : Archive(owner, lowpath)
{
	m_title = owner->GetTitleFromPM(lowpath->GetHandle());
}
P9File* Archive2345678e::OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result)
{
	char path[9];
	strcpy_s(path,8, (char*)lowpath->getraw() + 4);
	LOG("   path: type = %08x, str = %s", *(u32*)lowpath->getraw(), path);
	FILE * fd = openapp(m_title >> 32, (u32)m_title);
	u64 size;
	u8 *hash = new u8[0x20];
	s64 offset;
	switch (*(u32*)lowpath->getraw())
	{
	case 0:
		offset = FindRomFSOffset(fd, path, size, hash);
		break;
	case 1:
		offset = FindTableOffset(fd, path, size, hash);
		break;
	default:
		LOG("error unknown src");
		for (u32 i = 0; i < lowpath->GetSize(); i++)
			printf("%02x", lowpath->getraw()[i]);
		LOG("");
		return NULL;
	}
	P9File * f = new P9File(m_owner, m_lowpath, *lowpath, fd, offset, size, hash, 0x2345678e);
	return f;
}
