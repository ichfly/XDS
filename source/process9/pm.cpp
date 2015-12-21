#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "Bootloader.h"

#define LOGPM

P9PM::P9PM(Process9* owner) :m_open(), m_owner(owner), handlecount(0x100000001)
{

}
P9PM::~P9PM()
{

}

extern FILE* openapp(u32 titlehigh, u32 titlelow); //this is from Bootloader.cpp

void P9PM::Command(u32 data[],u32 numb)
{
    u32 resdata[0x200];
    memset(resdata, 0, sizeof(resdata));

    u16 cmd = (data[0] >> 16);
    switch (cmd)
    {
    case 1:
    {
        u64 handle = (data[2] >> 0) | ((u64)(data[1]) << 32);
        resdata[0] = 0x00020040;
        auto a = m_open.list;
        while (a)
        {
            if (a->data->handle == handle)
                break;
            a = a->next;
        }
        if (a)
        {
            LOG("pm getexheader handle=%" PRIx64 ", titleid=%" PRIx64, handle, a->data->title);
            KMemoryMap* map = m_owner->m_kernel->m_IPCFIFOAdresses[(data[3] >> 4) &0xF];
            resdata[1] = 0xE0000000;
            FILE * fd = openapp(a->data->title >> 32, (u32)a->data->title);
            if (fd)
            {
                //open the container
                char ex[0x400];
                ctr_ncchheader loader_h;
                u32 ncch_off = 0;

                // Read header.
                if (fread(&loader_h, sizeof(loader_h), 1, fd) != 1) {
                    XDSERROR("failed to read header.");
                    break;
                }
                // Load NCCH
                if (memcmp(&loader_h.magic, "NCCH", 4) != 0) {
                    XDSERROR("invalid magic.. wrong file?");
                    break;
                }

                // Read Exheader.
                if (fread(&ex, 0x400, 1, fd) != 1) { //this is fixed 
                    XDSERROR("failed to read exheader.");
                    break;
                }
                for (int i = 0; i < sizeof(ex); i++)
                {
                    if (map->Write8(data[4] + i, ex[i]) != Success)
                    {
                        break;
                    }
                }
                resdata[1] = 0;

            }
        }
        else
        {
#ifdef LOGPM
            LOG("pm getexheader handle=%" PRIx64, handle);
#endif
            resdata[1] = 0xE0000000; //todo correct error
        }
    }
	break;
    case 2:
    {
        u64 title = (data[1] >> 0) | ((u64)(data[2]) << 32);
#ifdef LOGPM
        LOG("pm register %08x %08x", data[1], data[2]);
#endif
        if ((data[3] >> 24) != 0)
            LOG("register flags %02x %02x", (data[3] >> 24), (data[3 + 4] >> 24));
        if (data[1] != data[1 + 4] && data[2] != data[2 + 4])
            LOG("register secound %08x %08x", data[1 + 4], data[2 + 4]);

        struct PMOpenprocess * neone = (PMOpenprocess*)malloc(sizeof(struct PMOpenprocess));
        neone->handle = handlecount++;
        neone->title = title;
        m_open.AddItem(neone);
        resdata[0] = 0x000200C0;
        resdata[1] = 0x0;
        resdata[2] = neone->handle >> 32;
        resdata[3] = (u32)neone->handle;
    }
    break;
    default:
            LOG("unknown PM cmd %08x", data[0]);
            break;
    }
    m_owner->Sendresponds(numb, resdata);
}

u64 P9PM::GetTitle(u64 handle)
{
    auto a = m_open.list;
    while(a)
    {
        if(a->data->handle == handle)
            break;
        a = a->next;
    }
    
    if(a)
    {
        return a->data->title;
    }
    
    return -1;
}
