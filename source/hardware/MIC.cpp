#include "Kernel.h"
#include "Hardware.h"


MIC::MIC(KKernel * kernel) : m_kernel(kernel)
{
}
u8 MIC::Read8(u32 addr)
{
	LOG("MIC unknown u8 read from %08x",addr);
	return 0;
}
u16 MIC::Read16(u32 addr)
{
	if ((addr & 0xFFF) == 0)
	{
		return m_CNT;
	}
	LOG("MIC unknown u16 read from %08x", addr);
	return 0;
}

u32 MIC::Read32(u32 addr)
{
	if ((addr & 0xFFF) == 4)
	{
		return 0; //mic data
	}
	LOG("MIC unknown u32 read from %08x", addr);
	return 0;
}
void MIC::Write8(u32 addr, u8 data)
{
	LOG("MIC unknown write %02x to %08x", data, addr);
}
void MIC::Write16(u32 addr, u16 data)
{
	if ((addr & 0xFFF) == 0)
	{
		m_CNT = data;
		return;
	}
	LOG("MIC unknown write %04x to %08x", data, addr);
}
void MIC::Write32(u32 addr, u32 data)
{
	LOG("MIC unknown write %08x to %08x", data, addr);
}