#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "Bootloader.h"

#define LOGMC

P9MC::P9MC(Process9* owner) : m_owner(owner)
{

}
P9MC::~P9MC()
{

}


void P9MC::Command(u32 data[], u32 numb)
{
    u32 resdata[0x200];
    memset(resdata, 0, sizeof(resdata));

    u16 cmd = (data[0] >> 16);
    switch (cmd)
    {
	case 0xA: //unkA
		LOG("unk MC stub %08x %08x %08x"
			, data[1], data[2], data[3]);
		resdata[0] = 0x000A0040;
		resdata[1] = 0;
		break;
    default:
            LOG("unknown MC cmd %08x", data[0]);
            break;
    }
    m_owner->Sendresponds(numb, resdata);
}

