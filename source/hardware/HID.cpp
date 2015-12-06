#include "Kernel.h"
#include "Hardware.h"


extern "C" int citraPressedkey;
HID::HID(KKernel * kernel) : m_kernel(kernel), m_IO(0), m_DIR(0)
{
}
u8 HID::Read8(u32 addr)
{
	LOG("HID unknown u8 read from %08x",addr);
	return 0;
}
u16 HID::Read16(u32 addr)
{
	if ((addr & 0xFFF) == 0)
	{
		LOG("hid read key %04x", citraPressedkey);
		return citraPressedkey;
	}
	LOG("HID unknown u16 read from %08x", addr);
	return 0;
}

u32 HID::Read32(u32 addr)
{
	if ((addr & 0xFFF) == 0)
	{
		LOG("hid read key %08x", citraPressedkey);
		return citraPressedkey;
	}

	LOG("HID unknown u32 read from %08x", addr);
	return 0;
}
void HID::Write8(u32 addr, u8 data)
{
	LOG("HID unknown write %02x to %08x", data, addr);
}
void HID::Write16(u32 addr, u16 data)
{
	LOG("HID unknown write %04x to %08x", data, addr);
}
void HID::Write32(u32 addr, u32 data)
{
	LOG("HID unknown write %08x to %08x", data, addr);
}