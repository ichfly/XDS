#include "Kernel.h"

//tools
#define Read32(p) (p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24)

#define dyncoresizestart (1024 * 1024 * 20)

bool KProcess::Synchronization(KThread* thread, u32 &error)
{
    return true; //stall till the Process ends? TODO check that
}
KProcess::~KProcess()
{
	free(repretBuffer);
}
KProcess::KProcess(KCodeSet* codeset, u32 capabilities_num, u32* capabilities_ptr, KKernel* Kernel,bool is_firm_process)
	: m_memory(this), m_limit(new KResourceLimit()), LINEAR_memory_virtual_address_userland(0x14000000) /*this is not like on the 3DS but it works with old and new NCCH*/
{
    m_ProcessID = Kernel->GetNextProcessID();
    m_Kernel = Kernel;
    m_Kernel->AddProcess(this, is_firm_process);
    memset(m_AllowedInterrupt, 0, sizeof(m_AllowedInterrupt));
    memset(m_systemcallmask,   0, sizeof(m_systemcallmask));

    m_exheader_flags = 0;
    m_codeset.SetObject(codeset);

    ParseArm11KernelCaps(capabilities_num, capabilities_ptr);

    KResourceLimit* limit = (KResourceLimit*) *m_limit;
    // TODO: this is wrong but it should be overwritten after the boot anyway.
    limit->SetMaxValue(0, 0x04);
    limit->SetMaxValue(1, 0x01680000);
    limit->SetMaxValue(2, 0xC5);
    limit->SetMaxValue(3, 0xF5);
    limit->SetMaxValue(4, 0x23);
    limit->SetMaxValue(5, 0x3F);
    limit->SetMaxValue(6, 0x2B);
    limit->SetMaxValue(7, 0x1D);
    limit->SetMaxValue(8, 0x2A);
    limit->SetMaxValue(9, 0x3E8);

    //map shared mem
    m_memory.AddPages(0x1FF80000,
        chunk_Configuration->size, chunk_Configuration->data, chunk_Configuration,
        PERMISSION_R, MEMTYPE_SHAREDMEMORY, NULL);
    m_memory.AddPages(0x1FF81000,
        chunk_Shared->size, chunk_Shared->data, chunk_Shared,
        (m_exheader_flags & 0x8) ? PERMISSION_RW : PERMISSION_R, MEMTYPE_SHAREDMEMORY, NULL);

    codeset->MapInto(&m_memory, m_exheader_flags & (1 << 12));

	repretBuffer = (char*)malloc(dyncoresizestart); //must be increasable later
	repretBuffersize = dyncoresizestart;
	repretBuffertop = 0;
	CreamCache = new bb_map;

}

void KProcess::ParseArm11KernelCaps(u32 capabilities_num, u32* capabilities_ptr)
{
    u32 handletable_size= 0x200;

    for (u32 i = 0; i < capabilities_num; i++)
    {
        u32 desc = Read32(((u8*)(&capabilities_ptr[i])));

        // Allowed interrupts.
        if ((desc & 0xF0000000) == 0xE0000000) {
            for (int j = 0; j < 4; j++) {
                int irq = (desc >> (j * 7)) & 0x7F;

                if (irq != 0x7F)
                    m_AllowedInterrupt[irq] = true;
            }
        }
        // Allowed syscalls.
        else if ((desc & (0x1f << 27)) == (0x1e << 27)) {
            int init = ((desc >> 24) & 7)*0x18;

            for (int i = init; i < init + 0x18 && i < 0x80; i++) {
                int offset = i - init;
                m_systemcallmask[i] = (desc >> offset) & 0x1;
            }
        }
        // Handle-table size.
        else if ((desc & (0xff << 24)) == (0xfe << 24)) {
            handletable_size= desc & 0x3FF;
        }
        // Flags
        else if ((desc & (0x1ff << 23)) == (0x1fe << 23)) {
            m_exheader_flags = desc;
        }
        // Mapping I/O.
        else if ((desc & (0xfff << 20)) == (0xffe << 20)) {
            m_memory.AddIOMem((desc & 0xFFFFF) << 12, 0x1000, PERMISSION_RW);
        }
        // Mapping STATIC.
        else if ((desc & (0x7ff << 21)) == (0x7fc << 21)) {
            u32 startaddr = (desc & 0xFFFFF) << 12;

            if(i++ > capabilities_num) {
                XDSERROR("Error while parsing descriptor for STATIC memory.\n");
                continue;
            }

            u32 desc2 = Read32(((u8*)(&capabilities_ptr[i])));

            if ((desc2 & 0xFFE00000) != (desc & 0xFFE00000)) {
                XDSERROR("Error while parsing descriptor for STATIC memory (%08x %08x).\n",
                    desc, desc2);
                continue;
            }

            u32 endaddr = (desc2 & 0xFFFFF) << 12;
            m_memory.AddIOMem((desc & 0xFFFFF) << 12, endaddr - startaddr,
                (desc & (1 << 20)) ? PERMISSION_R : PERMISSION_RW);
        }
        // do nothing
        else if (desc == 0xFFFFFFFF){
        }
        else {
            LOG("Unknown descriptor %x\n", desc);
        }
    }

    m_handles = new KHandleTable(this, handletable_size);
}

void KProcess::AddQuickCode(u8* buf, size_t size) {
    m_memory.AddCodeSegment(0x100000, size, buf, PERMISSION_RWX); // TEMP
}

Result KProcess::WaitSynchronization(s64 timeout) {
    return -1; // TODO
}

void KProcess::Destroy() {
    // Empty.
}

KMemoryMap* KProcess::getMemoryMap() {
    return &m_memory;
}

void KProcess::AddThread(KThread * thread)
{
    //TODO start the thread
    u32 out_3DSAddr;
    u8* out_TLSpointer;
    m_memory.AddTLS(&out_3DSAddr, &out_TLSpointer);
    thread->m_TSL3DS = out_3DSAddr;
    thread->m_TSLpointer = out_TLSpointer;
    m_Threads.AddItem(thread);
    m_Kernel->StartThread(thread);
}

const char* KProcess::GetName()
{
    return ((KCodeSet*)*m_codeset)->GetName();
}

u32 KProcess::GetProcessID()
{
    return m_ProcessID;
}

void KProcess::SetResourceLimit(KResourceLimit* lim)
{
    m_limit.SetObject(lim);
}

KResourceLimit* KProcess::GetResourceLimit()
{
    return (KResourceLimit*)*m_limit;
}

KHandleTable* KProcess::GetHandleTable()
{
    return m_handles;
}

bool KProcess::IsInstanceOf(ClassName name) {
    if (name == KProcess::name)
        return true;

    return super::IsInstanceOf(name);
}
