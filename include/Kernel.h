#include "Common.h"
#include <unordered_map>

typedef s32 Result;
const Result Success = 0;

typedef u32 Handle;

enum ClassName {
    KAutoObject_Class,
    KSynchronizationObject_Class,
    KProcess_Class,
    KThread_Class,
    KTimer_Class,
    KMutex_Class,
    KSemaphore_Class,
    KAddressArbiter_Class,
    KClientPort_Class,
    KClientSession_Class,
    KServerPort_Class,
    KServerSession_Class,
    KMemoryBlock_Class,
    KResourceLimit_Class,
    KCodeSet_Class,
    KEvent_Class,
    KInterrupt_Class,
    KPort_Class,
    KSession_Class,
	KSharedMemory_Class,
	KDmaObject_Class
};


#include "Util.h"

class KProcess;

struct ThreadContext {
    u32 cpu_registers[13];
    u32 sp;
    u32 lr;
    u32 pc;
    u32 cpsr;
    u32 fpu_registers[32];
    u32 fpscr;
    u32 fpexc;
    // These are not part of native ThreadContext, but needed by emu
    u32 reg_15;
    u32 mode;

	KProcess* m_pro;

    u32 NFlag, ZFlag, CFlag, VFlag, IFFlags;
};

#include "kernel/TimedEvent.h"
#include "kernel/AutoObject.h"
#include "kernel/AutoObjectRef.h"
#include "kernel/LinkedList.h"
#include "kernel/SynchronizationObject.h"
#include "kernel/HandleTable.h"
#include "kernel/MemoryMap.h"
#include "kernel/CodeSet.h"
#include "kernel/ResourceLimit.h"
#include "kernel/Thread.h"
#include "arm/interpreter/arm_interpreter.h"
#include "arm/dyncom/arm_dyncom.h"
#include "kernel/Process.h"
#include "arm/ArmCore.h"
#include "kernel/Kernel.h"
#include "kernel/Scheduler.h"
#include "kernel/Memory.h"
#include "kernel/AddressArbiter.h"
#include "kernel/Swi.h"
#include "kernel/Event.h"
#include "kernel/Semaphore.h"
#include "kernel/Mutex.h"
#include "kernel/Interrupt.h"
#include "kernel/ClientSession.h"
#include "kernel/ServerSession.h"
#include "kernel/Session.h"
#include "kernel/ClientPort.h"
#include "kernel/ServerPort.h"
#include "kernel/Port.h"
#include "kernel/SharedMemory.h"
#include "kernel/DmaObject.h"
#include "kernel/Timer.h"


extern KThread*     current_thread;         // 0xFFFF9000
extern KProcess*    current_process;        // 0xFFFF9004
extern KScheduler*  current_scheduler;      // 0xFFFF9008
extern KThread*     current_core_first_thread;   // *0xFFFF900C + 0xC- first thread spawned on a given core, i think 0x2F0-0x2FC are arguments or something

extern SwitchContext* sc;           // 0xFFF2D0A8 (can change occasionally)
