#include "Kernel.h"
#include "Hardware.h"

namespace GPU {
	template <typename T>
	void Read(T &var, const u32 addr);

	template <typename T>
	void Write(u32 addr, const T data);
} // namespace


GPUHW::GPUHW(KKernel * kernel) : m_kernel(kernel)
{
	top = new Syncer(this, false);
	bot = new Syncer(this, true);
	memset(m_data, 0, sizeof(m_data));
}
u8 GPUHW::Read8(u32 addr)
{
	u8 data;
	GPU::Read<u8>(data,addr);
	return data;
	/*switch (addr & 0x1FFFF)
	{
	default:
		LOG("GPUHW unknown u8 read from %08x",addr);
	}
	return 0;*/
}
u16 GPUHW::Read16(u32 addr)
{
	u16 data;
	GPU::Read<u16>(data, addr);
	return data;
	/*switch (addr & 0x1FFFF)
	{
	default:
		LOG("GPUHW unknown u16 read from %08x", addr);
	}
	return 0;*/
}
u32 GPUHW::Read32(u32 addr)
{
	u32 data;
	LOG("GPUHW unknown u32 read from %08x", addr);
	GPU::Read<u32>(data, addr);
	return data;
	/*switch (addr & 0x1FFFF)
	{
	default:
		LOG("GPUHW unknown u32 read from %08x", addr);
		return m_data[(addr & 0x1FFFF)/ 4];
	}
	return 0;*/
}

void GPUHW::Write8(u32 addr, u8 data)
{
	GPU::Write<u8>(addr, data);
	return;
	/*
	switch (addr & 0x1FFFF)
	{
	default:
		LOG("GPUHW unknown write %02x to %08x", data, addr);
	}*/
}
void GPUHW::Write16(u32 addr, u16 data)
{
	GPU::Write<u16>(addr, data);
	return;
	/*
	switch (addr & 0x1FFFF)
	{
	default:
		LOG("GPUHW unknown write %04x to %08x", data, addr);
	}*/
}
void GPUHW::Write32(u32 addr, u32 data)
{
	GPU::Write<u32>(addr, data);
	LOG("GPUHW unknown write %08x to %08x", data, addr);
	return;
	/*
	switch (addr & 0x1FFFF)
	{
	case 0x1C: //Memory Fill1 "PSC0" 
	{
		LOG("GPUHW unknown write %08x to %08x", data, addr);
		if (data & 0x1 == 1)
			m_kernel->FireInterrupt(0x29);
		break;
	}
	case 0x2C: //Memory Fill2 "PSC1" 
	{
		LOG("GPUHW unknown write %08x to %08x", data, addr);
		if (data&0x1 == 1)
			m_kernel->FireInterrupt(0x28);
		break;
	}
	case 0xc18: //PFF start
	{
		LOG("GPUHW unknown write %08x to %08x", data, addr);
		if (data == 1)
		m_kernel->FireInterrupt(0x2C);
		break;
	}
	case 0x18f0:
	{
		LOG("GPUHW unknown write %08x to %08x", data, addr);
		if (data == 1)
			m_kernel->FireInterrupt(0x2D);
		break;
	}
		//0x2C PFF
		//0x2B PDC1/VBlankTop
		//0x2A PDC0/VBlankTop
		//0x29 PSC0
		//0x28 PSC1
	default:
		LOG("GPUHW unknown write %08x to %08x", data, addr);
	}*/
}