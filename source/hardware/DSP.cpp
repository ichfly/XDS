#include "Kernel.h"
#include "Hardware.h"

DSP::DSP(KKernel * kernel) : m_kernel(kernel), m_DSP_PDATA(0), m_DSP_PADR(0), m_DSP_PCFG(0x100) /*Write FIFO Empty (Read FIFO Empty Flag)*/, m_DSP_PSTS(0), m_DSP_PSEM(0), m_DSP_PMASK(0), m_DSP_PCLEAR(0), m_DSP_SEM(0)
{
	memset(m_DSP_CMD, 0, sizeof(m_DSP_CMD));
	memset(m_DSP_REP, 0, sizeof(m_DSP_REP));
}
u8 DSP::Read8(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("DSP unknown u8 read from %08x",addr);
	}
	return 0;
}
u16 DSP::Read16(u32 addr)
{
	u16 temp;
	LOG("DSP u16 read from %08x", addr);
	switch (addr & 0xFFF)
	{
	case 0:
		return m_DSP_PDATA;

	case 4:
		return m_DSP_PADR;
	case 8:
		return m_DSP_PCFG;
	case 0xC:
		return m_DSP_PSTS;
	case 0x10:
		return m_DSP_PSEM;
	case 0x14:
		return m_DSP_PMASK;
	/*case 0x18:
		return m_DSP_PCLEAR;*/
	case 0x1C:
		return m_DSP_SEM;
	case 0x24:
		m_DSP_PSTS &= ~(1 << 10);
		temp = m_DSP_REP[0];
		REPread(0);
		return temp;
	case 0x2C:
		m_DSP_PSTS &= ~(1 << 11);
		temp = m_DSP_REP[1];
		REPread(1);
		return temp;
	case 0x34:
		m_DSP_PSTS &= ~(1 << 12);
		temp = m_DSP_REP[2];
		REPread(2);
		return temp;
	default:
		LOG("DSP unknown u16 read from %08x", addr);
	}
	return 0;
}
u32 DSP::Read32(u32 addr)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("DSP unknown u32 read from %08x", addr);
	}
	return 0;
}

void DSP::Write8(u32 addr, u8 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("DSP unknown write %02x to %08x", data, addr);
	}
}
void DSP::Write16(u32 addr, u16 data)
{
	LOG("DSP write %04x to %08x", data, addr);
	switch (addr & 0xFFF)
	{
	case 0:
		m_DSP_PDATA = data;
		break;
	case 4:
		m_DSP_PADR = data;
		break;
	case 8:
		if (m_DSP_PCFG & 0x1 && !(data & 0x1))
			Reset();
		m_DSP_PCFG = data;
		break;
	case 0xC:
		m_DSP_PSTS = data;
		break;
	case 0x10:
		m_DSP_PSEM = data;
		break;
	case 0x14:
		m_DSP_PMASK = data;
		break;
	case 0x18:
		m_DSP_SEM &= ~data;
		break;
	/*case 0x1C:
		m_DSP_SEM = data;
		break;*/
	case 0x20:
		m_DSP_CMD[0] = data;
		m_DSP_PSTS &= ~(1 << 13);
		break;
	case 0x28:
		m_DSP_CMD[1] = data; 
		m_DSP_PSTS &= ~(1 << 14);
		break;
	case 0x30:
		m_DSP_CMD[2] = data;
		m_DSP_PSTS &= ~(1 << 15);
		break;
	default:
		LOG("DSP unknown write %04x to %08x", data, addr);
	}
}
void DSP::Write32(u32 addr, u32 data)
{
	switch (addr & 0xFFF)
	{
	default:
		LOG("DSP unknown write %08x to %08x", data, addr);
	}
}

u16 DSP::DSPreadCMD(u8 numb)
{
	m_DSP_PSTS |= (1 << (12 + numb));
	return m_DSP_CMD[numb];
}
void DSP::DSPwriteRES(u16 data, u8 numb)
{
	m_DSP_PSTS |= (1 << (10 + numb));
	m_DSP_REP[numb] = data;
}

//HLE stuff

void DSP::Reset()
{
	//this is the reset that must happen inorder for the DSP module to return
	for (int i = 0; i < 3; i++)
	{
		DSPreadCMD(i);
		DSPwriteRES(0x1,i); //the 1 can be any other val the vals are ignoerd
	}
	m_phase = 0;
	//m_kernel->FireInterrupt(0x4a);
	//this causes
	//DSP u16 read from 1ed0301c
	//DSP u16 read from 1ed0300c
}
void DSP::REPread(u8 id)
{ 
	if (id == 2)
	{
		if (m_phase == 0) //just for handshake
		{
			DSPreadCMD(0);
			DSPwriteRES(0x1, 0); //must be 1
			DSPreadCMD(1);
			DSPwriteRES(0x1, 1); //must be 1
			DSPreadCMD(2);
			DSPwriteRES(0x1, 1); //must be 1
			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 0");
		}
		else if (m_phase == 1)
		{
			DSPreadCMD(2);
			DSPwriteRES(0x1, 2);

			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 1");
		}
		else if (m_phase == 2)
		{
			DSPreadCMD(2);
			DSPwriteRES(0x1, 2); 
			

			//it also updates the current PSTS state
			//must be between 0 and 8
			//Bit (3-1) select the funtion
			//Bit (0) sets a flag r2

			//function 0 only with flag not set else it dose nothing
			
			//function 1-3 are the same 
			//flag not set else nothing happen
			//signales a event
			m_DSP_PSTS |= 0x140; //why do I have to send data when I recv data

			DSPwriteRES(4, 0);//this should contain something for the data recv

			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 4");
		}
		else if (m_phase == 3) //this is part of the sending
		{
			DSPreadCMD(2);
			DSPwriteRES(4, 0);//this should contain something for the data recv
			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 5");
		}
		else if (m_phase == 4)
		{
			DSPreadCMD(2);
			DSPwriteRES(0x1, 4);

			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 2");
		}
		else if (m_phase == 5)
		{
			DSPreadCMD(2);
			DSPwriteRES(0x1, 6);

			m_kernel->FireInterrupt(0x4a);
			LOG("DSP 3");
		}
	m_phase++;
	}
}
void DSP::CMDwrite(u8 id)
{
	DSPreadCMD(id); //this is needed
}