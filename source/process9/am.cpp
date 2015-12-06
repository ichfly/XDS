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
		resdata[2] = 0;
		break;
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

