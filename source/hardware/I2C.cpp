#include "Kernel.h"
#include "Hardware.h"

#define LOGI2C

IOI2C::IOI2C(KKernel * kernel) : m_kernel(kernel)
{
	m_CNT = 0xFF;
#ifdef LOGI2C
	m_index = 0;
#endif
}
u8 IOI2C::Read8(u32 addr)
{
	//LOG("I2C u8 read from %08x", addr);
	switch (addr & 0xFFF)
	{
	case 0x0:
		return m_data;
	case 0x1:
		return 0x10;
	default:
		LOG("I2C u8 read from %08x", addr);
	}
	return 0x0;
}
u16 IOI2C::Read16(u32 addr)
{
	LOG("I2C u16 read from %08x", addr);
	return 0;
}
u32 IOI2C::Read32(u32 addr)
{
	LOG("I2C u32 read from %08x", addr);
	return 0;
}

void IOI2C::Write8(u32 addr, u8 data)
{
	//LOG("I2C u8 write %08x (%02x)", addr, data);
	switch (addr & 0xFFF)
	{
	case 0x0:
#ifdef LOGI2C
		if (sizeof(m_buffer) > m_index)
		{
			m_buffer[m_index] = data;
		}
#endif
		{
			bool m_ack = true;
			if (!(m_CNT & 0x20))
				Write(data, m_deviceID, m_CNT & 0x1, m_ack);
		}
		m_data = data;
		break;
	case 0x1:
		if (data & 0x2) //Start
		{
			m_deviceID = m_data;
		}
		if (data & 0x80)
		{
			bool m_ack = true;
			if (data & 0x20)
				Read(m_data, m_deviceID, data & 0x1, m_ack);
		}
#ifdef LOGI2C
		m_index++;
		if (data & 0x1) //Stop
		{
			if (data & 0x20)
			{
				LOG("I2C read (%02x) (%08x) end", m_deviceID, m_index); //this is of by one when the direction swap
			}
			if (!(data & 0x20))
			{
				LOG("I2C write (%02x) (%08x) end", m_deviceID, m_index); //this is of by one when the direction swap
				for (int i = 0; i < m_index; i++)
					printf("%02x ", m_buffer[i]);
				LOG("");
			}
		}
		if (data & 0x2) //Start
		{
			m_index = 0;
		}
		if ((m_CNT & 0x20) != (data & 0x20))
		{
			if (m_CNT & 0x20)
			{
				LOG("I2C read (%02x) (%08x)", m_deviceID, m_index); //this is of by one when the direction swap
			}
			else
			{
				LOG("I2C write (%02x) (%08x)", m_deviceID, m_index); //this is of by one when the direction swap
				for (int i = 0; i < m_index; i++)
					printf("%02x ", m_buffer[i]);
				LOG("");
			}
			m_index = 0;
		}
#endif
		m_CNT = data;
		break;
	default:
		LOG("I2C u8 write %08x (%02x)", addr, data);
		break;
	}
}
void IOI2C::Write16(u32 addr, u16 data)
{
	LOG("I2C u16 write %08x (%04x)", addr, data);
}
void IOI2C::Write32(u32 addr, u32 data)
{
	LOG("I2C u32 write %08x (%08x)", addr, data);
}