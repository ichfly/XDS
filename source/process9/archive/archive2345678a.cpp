#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"
#include "Bootloader.h"
extern FILE* openapp(u32 titlehigh, u32 titlelow); //this is from Bootloader.cpp
extern s64 FindTableOffset(FILE* fd, char* name, u64 &out_size, u8* hash_out);

//utill
static u32 Read32(uint8_t p[4])
{
	u32 temp = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
	return temp;
}

Archive2345678a::Archive2345678a(Process9* owner, LowPath *lowpath) : Archive(owner, lowpath)
{
}

P9File* Archive2345678a::OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result)
{
	FILE* fd = openapp(*(u32*)(m_lowpath.getraw() + 4), (*(u32*)m_lowpath.getraw() & 0xFFFFFF));
	if (!fd)
		return NULL;


	fseek(fd, 0, SEEK_SET);
	if (fd == NULL)
	{
		return NULL;
	}


	//open the container
	ctr_ncchheader loader_h;
	u32 ncch_off = 0;

	// Read header.
	if (fread(&loader_h, sizeof(loader_h), 1, fd) != 1) {
		XDSERROR("failed to read header.");
		return NULL;
	}
	// Load NCCH
	if (memcmp(&loader_h.magic, "NCCH", 4) != 0) {
		XDSERROR("invalid magic.. wrong file?");
		return NULL;
	}

	u32 fs_off = Read32(loader_h.romfsoffset) * 0x200;
	u32 fs_sz = Read32(loader_h.romfssize) * 0x200;

	fseek64(fd, 0, SEEK_END);
	u64 size = ftell64(fd);
	fseek64(fd, 0, SEEK_SET);

	u8 *hash = new u8[0x20];
	memset(hash, 0, 0x20); //TODO: Implement SHA256 hashing
	P9File * f = new P9File(m_owner, m_lowpath, *lowpath, fd, fs_off, fs_sz, hash, 0x2345678a);
	return f;
}