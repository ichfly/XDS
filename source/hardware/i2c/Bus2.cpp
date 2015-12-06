#include "Kernel.h"
#include "Hardware.h"

HWBUS2::HWBUS2(KKernel * kernel) : IOI2C(kernel)
{
	m_active = false;
}

bool HWBUS2::Read(u8 &data, u8 device, bool end, bool &noack)
{
	data = 0x1;
	switch (device)
	{
	case 0x4B: //MCU 8 Bit addresses (read only)
	{
		switch (m_register)
		{
			case 0xF: // ShellState 1 Byte? open here
				data = 0xFF;
				break;
		}
		break;
	}
	default:
		XDSERROR("unknown device %02x", device);
		break;
	}

	m_active = true;
	if (end)
		m_active = false;
	return true;
}
bool HWBUS2::Write(u8& data, u8 device, bool end, bool &noack)
{

	switch (device)
	{
	case 0x4A: //MCU 8 Bit addresses (write only)
	{
		if (!m_active)
			m_register = data;
		{

		}
		break;
	}
	default:
		XDSERROR("unknown device %02x", device);
		break;
	}
	m_active = true;
	if (end)
		m_active = false;
	return true;
}