#include "Kernel.h"
//todo correct ResourceLimmit attatch and use ResourceLimmit, timeout, the port stuff is not 100% correct

#define SWILOG
//#define SHOWCLEAREVENT

struct CodeSetInfo
{
	u8 	Name[8];
	u16 unk1;
	u16 unk2;
	u32 unk3;
	u32 TEXTAddr;
	u32 TEXTSize;
	u32 ROAddr;
	u32 ROSize;
	u32 	RWAddr;
	u32 	RWSize;
	u32 	PagesTEXT;
	u32 	PagesRO;
	u32 	PagesRW;
	u32 	unk4;
	u8 	TitleID[8];
};

struct DmaSubConfig {
    s8 peripheral_id; // @0 If not *_ALT_CFG set, this must be < 0x1E.
    u8 type; // @1 Accepted values: 4=fixed_addr??, 8=increment_addr??, 12=lgy_fb_copy?, 15=userspace_copy?
    s16 unk3; // @2 Must be 0 or multiple of 4?
    s16 transfer_size; // @4 Must not be 0 if peripheral_id == 0xFF.
    s16 unk4; // @6
    s16 transfer_stride; // @8
};

struct DmaConfig {
    s8 channel_sel; // @0 Selects which DMA channel to use: 0-7, -1 = don't care.
    u8 endian_swap_size; // @1 Accepted values: 0=none, 2=16bit, 4=32bit, 8=64bit.
    u8 flags; // @2 bit0: SRC_CFG, bit1: DST_CFG, bit2: SHALL_BLOCK, bit3: ???, bit6: SRC_ALT_CFG, bit7: DST_ALT_CFG
    u8 padding;
    DmaSubConfig src_cfg;
    DmaSubConfig dst_cfg;
};

static u64 Read64(uint8_t p[8])
{
	u64 temp = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24 | (u64)(p[4]) << 32 | (u64)(p[5]) << 40 | (u64)(p[6]) << 48 | (u64)(p[7]) << 56;
	return temp;
}

#define PAGE_SIZE                                                        0x1000

void ProcessSwi(u8 swi, u32 Reg[15], KThread * currentThread)
{
#ifdef SWILOG
    //LOG("Process %s thread %d syscall %02X", currentThread->m_owner->GetName(), currentThread->m_thread_id, swi);
#endif
    if (!currentThread->m_owner->m_systemcallmask[swi])
    {
        XDSERROR("Process %s thread %u tryed to call syscall %02X but is not allowed to do that", currentThread->m_owner->GetName(), currentThread->m_thread_id, swi);
        return; //TODO the 3DS would terminate the Process
    }
    switch (swi)
    {
    case 1://ControlMemory(u32* address, u32 addr0, u32 addr1, u32 size, <onstack> u32 operation, u32 permissions)
    {
        u32 op = Reg[0];
        u32 addr0 = Reg[1];
        u32 addr1 = Reg[2];
        u32 size = Reg[3];
        u32 perm = Reg[4];
        u32 address = 0;
        s32 ret = ControlMemory_swi(&address, addr0, addr1, size, op, perm, currentThread);
        Reg[1] = address;
        Reg[0] = ret;

#ifdef SWILOG
        LOG("Process %s thread %u ControlMemory (%08x %08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, op, addr0, addr1, size, perm, Reg[0], Reg[1]);
#endif

        return;
    }
    case 2://QueryMemory(MemoryInfo* info, PageInfo* out, u32 Addr)
    {
        u32 addr = Reg[2];
        struct MemoryInfo Minfo;
        struct PageInfo Pinfo;
        s32 ret = currentThread->m_owner->getMemoryMap()->QueryMemory(&Minfo, &Pinfo, addr);
        Reg[1] = Minfo.base_address;
        Reg[2] = Minfo.size;
        Reg[3] = Minfo.perm & 0x3;
        Reg[4] = Minfo.state & 0xFF;
        Reg[5] = Pinfo.flags;
        Reg[0] = ret;
#ifdef SWILOG
        LOG("Process %s thread %u QueryMemory (%08x | %08x %08x %08x %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, addr, Reg[0], Reg[1], Reg[2], Reg[3], Reg[4], Reg[5]);
#endif
        return;
    }
	case 0x5: //SetProcessAffinityMask(Handle process, u8* affinitymask, s32 processorcount) 
	{
		u32 process = Reg[0];
		u32 affinitymask = Reg[1];
		u32 processorcount = Reg[2];
		Reg[0] = 0;
#ifdef SWILOG
		LOG("Process %s thread %u SetProcessAffinityMask stub (%08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, process, affinitymask, processorcount, Reg[0]);
#endif
		return;
	}
	case 0x7: //SetProcessIdealProcessor(Handle process, s32 idealprocessor) 
	{
		u32 process = Reg[0];
		u32 idealprocessor = Reg[1];
		Reg[0] = 0;
#ifdef SWILOG
		LOG("Process %s thread %u SetProcessAffinityMask stub (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, process, idealprocessor, Reg[0]);
#endif
		return;
	}
    case 0x8: //CreateThread(Handle* thread, func entrypoint, u32 arg, u32 stacktop, s32 threadpriority, s32 processorid) 
    {
        u32 entrypoint = Reg[1];
        u32 arg = Reg[2];
        u32 stacktop = Reg[3];
        u32 processorid = Reg[4];
        s32 threadpriority = Reg[0];

        KThread * thread = new KThread(processorid, currentThread->m_owner);
        thread->m_context.reg_15 = entrypoint & ~0x1;
        thread->m_context.cpu_registers[0] = arg;
        thread->m_context.pc = entrypoint;
        thread->m_context.sp = stacktop;
        thread->m_thread_prio = threadpriority;

		if (entrypoint & 0x1)
		{
			thread->m_context.cpsr = 0x30; //User,THUMB
		}
		else
		{
			thread->m_context.cpsr = 0x10; //User
		}

        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, thread);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateThread %u (%08x %08x %08x %08x %08x | %08x %08x)", 
                currentThread->m_owner->GetName(), currentThread->m_thread_id, thread->m_thread_id, 
                entrypoint, arg,  stacktop,  threadpriority, processorid, 
                Reg[0], Reg[1]);
#endif
            delete thread;
            return;
        }

        currentThread->m_owner->AddThread(thread);

        Reg[0] = 0;
        Reg[1] = hand;

#ifdef SWILOG
        LOG("Process %s thread %u CreateThread %u (%08x %08x %08x %08x %08x | %08x %08x)", 
            currentThread->m_owner->GetName(), currentThread->m_thread_id, thread->m_thread_id,
            entrypoint, arg, stacktop, threadpriority, processorid,
            Reg[0], Reg[1]);
#endif
        return;
    }
	case 0x9: //ExitThread
	{
		currentThread->m_owner->m_Kernel->ReScheduler();
		currentThread->stop();
#ifdef SWILOG
		LOG("Process %s thread %u ExitThread", currentThread->m_owner->GetName(), currentThread->m_thread_id);
#endif
		return;
	}
    case 0xA: //SleepThread(s64 nanoseconds) 
    {
		u64 nanoseconds = Reg[0] | ((u64)Reg[1] << 32);
#ifdef SWILOG
		LOG("Process %s thread %u SleepThread %llu", currentThread->m_owner->GetName(), currentThread->m_thread_id, nanoseconds);
#endif
		KLinkedList<KSynchronizationObject> *list = new KLinkedList<KSynchronizationObject>();
		list->AddItem(currentThread);
		currentThread->SyncStall(list, true);

		currentThread->m_owner->m_Kernel->m_Timedevent.AddItem(currentThread);
		currentThread->m_owner->m_Kernel->FireNextTimeEvent(currentThread, nanoseconds + 1);

        currentThread->m_owner->m_Kernel->ReScheduler();
        return;
    }
    case 0xB: //GetThreadPriority(s32* priority, Handle thread)
    {
        u32 hand = Reg[1];
        KThread* th;
        if (hand == 0xffff8000)
            th = currentThread;
        else
            th = (KThread*)*currentThread->m_owner->GetHandleTable()->GetHandle<KThread>(hand);
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetThreadPriority (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[1] = th->m_thread_prio;
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u GetThreadPriority (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0xC: //SetThreadPriority(Handle thread, s32 priority) 
    {
        u32 hand = Reg[0];
        KThread* th;
        if (hand == 0xffff8000)
            th = currentThread;
        else
            th = (KThread*)*currentThread->m_owner->GetHandleTable()->GetHandle<KThread>(hand);
        if (th == NULL)
        {

            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SetThreadPriority (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[1], Reg[0]);
#endif 
            return;
        }
        th->m_thread_prio = Reg[1];
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u SetThreadPriority (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[1], Reg[0]);
#endif 
        return;
    }
	case 0x12: //Run(Handle process, StartupInfo* info) 
	{
		u32 handle = Reg[0];
		u32 prio = Reg[1];
		u32 stacksize = Reg[2];
		u32 argc = Reg[3]; //not used
		u32 argv = Reg[4]; //not used
		u32 envp = Reg[5]; //not used
		u32 unused;
#ifdef SWILOG
		LOG("Process %s thread %u Run (%08x %08x %08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, prio, stacksize, argc, argv, envp, Reg[0]);
#endif
		KProcess* process = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(handle);
		if (process == NULL)
		{
			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u Run (%08x %08x %08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, prio, stacksize, argc, argv, envp, Reg[0]);
#endif
			return;
		}
		process->getMemoryMap()->ControlMemory(&unused, 0x10000000 - stacksize, 0, stacksize, OPERATION_COMMIT, PERMISSION_RW);

		KThread * thread = new KThread(SERVICECORE, process); //todo find out when to use the other core
		u32 startaddr = (process->m_exheader_flags & (1 << 12)) ? 0x14000000 : 0x00100000;
		thread->m_context.reg_15 = startaddr;
		thread->m_context.pc = startaddr;
		thread->m_context.sp = 0x10000000;
		thread->m_context.cpsr = 0x10; //User
		process->AddThread(thread);

		Reg[0] = 0;

#ifdef SWILOG
		LOG("Process %s thread %u Run (%08x %08x %08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, prio, stacksize, argc, argv, envp, Reg[0]);
#endif
		return;

	}
    case 0x13: //CreateMutex(Handle* mutex, bool initialLocked)
    {
        KMutex* lim = new KMutex(currentThread->m_owner, Reg[1]);
        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, lim);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateMutex (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u CreateMutex (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x14: //ReleaseMutex(Handle mutex) 
    {
        u32 hand = Reg[0];
        KMutex* th = (KMutex*)*currentThread->m_owner->GetHandleTable()->GetHandle<KMutex>(hand);
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u ReleaseMutex (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif
            return;
        }
        th->Release();
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u ReleaseMutex (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif
        return;
    }
    case 0x15: //CreateSemaphore(Handle* semaphore, s32 initialCount, s32 maxCount) 
    {
        u32 count = Reg[1];
        u32 maxcount = Reg[2];
        KSemaphore * sema = new KSemaphore(maxcount - count, maxcount, currentThread->m_owner);
        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, sema);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateSemaphore (%08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, count, maxcount, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u CreateSemaphore (%08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, count, maxcount, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x16: //ReleaseSemaphore(s32* count, Handle semaphore, s32 releaseCount) 
    {
        u32 hand = Reg[1];
        s32 releaseCount = Reg[2];
        KSemaphore* th = (KSemaphore*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSemaphore>(hand);
        if (th == NULL)
        {

            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u ReleaseSemaphore (%08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, releaseCount, Reg[0], Reg[1]);
#endif 
            return;
        }
        u32 out_count = 0;
        Reg[0] = th->ReleaseSemaphore(releaseCount, out_count);
        Reg[1] = out_count;
#ifdef SWILOG
        LOG("Process %s thread %u ReleaseSemaphore (%08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, releaseCount, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x17: //CreateEvent(Handle* event, ResetType resettype) 
    {
        u32 resettype = Reg[1];
        KEvent* eve = new KEvent(0, resettype, currentThread->m_owner);
        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, eve);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateEvent (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, resettype, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u CreateEvent (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, resettype, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x18: //SignalEvent(Handle event) 
    {
        u32 hand = Reg[0];
        KEvent* th = (KEvent*)*currentThread->m_owner->GetHandleTable()->GetHandle<KEvent>(hand);
        if (th == NULL)
        {

            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SignalEvent (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
            return;
        }
        //free all and open
        th->Triggerevent();
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u SignalEvent (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
        return;
    }
	case 0x19: //ClearEvent(Handle event)
	{
		u32 hand = Reg[0];
		KEvent* th = (KEvent*)*currentThread->m_owner->GetHandleTable()->GetHandle<KEvent>(hand);
		if (th == NULL)
		{

			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u ClearEvent (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
			return;
		}
		//free all and open
		th->Clear();
		Reg[0] = 0;
#ifdef SWILOG
#ifdef SHOWCLEAREVENT
		LOG("Process %s thread %u ClearEvent (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
#endif 
		return;
	}
	case 0x1A: //CreateTimer(Handle* timer, ResetType resettype) 
	{
		u32 resettype = Reg[1];
		KTimer* time = new KTimer(currentThread->m_owner, resettype);
		u32 hand = 0;
		s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, time);
		if (ret != Success)
		{
			Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
			LOG("Process %s thread %u CreateTimer (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, resettype, Reg[0], Reg[1]);
#endif
			return;
		}
		Reg[0] = 0;
		Reg[1] = hand;
#ifdef SWILOG
		LOG("Process %s thread %u CreateTimer (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, resettype, Reg[0], Reg[1]);
#endif
		return;
	}
	case 0x1B://SetTimer(Handle timer, s64 initial, s64 interval) 
	{
		u32 hand = Reg[0];
		s64 initial = ((u64)Reg[3] << 32) | Reg[2];
		s64 interval = ((u64)Reg[4] << 32) | Reg[1];

		KTimer* th = (KTimer*)*currentThread->m_owner->GetHandleTable()->GetHandle<KTimer>(hand);
		if (th == NULL)
		{
			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u SetTimer (%08x %I64d %I64d | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, initial, interval, Reg[0]);
#endif 
			return;
		}
		Reg[0] = th->SetTimer(initial, interval);
#ifdef SWILOG
		LOG("Process %s thread %u SetTimer (%08x %I64d %I64d | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, initial, interval, Reg[0]);
#endif 
		return;
	}
	case 0x1C://CancelTimer(Handle timer)
	{
		u32 hand = Reg[0];

		KTimer* th = (KTimer*)*currentThread->m_owner->GetHandleTable()->GetHandle<KTimer>(hand);
		if (th == NULL)
		{
			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u CancelTimer (%08x| %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
			return;
		}
		th->Cancel();
		Reg[0] = Success;
#ifdef SWILOG
		LOG("Process %s thread %u CancelTimer (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0]);
#endif 
		return;
	}
	case 0x1E: //CreateMemoryBlock(Handle* memblock, u32 addr, u32 size, u32 mypermission, u32 otherpermission)
	{
		u32 otherperm = Reg[0];
		u32 addr = Reg[1];
		u32 size = Reg[2];
		u32 myperm = Reg[3];
		KSharedMemory* smem = new KSharedMemory(addr,size , myperm, otherperm, currentThread->m_owner);
		u32 hand = 0;
		s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, smem);
		if (ret != Success)
		{
			Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
			LOG("Process %s thread %u CreateMemoryBlock (%08x %08x %08x %08x| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, otherperm, addr, size, myperm, Reg[0], Reg[1]);
#endif
			return;
		}
		Reg[0] = 0;
		Reg[1] = hand;
#ifdef SWILOG
		LOG("Process %s thread %u CreateMemoryBlock (%08x %08x %08x %08x| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, otherperm, addr, size, myperm, Reg[0], Reg[1]);
#endif
		return;
	}
	case 0x1F: //MapMemoryBlock(Handle memblock, u32 addr, u32 mypermissions, u32 otherpermission) 
	{
		u32 handle = Reg[0];
		u32 addr = Reg[1];
		u32 myperm = Reg[2];
		u32 otherperm = Reg[3];
		KSharedMemory* th = (KSharedMemory*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSharedMemory>(handle);
		if (th == NULL)
		{
			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u MapMemoryBlock (%08x %08x %08x %08x| %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, addr, myperm, otherperm, Reg[0]);
#endif    
			return;
		}
		Reg[0] = th->map(addr,myperm,otherperm,currentThread->m_owner);
#ifdef SWILOG
		LOG("Process %s thread %u MapMemoryBlock (%08x %08x %08x %08x| %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, addr, myperm, otherperm, Reg[0]);
#endif
		return;
	}
    case 0x21: //CreateAddressArbiter(Handle* arbiter)
    {
        KAddressArbiter* lim = new KAddressArbiter(currentThread->m_owner);
        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, lim);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateAddressArbiter (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u CreateAddressArbiter (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x22: //ArbitrateAddress(Handle arbiter, u32 addr, ArbitrationType type, s32 value, s64 nanoseconds)
    {
        u32 arbiter = Reg[0];
        u32 addr = Reg[1];
        u32 type = Reg[2];
        s32 value = Reg[3];
        s64 time = Reg[4] | ((u64)Reg[3] << 32);

        KAddressArbiter* th = (KAddressArbiter*)*currentThread->m_owner->GetHandleTable()->GetHandle<KAddressArbiter>(arbiter);
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u ArbitrateAddress (%08x %08x %08x %08x| %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, arbiter, addr, type, value, Reg[0]);
#endif    
            return;
        }
        if (addr & 3) {
            Reg[0] = 0xD8E007F1;
#ifdef SWILOG
            LOG("Process %s thread %u ArbitrateAddress (%08x %08x %08x %08x| %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, arbiter, addr, type, value, Reg[0]);
#endif
            return;
        }
        Reg[0] = th->ArbitrateAddress(addr,type, value, time, currentThread);


#ifdef SWILOG
        LOG("Process %s thread %u ArbitrateAddress (%08x %08x %08x %08x| %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, arbiter, addr, type, value, Reg[0]);
#endif
        return;
    }
    case 0x23: //CloseHandle(Handle handle)
    {
        u32 handle = Reg[0];
        s32 ret = currentThread->m_owner->GetHandleTable()->CloseHandle(handle); //TODO if this closes the Process the Process gets terminated?
        if (ret != Success)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u CloseHandle (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0]);
#endif
        }
        else
        {
            Reg[0] = 0;
#ifdef SWILOG
            LOG("Process %s thread %u CloseHandle (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0]);
#endif
        }
        return;
    }
    case 0x24://WaitSynchronization1(Handle handle, s64 nanoseconds) 
    {
        u32 handle = Reg[0];
        u64 timeout = Reg[2] | ((u64)Reg[3] << 32);

        KSynchronizationObject* th = (KSynchronizationObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSynchronizationObject>(handle);
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u WaitSynchronization1 (%08x %" PRIx64 " | % 08x % 08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, timeout, Reg[0], Reg[1]);
#endif    
            return;
        }

        KLinkedList<KSynchronizationObject> *list = new KLinkedList<KSynchronizationObject>();
        list->AddItem(th);
        currentThread->SyncStall(list, true);
#ifdef SWILOG
        LOG("Process %s thread %u WaitSynchronization1 (%08x %" PRIx64 ") stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, timeout);
#endif    
        return;
    }
    case 0x25: //WaitSynchronizationN(s32* out, Handle* handles, s32 handlecount, bool waitAll, s64 nanoseconds)
    {
        u32 pointer = Reg[1];
        s32 handleCount = Reg[2];
        u32 waitall = Reg[3];
        u64 timeout = Reg[0] | ((u64)Reg[4] << 32);

        KLinkedList<KSynchronizationObject> *list = new KLinkedList<KSynchronizationObject>();
        for (int i = 0; i < handleCount; i++)
        {
            u32 handle;
            if (currentThread->m_owner->getMemoryMap()->Read32(pointer + i * 4, handle) != Success)
            {
                Reg[0] = -1;
#ifdef SWILOG
                LOG("Process %s thread %u WaitSynchronizationN (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, waitall, Reg[0], Reg[1]);
#endif
                delete list;
                return;
            }
            KSynchronizationObject* th = (KSynchronizationObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSynchronizationObject>(handle);
#ifdef SWILOG
            LOG("handle: %08x", handle);
#endif
            if (th) //todo send error message
                list->AddItem(th);
        }
        currentThread->SyncStall(list, waitall);
#ifdef SWILOG
        LOG("Process %s thread %u WaitSynchronizationN (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, waitall, Reg[0], Reg[1]);
#endif
        return;
    }

    case 0x27: //DuplicateHandle(Handle* out, Handle original) 
    {
        s32 ret;
        KAutoObjectRef obj;
        u32 original = Reg[1];
        if (original == 0xFFFF8000)
        {
            obj.SetObject(currentThread);
        }
        else if (original == 0xFFFF8001)
        {
            obj.SetObject(currentThread->m_owner);
        }
        else
        {
            ret = currentThread->m_owner->GetHandleTable()->GetHandleObject(obj, original);
            if (ret != Success)
            {
                Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
                LOG("Process %s thread %u DuplicateHandle (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, original, Reg[0],Reg[1]);
#endif
                return;
            }
        }
        u32 hand = 0;
        ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, *obj);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u DuplicateHandle (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, original, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u DuplicateHandle (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, original, Reg[0], Reg[1]);
#endif
        return;
    }
	case 0x28:
	{
		Reg[0] = currentThread->m_core->Getticks();
		Reg[1] = currentThread->m_core->Getticks() >> 32;
#ifdef SWILOG
		LOG("Process %s thread %u GetSystemTick ( | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
	}

    case 0x2A: //GetSystemInfo(u64*out,Handle process, ProcessInfoType type)
    {
        u32 type = Reg[1];
        u32 param = Reg[2];
        switch (type)
        {
        case 26:
            Reg[0] = 0;
            Reg[1] = currentThread->m_owner->m_Kernel->m_numbFirmProcess;
            Reg[2] = 0;
            break;
        default:
            XDSERROR("unknown GetSystemInfo enum %u", type);
            Reg[0] = 0;
            break;
        }
#ifdef SWILOG
        LOG("Process %s thread %u GetSystemInfo (%08x %08x | %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, type, param, Reg[0], Reg[1], Reg[2]);
#endif
        return;
    }

    case 0x2B: //GetProcessInfo(u64*out,Handle process, ProcessInfoType type)
    {
        u32 hand = Reg[1];
        u32 type = Reg[2];
        switch (type)
        {
		case 2: //this gets the size of the data used by the Process
			LOG("get size ? used ? -stub- enum %u", type);
			Reg[0] = 0;
			Reg[1] = 0x20000;
			Reg[2] = 0;
			break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
            Reg[0] = SVCERROR_INVALID_ENUM_VALUE;
            break;
        case 20:
            Reg[0] = 0;
            Reg[1] = 0x20000000 - currentThread->m_owner->LINEAR_memory_virtual_address_userland;
            Reg[2] = 0;
            break;
        default:
            XDSERROR("unknown GetProcessInfo enum %u",type);
            Reg[0] = SVCERROR_INVALID_ENUM_VALUE;
            break;
        }
#ifdef SWILOG
        LOG("Process %s thread %u GetProcessInfo (%08x %08x | %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, type, Reg[0], Reg[1], Reg[2]);
#endif
        return;
    }
    case 0x2C: //GetThreadInfo(s64* out, Handle thread, ThreadInfoType type)
    {
        u32 hand = Reg[0];
        u32 type = Reg[1];
        KThread* th = (KThread*)*currentThread->m_owner->GetHandleTable()->GetHandle<KThread>(hand);
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetThreadInfo (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, type, Reg[0]);
#endif
            return;
        }
        Reg[0] = SVCERROR_INVALID_ENUM_VALUE;
#ifdef SWILOG
        LOG("Process %s thread %u GetThreadInfo (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, type, Reg[0]);
#endif
        return;
    }

    case 0x2D: //ConnectToPort(Handle* out, const char* portName) 
    {
        u32 pointer = Reg[1];
        Reg[0] = 0xd88007fa; //obj was not found
        char name[9];
        for (int i = 0; i < 8; i++)
        {
            u8 data = 0;
            s32 ret = currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
            if (ret != Success)
            {
                Reg[0] = -1;
                //just a error
            }
            if (data == NULL)
            {
                name[i] = 0;
                break;
            }
            name[i] = data;
        }
        name[8] = 0;

        size_t name_len = strlen(name);
        //search for the correct handle
        KLinkedListNode<KPort> *temp = currentThread->m_owner->m_Kernel->m_Portlist.list;
        while (temp)
        {
            if (memcmp(temp->data->m_Name, name, strlen(temp->data->m_Name)) == 0)
            {
                if(strlen(temp->data->m_Name) == name_len)
                {
                    //found it
                    KClientSession* ses;
                    u32 hand;
                    if (temp->data->m_Client.connect(ses) == Success)
                    {
                        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, ses);
                        if (ret != Success)
                        {
                            Reg[0] = SVCERROR_CREATE_HANLE;
                        }
                        else
                        {
                            Reg[1] = hand;
                            Reg[0] = 0;
                        }
                    }
                    else
                    {
                        Reg[0] = -1; //todo the correct error
                    }
                    break;
                }
            }
            temp = temp->next;
        }
#ifdef SWILOG
        LOG("Process %s thread %u ConnectToPort (%08x | %08x %08x) stub", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, Reg[0], Reg[1]);
        for (int i = 0; i < 8; i++)
        {
            u8 data = 0;
            currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
            if (data == NULL)
                break;
            printf("%c", data);
        }
        printf("\n");
#endif        
        return;
    }
    case 0x32: //SendSyncRequest
    {
        u32 handle = Reg[0];
        KClientSession* KSession = (KClientSession*)*currentThread->m_owner->GetHandleTable()->GetHandle<KClientSession>(handle);
        if (KSession == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SendSyncRequest (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0]);
#endif
            return;
        }
        KLinkedList<KSynchronizationObject> *list = new KLinkedList<KSynchronizationObject>();
        list->AddItem(KSession);
        currentThread->SyncStall(list, true);
        Reg[0] = Success;
#ifdef SWILOG
        LOG("Process %s thread %u SendSyncRequest (%08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0]);
#endif

        return;
    }
    case 0x33: //OpenProcess(Handle* process, u32 processId) 
    {
        u32 ID = Reg[1];

        u32 outhand = 0;
        u32 hand;

        KLinkedListNode<KProcess>* proc =  currentThread->m_owner->m_Kernel->m_processes.list;

        KProcess * found = NULL;
        while (proc)
        {
            if (proc->data->m_ProcessID == ID)
            {
                found = proc->data;
                break;
            }
            proc = proc->next;
        }
        if (!proc)
        {
            Reg[0] = -1;
#ifdef SWILOG
            LOG("Process %s thread %u OpenProcess (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, ID, Reg[0], Reg[1]);
#endif
            return;
        }

        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, found);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u OpenProcess (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, ID, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[1] = hand;
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u OpenProcess (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, ID, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x35: //GetProcessId(u32* processId, Handle process)
    {
        u32 hand = Reg[1];
        KProcess* th;
        if (hand == 0xFFFF8001)
        {
            th = currentThread->m_owner;
        }
        else
        {
            th = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(hand);
        }
        if (th == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetProcessId (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[1] = th->GetProcessID();
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u GetProcessId (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x37: //GetThreadId(u32* threadId, Handle thread) 
    {
        u32 hand = Reg[1];
        KThread* pr;
        if (hand == 0xffff8000)
            pr = currentThread;
        else
            pr = (KThread*)*currentThread->m_owner->GetHandleTable()->GetHandle<KThread>(hand);
        if (pr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetThreadId (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif           
            return;
        }
        Reg[0] = 0;
        Reg[1] = pr->m_thread_id;
#ifdef SWILOG
        LOG("Process %s thread %u GetThreadId (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif   
        return;
    }
    case 0x38: //GetResourceLimit(Handle* resourceLimit, Handle process) 
    {
        u32 hand = Reg[0];

        KProcess* pr = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(hand);
        if (pr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimit (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0],Reg[1]);
#endif           
            return;
        }
        u32 outhand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, currentThread->m_owner->GetResourceLimit());
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimit (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[1] = outhand;
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u GetResourceLimit (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, hand, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x39: //GetResourceLimitLimitValues(s64* values, Handle resourceLimit, LimitableResource* names, s32 nameCount)
    {
        u32 values_ptr = Reg[0];
        u32 handleResourceLimit = Reg[1];
        u32 names_ptr = Reg[2];
        u32 nameCount = Reg[3];

        if (names_ptr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_POINTER;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimitLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }
        if (nameCount <= 0)
        {
            Reg[0] = SVCERROR_OUT_OF_RANGE;
            return;
        }

        KResourceLimit* pr = (KResourceLimit*)*currentThread->m_owner->GetHandleTable()->GetHandle<KResourceLimit>(handleResourceLimit);
        if (pr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimitLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }


        for (u32 i = 0; i < nameCount; i++)
        {
            u32 data;

            s32 ret = currentThread->m_owner->getMemoryMap()->Read32(names_ptr + i * 4, data); //read from user mode
            if (ret != Success)
            {
                Reg[0] = SVCERROR_INVALID_PARAMS;
#ifdef SWILOG
                LOG("Process %s thread %u GetResourceLimitLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }
            if (data >= 0xA)
            {
                Reg[0] = SVCERROR_OUT_OF_RANGE;
#ifdef SWILOG
                LOG("Process %s thread %u GetResourceLimitLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }

            s64 out_data = pr->GetMaxValue(data); //gets data from KResourceLimitobj + 0x8
#ifdef SWILOG
            LOG("%08x -> %08x", data, (u32)out_data);
#endif
            currentThread->m_owner->getMemoryMap()->Write64(names_ptr + i * 8, out_data);
        }
#ifdef SWILOG
        LOG("Process %s thread %u GetResourceLimitLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
        Reg[0] = 0;
        return;
    }
    case 0x3A: //GetResourceLimitCurrentValues(s64* values, Handle resourceLimit, LimitableResource* names, s32 nameCount) 
    {
        u32 values_ptr = Reg[0];
        u32 handleResourceLimit = Reg[1];
        u32 names_ptr = Reg[2];
        u32 nameCount = Reg[3];

        if (names_ptr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_POINTER;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }
        if (nameCount <= 0)
        {
            Reg[0] = SVCERROR_OUT_OF_RANGE;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }

        KResourceLimit* pr = (KResourceLimit*)*currentThread->m_owner->GetHandleTable()->GetHandle<KResourceLimit>(handleResourceLimit);
        if (pr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }


        for (u32 i = 0; i < nameCount; i++)
        {
            u32 data;

            s32 ret = currentThread->m_owner->getMemoryMap()->Read32(names_ptr + i * 4, data); //read from user mode
            if (ret != Success)
            {
                Reg[0] = SVCERROR_INVALID_PARAMS;
#ifdef SWILOG
                LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }
            if (data >= 0xA)
            {
                Reg[0] = SVCERROR_OUT_OF_RANGE;
#ifdef SWILOG
                LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }

            s64 out_data = pr->GetCurrentValue(data);
#ifdef SWILOG
            LOG("%08x -> %08x", data, (u32)out_data);
#endif
            currentThread->m_owner->getMemoryMap()->Write64(names_ptr + i * 8, out_data);
        }

        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u GetResourceLimitCurrentValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
        return;
    }
	case 0x3D:
	{
		LOG("Process %s thread %u DEBUG output:", currentThread->m_owner->GetName(), currentThread->m_thread_id);
		for (size_t sz = Reg[1]; sz > 0; sz--)
		{
			u8 data;
			if (currentThread->m_owner->getMemoryMap()->Read8(Reg[0]++, data))
				break;
			putchar(data);
		}
		LOG("");
		Reg[0] = 0; //there are some cases where this errors but we don't emulate them
		return;
	}
    case 0x47: //CreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions)
    {
        u32 pointer = Reg[2];
        u32 maxhandle = Reg[3];
        Reg[0] = 0xd88007fa; //obj was not found

        char name[9];
        for (int i = 0; i < 8; i++)
        {
            u8 data = 0;
            s32 ret = currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
            if (ret != Success)
            {
                Reg[0] = -1;
                //just a error
            }
            if (data == NULL)
            {
                name[i] = 0;
                break;
            }
            name[i] = data;
        }
        name[8] = 0;
        KPort *port = new KPort(name, maxhandle);
        currentThread->m_owner->m_Kernel->m_Portlist.AddItem(port);

        u32 hand1 = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand1, &port->m_Server);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreatePort (%08x %08x | %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, maxhandle, Reg[0], Reg[1], Reg[2]);
            for (int i = 0; i < 8; i++)
            {
                u8 data = 0;
                currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
                if (data == NULL)
                    break;
                printf("%c", data);
            }
            printf("\n");
#endif 
            return;
        }
        u32 hand2 = 0;
        ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand2, &port->m_Client);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreatePort (%08x %08x | %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, maxhandle, Reg[0], Reg[1], Reg[2]);
            for (int i = 0; i < 8; i++)
            {
                u8 data = 0;
                currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
                if (data == NULL)
                    break;
                printf("%c", data);
            }
            printf("\n");
#endif 
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand1;
        Reg[2] = hand2;

#ifdef SWILOG
        LOG("Process %s thread %u CreatePort (%08x %08x | %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, maxhandle, Reg[0], Reg[1], Reg[2]);
        for (int i = 0; i < 8; i++)
        {
            u8 data = 0;
            currentThread->m_owner->getMemoryMap()->Read8(pointer + i, data);
            if (data == NULL)
                break;
            printf("%c", data);
        }
        printf("\n");
#endif        
        return;
    }

    case 0x48: //CreateSessionToPort(Handle* session, Handle port) 
    {
        u32 handle = Reg[1];
        KClientPort* SPort = (KClientPort*)*currentThread->m_owner->GetHandleTable()->GetHandle<KClientPort>(handle);
        if (SPort == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateSessionToPort (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0], Reg[1]);
#endif
            return;
        }

        //found it
        KClientSession* ses;
        u32 hand;
        if (SPort->connect(ses) == Success)
        {
            s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, ses);
            if (ret != Success)
            {
                Reg[0] = SVCERROR_CREATE_HANLE;
            }
            else
            {
                Reg[1] = hand;
                Reg[0] = 0;
            }
        }
        else
        {
            Reg[0] = -1; //todo the correct error
        }
#ifdef SWILOG
        LOG("Process %s thread %u CreateSessionToPort (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0], Reg[1]);
#endif        
        return;
    }
    case 0x49: //CreateSession(Handle* sessionServer, Handle* sessionClient) 
    {
        KSession* sesi = new KSession();
        u32 hand1,hand2;

        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand1, &sesi->m_Server);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateSession (| %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1], Reg[2]);
#endif
            return;
        }
        ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand2, &sesi->m_Client);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateSession (| %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1], Reg[2]);
#endif
            return;
        }

        Reg[0] = 0;
        Reg[1] = hand1;
        Reg[2] = hand2;
#ifdef SWILOG
        LOG("Process %s thread %u CreateSession (| %08x %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1], Reg[2]);
#endif
        return;
    }
    case 0x4A: //AcceptSession(Handle* session, Handle port)
    {
        u32 handle = Reg[1];
        KServerPort* SPort = (KServerPort*)*currentThread->m_owner->GetHandleTable()->GetHandle<KServerPort>(handle);
        if (SPort == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u AcceptSession (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0], Reg[1]);
#endif
            return;
        }
        u32 hand = 0;
        KServerSession* ses = SPort->AcceptSesion();

        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, ses);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u AcceptSession (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0], Reg[1]);
#endif
            return;
        }

        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u AcceptSession (%08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x4F: //ReplyAndReceive(s32* index, Handle* handles, s32 handleCount, Handle replyTarget)
    {
        u32 pointer = Reg[1];
        s32 handleCount = Reg[2];
        u32 replyTarget = Reg[3];

        if (replyTarget != NULL)
        {
            KServerSession* SSession = (KServerSession*)*currentThread->m_owner->GetHandleTable()->GetHandle<KServerSession>(replyTarget);
            if (SSession == NULL)
            {
                Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
                LOG("Process %s thread %u ReplyAndReceive (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, replyTarget, Reg[0], Reg[1]);
#endif
                return;
            }
            s32 ret = SSession->reply(currentThread);
            /*if (ret != Success)
            {
                Reg[0] = ret;
#ifdef SWILOG
                LOG("Process %s thread %u ReplyAndReceive (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, replyTarget, Reg[0], Reg[1]);
#endif
                return;
            }*/ //don't do anything here
        }

        KLinkedList<KSynchronizationObject> *list = new KLinkedList<KSynchronizationObject>();
        for (int i = 0; i < handleCount; i++)
        {
            u32 handle;
            if (currentThread->m_owner->getMemoryMap()->Read32(pointer + i * 4, handle) != Success)
            {
                Reg[0] = -1;
#ifdef SWILOG
                LOG("Process %s thread %u ReplyAndReceive (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, replyTarget, Reg[0], Reg[1]);
#endif
                delete list;
                return;
            }
            KSynchronizationObject* th = (KSynchronizationObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSynchronizationObject>(handle);
#ifdef SWILOG
            LOG("handle: %08x", handle);
#endif
            if (th) //todo send error message
                list->AddItem(th);
        }
        currentThread->SyncStall(list, false);
#ifdef SWILOG
        LOG("Process %s thread %u ReplyAndReceive (%08x %08x %08x | %08x %08x) stub + not 100 correct", currentThread->m_owner->GetName(), currentThread->m_thread_id, pointer, handleCount, replyTarget, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x50: //BindInterrupt(Interrupt name, Handle syncObject, s32 priority, bool isManualClear)
    {
        s32 name = Reg[0];
        Handle syncObject = Reg[1];
        s32 priority = Reg[2];
        u32 isManualClear = Reg[3];

        KSynchronizationObject* obj = (KSynchronizationObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSynchronizationObject>(syncObject);
        if (obj == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u BindInterrupt (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, name, syncObject, priority, isManualClear, Reg[0]);
#endif
            return;
        }
        Reg[0] = currentThread->m_owner->m_Kernel->RegisterInterrupt(name, obj, priority, isManualClear);
#ifdef SWILOG
        LOG("Process %s thread %u BindInterrupt (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, name, syncObject, priority, isManualClear, Reg[0]);
#endif
        return;
    }
    case 0x51: //UnbindInterrupt(Interrupt name, Handle syncObject) 
    {
        s32 name = Reg[0];
        Handle syncObject = Reg[1];

        KSynchronizationObject* obj = (KSynchronizationObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KSynchronizationObject>(syncObject);
        if (obj == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u UnbindInterrupt (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, name, syncObject, Reg[0]);
#endif
            return;
        }
        Reg[0] = currentThread->m_owner->m_Kernel->UnRegisterInterrupt(name, obj);
#ifdef SWILOG
        LOG("Process %s thread %u UnbindInterrupt (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, name, syncObject, Reg[0]);
#endif
        return;
    }
	case 0x53: //StoreProcessDataCache
	{
		u32 addr = Reg[1];
		u32 size = Reg[2];
		Reg[0] = 0;

#ifdef SWILOG
		LOG("Process %s thread %u StoreProcessDataCache (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, addr, size, Reg[0]);
#endif
		return;
	}
	case 0x54: //FlushProcessDataCache
	{
		u32 addr = Reg[1];
		u32 size = Reg[2];
		Reg[0] = 0;

#ifdef SWILOG
		LOG("Process %s thread %u FlushProcessDataCache (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, addr, size, Reg[0]);
#endif
		return;
	}
	case 0x55://StartInterProcessDma(Handle* dma, Handle dstProcess, void* dst, Handle srcProcess, const void* src, u32 size, const DmaConfig& config)
	{
		u32 srcAddress = Reg[0];
		Handle srcProcessID = Reg[1];
		u32 dstAddress = Reg[2];
		Handle dstProcessID = Reg[3];
		u32 size = Reg[4];
		u32 config = Reg[5];

		KProcess * dstProcess;
		KProcess * srcProcess;
		if (dstProcessID == 0xFFFF8001)
		{
			dstProcess = currentThread->m_owner;
		}
		else
		{
			dstProcess = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(dstProcessID);
		}

		if (srcProcessID == 0xFFFF8001)
		{
			srcProcess = currentThread->m_owner;
		}
		else
		{
			srcProcess = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(srcProcessID);
		}

		struct DmaConfig dmaConfig;
        currentThread->m_owner->getMemoryMap()->ReadN(config, (u8*)&dmaConfig, sizeof(struct DmaConfig));

		if (!dstProcess || !srcProcess)
		{
			Reg[0] = -1;
#ifdef SWILOG
			LOG("Process %s thread %u StartInterProcessDma failed (%08x %08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, dstProcessID, dstAddress, srcProcessID, srcAddress, size, config);
#endif
			return;
		}

		//TODO: Init with params from DmaConfig data
		KDmaObject* lim = new KDmaObject(0,2,currentThread->m_owner);
		u32 dma_hand = 0;
		s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(dma_hand, lim);
		if (ret != Success)
		{
			Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
			LOG("Process %s thread %u StartInterProcessDma (%s %08x %s %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, dstProcess->GetName(), dstAddress, srcProcess->GetName(), srcAddress, size, config);
#endif
			return;
		}

#ifdef SWILOG
        LOG("Process %s thread %u StartInterProcessDma (%s %08x %s %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, dstProcess->GetName(), dstAddress, srcProcess->GetName(), srcAddress, size, config);
        LOG("DmaConfig:");
        LOG("  channel_sel: %d", dmaConfig.channel_sel);
        LOG("  endian_swap_size: %d (0x%02X)", dmaConfig.endian_swap_size, dmaConfig.endian_swap_size);
        LOG("  flags: %d (0x%02X)", dmaConfig.flags, dmaConfig.flags);
        LOG("  padding: %d (0x%02X)", dmaConfig.padding, dmaConfig.padding);
        LOG("  SubDmaConfig Destination:");
        LOG("    peripheral_id: %d (0x%02X)", dmaConfig.dst_cfg.peripheral_id, dmaConfig.dst_cfg.peripheral_id);
        LOG("    type: %d (0x%02X)", dmaConfig.dst_cfg.type, dmaConfig.dst_cfg.type);
        LOG("    unk3: %d (0x%04X)", dmaConfig.dst_cfg.unk3, dmaConfig.dst_cfg.unk3);
        LOG("    transfer_size: %d (0x%04X)", dmaConfig.dst_cfg.transfer_size, dmaConfig.dst_cfg.transfer_size);
        LOG("    unk4: %d (0x%04X)", dmaConfig.dst_cfg.unk4, dmaConfig.dst_cfg.unk4);
        LOG("    transfer_stride: %d (0x%04X)", dmaConfig.dst_cfg.transfer_stride, dmaConfig.dst_cfg.transfer_stride);
        LOG("  SubDmaConfig Source:");
        LOG("    peripheral_id: %d (0x%02X)", dmaConfig.src_cfg.peripheral_id, dmaConfig.src_cfg.peripheral_id);
        LOG("    type: %d (0x%02X)", dmaConfig.src_cfg.type, dmaConfig.src_cfg.type);
        LOG("    unk3: %d (0x%04X)", dmaConfig.src_cfg.unk3, dmaConfig.src_cfg.unk3);
        LOG("    transfer_size: %d (0x%04X)", dmaConfig.src_cfg.transfer_size, dmaConfig.src_cfg.transfer_size);
        LOG("    unk4: %d (0x%04X)", dmaConfig.src_cfg.unk4, dmaConfig.src_cfg.unk4);
        LOG("    transfer_stride: %d (0x%04X)", dmaConfig.src_cfg.transfer_stride, dmaConfig.src_cfg.transfer_stride);

#endif
		for (u32 i = 0; i < size;)
		{
			u8 val8;
			u16 val16;
			u32 val32;
			switch (dmaConfig.dst_cfg.type)
			{
			case 1:
				srcProcess->getMemoryMap()->Read8(srcAddress + i, val8);
				dstProcess->getMemoryMap()->Write8(dstAddress + (i % dmaConfig.dst_cfg.transfer_stride), val8);
				i++;
				break;
			case 2:
				srcProcess->getMemoryMap()->Read16(srcAddress + i, val16);
				dstProcess->getMemoryMap()->Write16(dstAddress + (i % dmaConfig.dst_cfg.transfer_stride), val16);
				i+=2;
				break;
			case 4:
				srcProcess->getMemoryMap()->Read32(srcAddress + i, val32);
				dstProcess->getMemoryMap()->Write32(dstAddress + (i % dmaConfig.dst_cfg.transfer_stride), val32);
				i += 4;
				break;
			default:
				LOG("error dmaConfig.dst_cfg.type 0x%02X not supported yet", dmaConfig.dst_cfg.type);
				i++;
				break;
			}
		}

		Reg[1] = dma_hand;
		Reg[0] = 0;

		return;
	}
    case 0x56: //StopDma
    {
        u32 handle = Reg[0];

        KDmaObject *dmaObject = (KDmaObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KDmaObject>(handle);

        if(!dmaObject)
        {
            Reg[0] = 0xD8E007F7;
#ifdef SWILOG
            LOG("Process %s thread %u StopDma failed (%08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle);
#endif
            return;
        }

        Reg[0] = 0;

#ifdef SWILOG
        LOG("Process %s thread %u StopDma (%08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle);
#endif
        return;
    }
	case 0x57: //GetDmaState
	{
		u32 handle = Reg[1];

		KDmaObject *dmaObject = (KDmaObject*)*currentThread->m_owner->GetHandleTable()->GetHandle<KDmaObject>(handle);

		if (!dmaObject)
		{
			Reg[0] = 0xD8E007F7;
#ifdef SWILOG
			LOG("Process %s thread %u GetDmaState failed (%08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle);
#endif
			return;
		}

		Reg[0] = 0;
		Reg[1] = dmaObject->GetState();

#ifdef SWILOG
		LOG("Process %s thread %u GetDmaState (%08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handle, Reg[1]);
#endif
		return;
	}
	case 0x73: //CreateCodeSet(Handle* handle_out, struct CodeSetInfo, u32 code_ptr, u32 ro_ptr, u32 data_ptr)
	{
		u32 CodeSetInfo = Reg[1];
		u32 code_ptr = Reg[2];
		u32 ro_ptr = Reg[3];
		u32 data_ptr = Reg[0];
		u32 hand = 0;

		struct CodeSetInfo cs;
		currentThread->m_owner->getMemoryMap()->ReadN(CodeSetInfo, (u8*)&cs, sizeof(struct CodeSetInfo));

		u8* codebuffer = (u8*)malloc(cs.TEXTSize*PAGE_SIZE); //todo check if the alloc worked
		u8* robuffer = (u8*)malloc(cs.ROSize * PAGE_SIZE);
		u8* databuffer = (u8*)malloc(cs.RWSize * PAGE_SIZE);

		currentThread->m_owner->getMemoryMap()->ReadN(code_ptr, codebuffer, cs.TEXTSize*PAGE_SIZE);
		currentThread->m_owner->getMemoryMap()->ReadN(ro_ptr, robuffer, cs.ROSize * PAGE_SIZE);
		currentThread->m_owner->getMemoryMap()->ReadN(data_ptr, databuffer, cs.RWSize * PAGE_SIZE);

		char othername[9];
		strncpy(othername, (char*)cs.Name, 8);
		KCodeSet* CSet = new KCodeSet(codebuffer, cs.PagesTEXT, robuffer, cs.PagesRO, databuffer, cs.RWSize,cs.PagesRW - cs.RWSize, Read64(cs.TitleID), othername); //todo check if the buffer are big enough

		if (currentThread->m_owner->getMemoryMap()->RemovePages(code_ptr, cs.TEXTSize*PAGE_SIZE) != 0)
		{
			Reg[0] = 0xE0A01BF5;
#ifdef SWILOG
			LOG("Process %s thread %u CreateCodeSet (%08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, CodeSetInfo, code_ptr, ro_ptr, data_ptr, Reg[0], Reg[1]);
#endif
			return;
		}
		if (currentThread->m_owner->getMemoryMap()->RemovePages(ro_ptr, cs.ROSize*PAGE_SIZE) != 0)
		{
			Reg[0] = 0xE0A01BF5;
#ifdef SWILOG
			LOG("Process %s thread %u CreateCodeSet (%08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, CodeSetInfo, code_ptr, ro_ptr, data_ptr, Reg[0], Reg[1]);
#endif
			return;
		}
		if (currentThread->m_owner->getMemoryMap()->RemovePages(data_ptr, cs.RWSize*PAGE_SIZE) != 0)
		{
			Reg[0] = 0xE0A01BF5;
#ifdef SWILOG
			LOG("Process %s thread %u CreateCodeSet (%08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, CodeSetInfo, code_ptr, ro_ptr, data_ptr, Reg[0], Reg[1]);
#endif
			return;
		}

		free(codebuffer);
		free(robuffer);
		free(databuffer);

		s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, CSet);
		if (ret != Success)
		{
			Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
			LOG("Process %s thread %u CreateCodeSet (%08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, CodeSetInfo, code_ptr, ro_ptr, data_ptr, Reg[0], Reg[1]);
#endif
			return;
		}

		Reg[0] = 0;
		Reg[1] = hand;
#ifdef SWILOG
		LOG("Process %s thread %u CreateCodeSet (%08x %08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, CodeSetInfo, code_ptr, ro_ptr, data_ptr, Reg[0], Reg[1]);
#endif
		return;
	}
	case 0x75: //CreateProcess(Handle* handle_out, Handle codeset_handle, u32 arm11kernelcaps_ptr, u32 arm11kernelcaps_num)
	{
		u32 arm11kernelcapsfild[0x80];
		u32 codeset_handle = Reg[1];
		u32 arm11kernelcaps_ptr = Reg[2];
		u32 arm11kernelcaps_num = Reg[3];
		u32 hand = 0;

		KCodeSet* Codeset = (KCodeSet*)*currentThread->m_owner->GetHandleTable()->GetHandle<KCodeSet>(codeset_handle);
		if (Codeset == NULL)
		{
			Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
			LOG("Process %s thread %u CreateProcess (%08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, codeset_handle, arm11kernelcaps_ptr, arm11kernelcaps_num, Reg[0], Reg[1]);
#endif
			return;
		}
		//todo this is a workaround
		currentThread->m_owner->getMemoryMap()->ReadN(arm11kernelcaps_ptr, (u8*)arm11kernelcapsfild, sizeof(arm11kernelcapsfild));
		KProcess* Pro = new KProcess(Codeset, arm11kernelcaps_num, arm11kernelcapsfild, currentThread->m_owner->m_Kernel, false);

		s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, Pro);
		if (ret != Success)
		{
			Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
			LOG("Process %s thread %u CreateProcess (%08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, codeset_handle, arm11kernelcaps_ptr, arm11kernelcaps_num, Reg[0], Reg[1]);
#endif
			return;
		}

		Reg[0] = 0;
		Reg[1] = hand;
#ifdef SWILOG
		LOG("Process %s thread %u CreateProcess (%08x %08x %08x | %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, codeset_handle, arm11kernelcaps_ptr, arm11kernelcaps_num, Reg[0], Reg[1]);
#endif
		return;
	}

    case 0x77: //SetProcessResourceLimits(Handle KProcess, Handle KResourceLimit)
    {
        u32 handleResourceLimit = Reg[1];
        u32 handleProcess = Reg[0];

        KResourceLimit* lim = (KResourceLimit*)*currentThread->m_owner->GetHandleTable()->GetHandle<KResourceLimit>(handleResourceLimit);
        if (lim == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SetProcessResourceLimits (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handleResourceLimit, handleProcess, Reg[0]);
#endif
            return;
        }

        KProcess* pr = (KProcess*)*currentThread->m_owner->GetHandleTable()->GetHandle<KProcess>(handleProcess);
        if (pr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SetProcessResourceLimits (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handleResourceLimit, handleProcess, Reg[0]);
#endif
            return;
        }
        pr->SetResourceLimit(lim);
        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u SetProcessResourceLimits (%08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, handleResourceLimit, handleProcess, Reg[0]);
#endif
        return;
    }
    case 0x78: //CreateResourceLimit(Handle *KResourceLimit) 
    {
        KResourceLimit* lim = new KResourceLimit();
        u32 hand = 0;
        s32 ret = currentThread->m_owner->GetHandleTable()->CreateHandle(hand, lim);
        if (ret != Success)
        {
            Reg[0] = SVCERROR_CREATE_HANLE;
#ifdef SWILOG
            LOG("Process %s thread %u CreateResourceLimit (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
            return;
        }
        Reg[0] = 0;
        Reg[1] = hand;
#ifdef SWILOG
        LOG("Process %s thread %u CreateResourceLimit (| %08x %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, Reg[0], Reg[1]);
#endif
        return;
    }
    case 0x79: //SetResourceLimitValues(s64* values, Handle resourceLimit, LimitableResource* names, s32 nameCount) 
    {
        u32 handleResourceLimit = Reg[0];
        u32 names_ptr = Reg[1];
        u32 values_ptr = Reg[2];
        u32 nameCount = Reg[3];

        if (names_ptr == NULL)
        {
            Reg[0] = SVCERROR_INVALID_POINTER;
#ifdef SWILOG
            LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }
        if (nameCount <= 0)
        {
            Reg[0] = SVCERROR_OUT_OF_RANGE;
#ifdef SWILOG
            LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }

        KResourceLimit* lim = (KResourceLimit*)*currentThread->m_owner->GetHandleTable()->GetHandle<KResourceLimit>(handleResourceLimit);
        if (lim == NULL)
        {
            Reg[0] = SVCERROR_INVALID_HANDLE;
#ifdef SWILOG
            LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
            return;
        }


        for (u32 i = 0; i < nameCount; i++)
        {
            u32 data;

            s32 ret = currentThread->m_owner->getMemoryMap()->Read32(names_ptr + i * 4, data); //read from user mode
            if (ret != Success)
            {
                Reg[0] = SVCERROR_INVALID_PARAMS;
#ifdef SWILOG
                LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }
            if (data >= 0xA)
            {
                Reg[0] = SVCERROR_OUT_OF_RANGE;
#ifdef SWILOG
                LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }
            u64 value;
            ret = currentThread->m_owner->getMemoryMap()->Read64(values_ptr + i * 8, value);
            if (ret != Success)
            {
                Reg[0] = SVCERROR_INVALID_PARAMS;
#ifdef SWILOG
                LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
                return;
            }
#ifdef SWILOG
            LOG("%08x -> %" PRIx64, data, value);
#endif
            lim->SetMaxValue(data, value);

        }

        Reg[0] = 0;
#ifdef SWILOG
        LOG("Process %s thread %u SetResourceLimitValues (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, values_ptr, handleResourceLimit, names_ptr, nameCount, Reg[0]);
#endif
        return;
    }
    case 0x7C: // KernelSetState(unsigned int Type, unsigned int Param0, unsigned int Param1, unsigned int Param2) 
    {
        u32 type = Reg[0];
        u32 param0 = Reg[1];
        u32 param1 = Reg[2];
        u32 param2 = Reg[3];
        if (type == 3 && param0 == 0) //map the firm laod param
        {

            MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

            chunk->size = 0x1000;
            chunk->data = currentThread->m_owner->m_Kernel->m_FIRM_Launch_Parameters;

            currentThread->m_owner->getMemoryMap()->AddPages(param1, 0x1000, chunk->data, chunk, PERMISSION_RW, MEMTYPE_MIRROR, NULL); //todo is type MEMTYPE_MIRROR realy correct don't think so
            Reg[0] = 0;
        }
        else
        {
            LOG("Process %s thread %u KernelSetState unknown (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, type, param0, param1, param2, Reg[0]);
            Reg[0] = -1;
        }
#ifdef SWILOG
        LOG("Process %s thread %u KernelSetState (%08x %08x %08x %08x | %08x)", currentThread->m_owner->GetName(), currentThread->m_thread_id, type, param0, param1, param2, Reg[0]);
#endif
        return;
    }
    default:
        XDSERROR("Process %s thread %u tryed to call syscall %02X but that syscall is not implemented", currentThread->m_owner->GetName(), currentThread->m_thread_id, swi);

        Reg[0] = 0xF8C007F4;//TODO the 3DS would terminate the Process if it is realy unknown but some are just stubs
		if (swi == 0x3C)
		{
			fflush(stdout);
			while (1);
		}
        break;
    }

}

u32 ControlMemory_swi(u32* address, u32 addr0, u32 addr1, u32 size, u32 op, u32 permissions, KThread * currentThread)
{
    if (addr0 & 0xFFF)
        return SVCERROR_ALIGN_ADDR;
    if (addr1 & 0xFFF)
        return SVCERROR_ALIGN_ADDR;
    if (size & 0xFFF)
        return SVCERROR_INVALID_SIZE;

    if (op & 0x10000) { // FFF680A4 //this was if(op == 0x10003) in Version < 5.0
        if (addr0 == 0) { // FFF680C4
            if (addr1 != 0)
                return SVCERROR_INVALID_PARAMS;
        }
        else if (size == 0) { // FFF680D0
            if (addr0 < 0x14000000)
                return SVCERROR_INVALID_PARAMS;
            if ((addr0 + size) >= 0x1C000000)
                return SVCERROR_INVALID_PARAMS;
            if (addr1 != 0)
                return SVCERROR_INVALID_PARAMS;
        }
        else {
            if (addr0 < 0x14000000)
                return SVCERROR_INVALID_PARAMS;
            if (addr0 >= 0x1C000000)
                return SVCERROR_INVALID_PARAMS;
            if (addr1 != 0)
                return SVCERROR_INVALID_PARAMS;
        }
    }
    else if (op == 1) {
        if (size == 0) { // FFF68110
            if (addr0 < 0x08000000) // FFF68130
                return SVCERROR_INVALID_PARAMS;
            if (addr0 >= 0x1C000000)
                return SVCERROR_INVALID_PARAMS;
        }
        else {
            if (addr0 < 0x08000000)
                return SVCERROR_INVALID_PARAMS;
            if ((addr0 + size) >= 0x1C000000)
                return SVCERROR_INVALID_PARAMS;
        }
    }
    else {
        if (size == 0) { // FFF68148
            if (addr0 < 0x08000000)
                return SVCERROR_INVALID_PARAMS;
            if (addr0 >= 0x14000000)
                return SVCERROR_INVALID_PARAMS;
        }
        else {
            if (addr0 < 0x08000000)
                return SVCERROR_INVALID_PARAMS;
            if ((addr0 + size) >= 0x14000000)
                return SVCERROR_INVALID_PARAMS;
        }

        if (op == 4 || op == 5) { // FFF680E8
            if (size == 0) {
                if (addr1 < 0x100000) // FFF681CC
                    return SVCERROR_INVALID_PARAMS;
                if (addr1 >= 0x14000000)
                    return SVCERROR_INVALID_PARAMS;
            }
            if (addr1 < 0x100000)
                return SVCERROR_INVALID_PARAMS;

            if ((addr1 + size) >= 0x14000000)
                return SVCERROR_INVALID_PARAMS;
        }
    }

    switch (op & 0xff) {
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
        break;
    default:
        return SVCERROR_INVALID_OPERATION;
    }


	if (((op & 0xFF) != 1) && (permissions & 0xFF) > 0x4) //NONE R-- W-- RW- dose not matter for free
    {
        return SVCERROR_INVALID_OPERATION; //Invalid combination 
    }
    op = op & 0xFFFFFF;
    if (currentThread->m_owner->GetProcessID() != 1) //loader is privileged to use all the mem regardless of the ResourceLimmit
    {
        if ((op & 0xFF) == 3)
        {
            bool ret = currentThread->m_owner->GetResourceLimit()->UseResource(RESOURCE_COMMIT, size);
            if (!ret)
                return SVCERROR_RESOURCE_LIMIT;
        }
        if (op & 0xF00)
        {
            op = (op & ~0xF00) | (currentThread->m_owner->m_exheader_flags & 0xF00); //force to be in the correct arear
        }
    }

    s32 ret = currentThread->m_owner->getMemoryMap()->ControlMemory(address, addr0, addr1, size, op, permissions);
    if (ret == Success && (op & 0xFF) == 1)
    {
        //free the Resource
        currentThread->m_owner->GetResourceLimit()->FreeResource(RESOURCE_COMMIT, size);
    }
    return ret;
}
