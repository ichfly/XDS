#include "Kernel.h"

#define LOGCOMMUNICATION

//tools


KSession::KSession(KPort * owner) : m_Server(this), m_Client(this)
{
    m_owner = owner;
}
KSession::~KSession()
{

}
bool KSession::IsInstanceOf(ClassName name) {
    if (name == KSession::name)
        return true;

    return super::IsInstanceOf(name);
}
s32 KSession::Communicate(KThread* sender, KThread* recver, bool IsResponse)
{
    u32* senddata = (u32*)(sender->m_TSLpointer + 0x80);
    u32* recvdata = (u32*)(recver->m_TSLpointer + 0x80);
#ifdef LOGCOMMUNICATION
    if (IsResponse)
    {
        if (m_owner)
        {
            LOG("responding %s <- %s port %s", recver->m_owner->GetName(), sender->m_owner->GetName(), m_owner->m_Name);
        }
        else
        {
            LOG("responding %s <- %s", recver->m_owner->GetName(), sender->m_owner->GetName());
        }
    }
    else
    {
        if (m_owner)
        {
            LOG("sending %s -> %s port %s", sender->m_owner->GetName(), recver->m_owner->GetName(), m_owner->m_Name);
        }
        else
        {
            LOG("sending %s -> %s", sender->m_owner->GetName(), recver->m_owner->GetName());
        }
    }
#endif
    u32 cmd = *senddata++;
    *recvdata++ = cmd;
    u32 translated = cmd & 0x3F;
    u32 nomal = (cmd >> 6) & 0x3F;
#ifdef LOGCOMMUNICATION
    LOG("cmd %08x", cmd);
#endif
    for (u32 i = 0; i < nomal; i++)
    {
        u32 data = *senddata++;
#ifdef LOGCOMMUNICATION
        LOG("data %08x", data);
#endif
        *recvdata++ = data;
    }
    for (u32 i = 0; i < translated; )
    {
        u32 descriptor = *senddata++;
        if (descriptor & 0x1)
        {
            LOG("descriptor with sec flag set %08x please send to XDS dev Team", descriptor);
        }
        switch (descriptor & 0xE)
        {
        case 0:
        {
            if (descriptor == 0x00000020)
            {
                u32 PID = sender->m_owner->GetProcessID();
#ifdef LOGCOMMUNICATION
                LOG("PID %08x", PID);
#endif
                i+= 2;
                *recvdata++ = descriptor;
                *recvdata++ = PID;
                senddata++;
            }
            else if ((descriptor&0x03FFFFFF) == 0x00000000)
            {
                *recvdata++ = descriptor;
                i++;
                int size = (descriptor >> 26) + 1;
				for (int j = 0; j < size && i < translated; j++)
                {
                    u32 data = *senddata++;
                    KAutoObjectRef obj;
                    u32 newhand = 0;
					s32 ret;
					if (data == 0xFFFF8000)
					{
						obj.SetObject(sender);
						ret = Success;
					}
					else if (data == 0xFFFF8001)
					{
						obj.SetObject(sender->m_owner);
						ret = Success;
					}
					else
					{
						ret = sender->m_owner->GetHandleTable()->GetHandleObject(obj, data);
					}
					if (ret == Success)
					{
						recver->m_owner->GetHandleTable()->CreateHandle(newhand, *obj);
					}

#ifdef LOGCOMMUNICATION
                    LOG("handle %08x -> %08x", data, newhand);
#endif
                    *recvdata++ = newhand;
                    i++;
                }
                
            }
            else if ((descriptor & 0x03FFFFFF) == 0x00000010)
            {
                *recvdata++ = descriptor;
                i++;
                int size = (descriptor >> 26) + 1;
				for (int j = 0; j < size && i < translated; j++)
                {
                    u32 data = *senddata++;
                    KAutoObjectRef obj;
                    u32 newhand = 0;
                    s32 ret = sender->m_owner->GetHandleTable()->GetHandleObject(obj, data);
                    if (ret == Success)
                    {
                        recver->m_owner->GetHandleTable()->CreateHandle(newhand, *obj);
                        sender->m_owner->GetHandleTable()->CloseHandle(data);
                    }
#ifdef LOGCOMMUNICATION
                    LOG("handle close %08x -> %08x", data, newhand);
#endif
                    *recvdata++ = newhand;
                    i++;
                }

            }
            else {
                recvdata++;
                *senddata = descriptor;
                senddata++;
                i++;
                LOG("unknown descriptor %08x", descriptor);
            }
        }
        break;
        case 0x2:
        {
            *recvdata = descriptor;
            recvdata++;

            u32 srcaddr = *senddata;

            u32* pointerlist = (u32*)(recver->m_TSLpointer + 0x180);

            u32 id = (descriptor >> 10) & 0xF;
            u32 sizewanted = (descriptor >> 14);
            
            pointerlist += id * 2;
            u32 tarsize = (*pointerlist++ >> 14);
            u32 targed = *pointerlist;


#ifdef LOGCOMMUNICATION
            LOG("TLS cpy %08x (id %01x size %06x) (%08x %08x)", srcaddr, (descriptor >> 10) & 0xF, (descriptor >> 14), targed, tarsize);
#endif
            if (targed != 0 && tarsize >= sizewanted)
            {
                for (u32 i = 0; i < sizewanted; i++)
                {
                    u8 data = 0;
                    s32 ret = sender->m_owner->getMemoryMap()->Read8(srcaddr + i, data);
                    if (ret != Success)
                    {
                        LOG("IPC Communicate error reading from %08x", srcaddr);
                        break;
                    }
#ifdef LOGCOMMUNICATION
					printf("%02x",data);
#endif
                    recver->m_owner->getMemoryMap()->Write8(targed + i, data);
                    if (ret != Success)
                    {
                        LOG("IPC Communicate error writing from %08x", targed);
                        break;
                    }
                }
                *recvdata = targed;
#ifdef LOGCOMMUNICATION
			LOG("");
#endif
			}
            else
            {
                *recvdata = 0;
            }
            recvdata++;
            senddata++;
            i += 2;
        }
        break;
        case 0x4:
        {
            recver->m_owner->m_Kernel->m_IPCFIFOAdresses[(descriptor >> 4) & 0xF] = sender->m_owner->getMemoryMap();
            recver->m_owner->m_Kernel->m_IPCFIFOAdressesRO[(descriptor >> 4) & 0xF] = false;

            *recvdata = descriptor;
            recvdata++;

            u32 data = *senddata++;
#ifdef LOGCOMMUNICATION
            LOG("IPC translate RW %08x (id %01x size %06x)", data, (descriptor >> 4) & 0xF, (descriptor >> 8));
#endif
            *recvdata++ = data;
            i += 2;
        }
        break;
        case 0x6:
        {
            recver->m_owner->m_Kernel->m_IPCFIFOAdresses[(descriptor>>4)&0xF] = sender->m_owner->getMemoryMap();
            recver->m_owner->m_Kernel->m_IPCFIFOAdressesRO[(descriptor >> 4) & 0xF] = true;

            *recvdata = descriptor;
            recvdata++;

            u32 data = *senddata++;
#ifdef LOGCOMMUNICATION
            LOG("IPC translate RO %08x (id %01x size %06x)", data, (descriptor >> 4) & 0xF, (descriptor >> 8));
#endif
            *recvdata++ = data;
            i += 2;
            break;
        }
        case 0xA:
        case 0xC:
        case 0xE:
        {
            MemoryPermissions perm;
            switch (descriptor & 0xE)
            {
            case 0xA:
                perm = PERMISSION_R;
                break;
            case 0xC:
                perm = PERMISSION_RW; //this is how it it else the fs fails
                break;
            case 0xE:
                perm = PERMISSION_RW;
                break;
            }
            u32 size = (descriptor >> 4);
            *recvdata = descriptor;
            recvdata++;
            u32 data = *senddata++;
			size = (((size + data + 0xFFF) / 0x1000) * 0x1000) - data;
            int j = 0x04000000;
            for (; j < 0x08000000; j += 0x1000)
            {
                if (sender->m_owner->getMemoryMap()->IPCMap(j, data, size, perm, recver->m_owner->getMemoryMap()) == Success)
                    break;
            }
            if (j >= 0x08000000)
                LOG("OUT OF IPC MEM implement cleanup plz");

#ifdef LOGCOMMUNICATION
            switch (descriptor & 0xE)
            {
            case 0xA:
                LOG("IPC map RO %08x (size %08x) to %08x", data, size, j);
                break;
            case 0xC:
                LOG("IPC map WO %08x (size %08x) to %08x", data, size, j);
                break;
            case 0xE:
                LOG("IPC map RW %08x (size %08x) to %08x", data, size, j);
                break;
            }
#endif

            *recvdata++ = j + (data &0xFFF);
            i += 2;
            break;
        }
        default:
            LOG("unknown descriptor %08x", descriptor);
            recvdata++;
            *senddata = descriptor;
            i++;
            break;
        }
    }
    return Success;
}