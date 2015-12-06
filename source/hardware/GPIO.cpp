#include "Kernel.h"
#include "Hardware.h"

GPIO::GPIO(KKernel * kernel) : m_kernel(kernel), m_IO(0), m_DIR(0)
{
}
u8 GPIO::Read8(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("GPIO unknown u8 read from %08x",addr);
	}
	return 0;
}
u16 GPIO::Read16(u32 addr)
{
	switch (addr & 0xFFF)
	{
	case 0x14:
		//return ~(m_DIR >> 16);
		return 0xFFFF;
	default:
		LOG("GPIO unknown u16 read from %08x", addr);
	}
	return 0;
}
u32 GPIO::Read32(u32 addr)
{
	switch (addr & 0xFFF)
	{
	case 0x24: //direction? not sure but looks like
		return m_DIR;
	default:
		LOG("GPIO unknown u32 read from %08x", addr);
	}
	return 0;
}

void GPIO::Write8(u32 addr, u8 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("GPIO unknown write %02x to %08x", data, addr);
	}
}
void GPIO::Write16(u32 addr, u16 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("GPIO unknown write %04x to %08x", data, addr);
	}
}
void GPIO::Write32(u32 addr, u32 data)
{
	switch (addr & 0xFFF)
	{
	case 0x24:
		m_DIR = data;
		break;
	default:
		LOG("GPIO unknown write %08x to %08x", data, addr);
	}
}