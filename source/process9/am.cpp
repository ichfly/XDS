#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "Bootloader.h"

P9AM::P9AM(Process9* owner) : m_owner(owner)
{

}
P9AM::~P9AM()
{

}


void P9AM::Command(u32 data[], u32 numb)
{
    u32 resdata[0x200];
    memset(resdata, 0, sizeof(resdata));

    u16 cmd = (data[0] >> 16);
    switch (cmd)
    {
	case 0x1:
		LOG("GetTitleCount %02x", data[1] & 0xFF);
		resdata[0] = 0x00010080;
		resdata[1] = 0;
		resdata[2] = 0; //only the old one are used (that is not correct but it works)
		break;
	case 0x3:
	{
		u8 medid = data[1] & 0xFF;
		u32 count = data[2];
		u32 desc_read = data[3];
		u32 ptr_read = data[4];
		u32 desc_write = data[5];
		u32 ptr_write = data[6];
		LOG("GetTitleInfo %02x %08x", medid, count);
		for (int j = 0; j < count; j++)
		{
			u8 temp = 0;
			for (u32 i = 0; i < 8; i++) // copy id
			{
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_read >> 4) & 0xF]->Read8(i + ptr_read, temp);
				printf("%02x", temp);
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Write8(i + ptr_write, temp);
			}
			for (u32 i = 8; i < 24; i++)
			{
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Write8(i + ptr_write, 0);
			}
			LOG("");
			ptr_write += 24;
		}

		resdata[0] = 0x00030040;
		resdata[1] = 0;
		break;
	}
	case 0x1F:
	{
		u8 count = data[1];
		u32 unk = data[2]; //(always 0?)
		u32 desc_read = data[3];
		u32 ptr_read = data[4];
		u32 desc_write = data[5];
		u32 ptr_write = data[6];
		LOG("GetTitleTemporaryInfo %08x %08x", unk, count);
		for (int j = 0; j < count; j++)
		{
			u8 temp = 0;
			for (u32 i = 0; i < 8; i++) // copy id
			{
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_read >> 4) & 0xF]->Read8(i + ptr_read, temp);
				printf("%02x", temp);
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Write8(i + ptr_write, temp);
			}
			for (u32 i = 8; i < 24; i++)
			{
				m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Write8(i + ptr_write, 0);
			}
			LOG("");
			ptr_write += 24;
		}

		resdata[0] = 0x00030040;
		resdata[1] = 0;
		break;
	}

	case 0x3F:
		LOG("unk3F %02x", data[1]&0xFF);
		resdata[0] = 0x003F0080;
		resdata[1] = 0;
		resdata[2] = 0; //u8
		break;
    default:
            LOG("unknown AM cmd %08x", data[0]);
            break;
    }
    m_owner->Sendresponds(numb, resdata);
}

