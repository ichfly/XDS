#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

extern FILE* openapp(u32 titlehigh, u32 titlelow); //this is from Bootloader.cpp
extern s64 FindTableOffset(FILE* fd, char* name, u64 &out_size, u8* hash_out);

Archive1234567c::Archive1234567c(Process9* owner, LowPath *lowpath) : Archive(owner, lowpath)
{
}

P9File* Archive1234567c::OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result)
{
	LOG("   path: sysmodule = %08x", *(u32*)lowpath->getraw());

	char string[0x100];
	snprintf(string, 0x100, "NAND/data/00000000000000000000000000000000/sysdata/%08x/00000000", *(u32*)lowpath->getraw());

	char mode[10];
	FILE * fd = Common::fopen_mkdir(string, P9File::FlagsToMode(flags, mode)); //TODO get proper openflags
	if (!fd)
		return NULL;

	fseek64(fd, 0, SEEK_END);
	u64 size = ftell64(fd);
	fseek64(fd, 0, SEEK_SET);

	u8 *hash = new u8[0x20];
	memset(hash, 0, 0x20); //TODO: Implement SHA256 hashing
	P9File * f = new P9File(m_owner, m_lowpath, *lowpath, fd, 0, size, hash, 0x1234567c);
	return f;
}

void Archive1234567c::DeleteFile(LowPath* lowpath, u32* result)
{
	LOG("   path: sysmodule = %08x", *(u32*)lowpath->getraw());

	char string[0x100];
	snprintf(string, 0x100, "NAND/data/00000000000000000000000000000000/sysdata/%08x/00000000", *(u32*)lowpath->getraw());

	*result = remove(string);
};