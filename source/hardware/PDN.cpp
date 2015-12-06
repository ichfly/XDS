#include "Kernel.h"
#include "Hardware.h"

#define LOGI2C

PDN::PDN(KKernel * kernel) : m_kernel(kernel), m_SPI_CNT(0)
{
}
u8 PDN::Read8(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("PDN unknown u8 read from %08x",addr);
	}
	return 0;
}
u16 PDN::Read16(u32 addr)
{
	switch (addr & 0xFFF)
	{
	case 0x1c0:
		return m_SPI_CNT;
	default:
		LOG("PDN unknown u16 read from %08x", addr);
	}
	return 0;
}
u32 PDN::Read32(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("PDN unknown u32 read from %08x", addr);
	}
	return 0;
}

void PDN::Write8(u32 addr, u8 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("PDN unknown write %02x to %08x", data, addr);
	}
}
void PDN::Write16(u32 addr, u16 data)
{
	switch (addr & 0xFFF)
	{
	case 0x1c0:
		m_SPI_CNT = data;
		return;
	default:
		LOG("PDN unknown write %04x to %08x", data, addr);
	}
}
void PDN::Write32(u32 addr, u32 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("GPIO unknown write %08x to %08x", data, addr);
	}
}