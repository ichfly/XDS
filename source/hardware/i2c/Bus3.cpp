#include "Kernel.h"
#include "Hardware.h"

HWBUS3::HWBUS3(KKernel * kernel) : IOI2C(kernel)
{
	m_active = false;
}

bool HWBUS3::Read(u8 &data, u8 device, bool end, bool &noack)
{
	XDSERROR("unknown bus %02x", device);
	data = 0;
	return true;
}
bool HWBUS3::Write(u8 &data, u8 device, bool end, bool &noack)
{
	XDSERROR("unknown bus %02x", device);
	return true;
}