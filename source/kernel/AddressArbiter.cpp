#include "Kernel.h"


KAddressArbiter::KAddressArbiter(KProcess* owner) : arbiterlist()
{
    m_owner = owner;
}

KAddressArbiter::~KAddressArbiter()
{

}
bool KAddressArbiter::IsInstanceOf(ClassName name) {
    if (name == KAddressArbiter::name)
        return true;

    return super::IsInstanceOf(name);
}
static KLinkedListNode<KThread>* threads_ArbitrateHighestPrioThread(KLinkedList<KThread> *list, u32 addr) //this also removes
{
    KLinkedListNode<KThread> *ret = NULL;
    s32 highest_prio = 0x80;
    KLinkedListNode<KThread>* node = list->list;
    while (node) {
        if (node->data->arb_addr == addr) {
            if (node->data->m_thread_prio <= highest_prio) {
                ret = node;
                highest_prio = node->data->m_thread_prio;
            }
        }
        node = node->next;
    }

    return ret;
}
Result KAddressArbiter::ArbitrateAddress(u32 addr,u32 type, s32 val, s64 time, KThread * caller)
{
    KLinkedListNode<KThread>* p;
    switch (type) {
        case 0: // Free
        // Negative value means resume all threads
            if (val < 0) {
                while (p = threads_ArbitrateHighestPrioThread(&arbiterlist, addr))
                {
                    m_owner->m_Kernel->StartThread(p->data);
                    arbiterlist.RemoveItem(p);
                }
        return 0;
        }

        // Resume first N threads
        for (int i = 0; i<val; i++) {
            if (p = threads_ArbitrateHighestPrioThread(&arbiterlist, addr))
            {
                m_owner->m_Kernel->StartThread(p->data);
                arbiterlist.RemoveItem(p);
            }
        else break;
        }

        return 0;

    case 1: // Acquire
        // If (signed cmp) value >= *addr, the thread is locked and added to wait-list.
        // Otherwise, thread keeps running and returns 0.
        u32 data;
        if (caller->m_owner->getMemoryMap()->Read32(addr, data) != Success)
        {
            return -1;
        }
        if (val > (s32)data)
        {
            m_owner->m_Kernel->ReScheduler();
            m_owner->m_Kernel->StopThread(caller);
            caller->arb_addr = addr;
            arbiterlist.AddItem(caller);
        }

        return 0;

        /*case 3: // Acquire Timeout
        if (value >= (s32)mem_Read32(addr))
        threads_SetCurrentThreadArbitrationSuspend(arbiter, addr);

        // XXX: Add timeout based on val2,3
        return 0;

        case 2: // Acquire Decrement
        val_addr = mem_Read32(addr) - 1;
        mem_Write32(addr, val_addr);

        if (value >= (s32)val_addr)
        threads_SetCurrentThreadArbitrationSuspend(arbiter, addr);

        return 0;

        case 4: // Acquire Decrement Timeout
        val_addr = mem_Read32(addr) - 1;
        mem_Write32(addr, val_addr);

        if (value >= (s32)val_addr)
        threads_SetCurrentThreadArbitrationSuspend(arbiter, addr);

        // XXX: Add timeout based on val2,3
        Reg[0] = 0;
        break;*/

    default:
        LOG("Invalid arbiter type %u\n", type);
        return 0xD8E093ED;
    }
}