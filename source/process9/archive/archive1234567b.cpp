#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

Archive1234567b::Archive1234567b(Process9* owner, LowPath *lowpath) : Archive(owner, lowpath)
{
	char string[0x200];
	char string2[0x200];

	memcpy(string2, lowpath->getraw(), lowpath->GetSize());
	snprintf(string, 0x200, "NAND/data/00000000000000000000000000000000/extdata/%08x/%08x", *(u32*)(m_lowpath.getraw() + 8), *(u32*)(m_lowpath.getraw() + 4));

	if (0 != access(string, 0x00)) {
		if (ENOENT == errno) {
			throw 0xc8804464;
		}
		if (ENOTDIR == errno) {
			throw 0xc8804464;
		}
	}
}
P9File* Archive1234567b::OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result)
{

	char string[0x200];
	char string2[0x200];

	memcpy(string2, lowpath->getraw(), lowpath->GetSize());
	snprintf(string, 0x200, "NAND/data/00000000000000000000000000000000/extdata/%08x/%08x%s", *(u32*)(m_lowpath.getraw() + 8), *(u32*)(m_lowpath.getraw() + 4), string2);

	LOG("   path: %s", string2);
	char mode[10];
	FILE * fd = Common::fopen_mkdir(string, P9File::FlagsToMode(flags, mode)); //TODO get proper openflags
	if (!fd)
		return NULL;

	fseek64(fd, 0, SEEK_END);
	u64 size = ftell64(fd);
	fseek64(fd, 0, SEEK_SET);

	u8 *hash = new u8[0x20];
	memset(hash, 0, 0x20); //TODO: Implement SHA256 hashing
	P9File * f = new P9File(m_owner, m_lowpath, *lowpath, fd, 0, size, hash, 0x1234567b);
	return f;
}