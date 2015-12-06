#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "Bootloader.h"

#define LOGPM

P9PS::P9PS(Process9* owner) : m_owner(owner)
{

}
P9PS::~P9PS()
{

}


void P9PS::Command(u32 data[], u32 numb)
{
    u32 resdata[0x200];
    memset(resdata, 0, sizeof(resdata));

    u16 cmd = (data[0] >> 16);
    switch (cmd)
    {
	case 0x3: //VerifyRsaSha256 
		LOG("VerifyRsaSha256 stub %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x"
			, data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10]);
		resdata[0] = 0x00030040;
		resdata[1] = 0;
		break;
    default:
            LOG("unknown PS cmd %08x", data[0]);
            break;
    }
    m_owner->Sendresponds(numb, resdata);
}

