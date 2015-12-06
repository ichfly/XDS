#include "Kernel.h"
#include "Hardware.h"

#define LOGI2C

SPI::SPI(KKernel * kernel) : m_kernel(kernel), SPI_NEW_CNT(0)
{
}
u8 SPI::Read8(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("SPI unknown u8 read from %08x",addr);
	}
	return 0;
}
u16 SPI::Read16(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("SPI unknown u16 read from %08x", addr);
	}
	return 0;
}
u32 SPI::Read32(u32 addr)
{
	switch (addr & 0xFFF)
	{
	case 0x800:
		return SPI_NEW_CNT;
	default:
		LOG("SPI unknown u32 read from %08x", addr);
	}
	return 0;
}

void SPI::Write8(u32 addr, u8 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("SPI unknown write %02x to %08x", data, addr);
	}
}
void SPI::Write16(u32 addr, u16 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("SPI unknown write %04x to %08x", data, addr);
	}
}
void SPI::Write32(u32 addr, u32 data)
{
	switch (addr & 0xFFF)
	{
	case 0x800:
		SPI_NEW_CNT = data &~0x8000; //always ready
	default:
		LOG("SPI unknown write %08x to %08x", data, addr);
	}
}