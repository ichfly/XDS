#include "Kernel.h"


KThread*     current_thread;        // 0xFFFF9000
KThread*     current_core_first_thread;  // *0xFFFF900C + 0xC
SwitchContext* sc;                  // 0xFFF2D0A8 (can change occasionally)

KScheduler::KScheduler()
{

}

KScheduler::~KScheduler()
{

}

void KScheduler::AddToScheduler(KThread* add_thread)    // add current_scheduler-> in front of all members?
{
    s16 core = m_core_num;

    if (m_schedule_entries[add_thread->m_thread_prio].next == NULL)
    {
        if (KScheduler::GetPrioType(add_thread->m_thread_prio) == PRIO_TYPE_HIGH)    // highest bit of prio to differentiate between high prio and low prio
        {
            m_thread_high_prio |= 0x80000000UL >> KScheduler::GetPrioField(add_thread->m_thread_prio); // shift the top bit down to the right bit in the field
        }
        else
        {
            m_thread_low_prio |= 0x80000000UL >> KScheduler::GetPrioField(add_thread->m_thread_prio);
        }
    }

    add_thread->m_prev = m_schedule_entries[add_thread->m_thread_prio].prev;    // prev entry becomes previous for thread
    add_thread->m_next = NULL;  // this thread is the new last
    
    if (m_schedule_entries[add_thread->m_thread_prio].prev == NULL) // if entry->prev is NULL, entry->next is the new thread
    {
        m_schedule_entries[add_thread->m_thread_prio].next = add_thread;
    }
    else
    {
        m_schedule_entries[add_thread->m_thread_prio].prev->m_next = add_thread; // otherwise, the next thread from entry->prev is the new thread
    }

    m_schedule_entries[add_thread->m_thread_prio].prev = add_thread; // then set entry->prev to the new thread

    m_scheduler_thread_count++; 

    if (core == m_core_num) // if the core being scheduled has not changed since the beginning of this function, swap context
    {
       if (current_thread->m_thread_prio > add_thread->m_thread_prio) // and if the prio of the current thread (0xFFFF9000) is greater than the thread being added, we need to do some scheduling because this thread needs to run
       {
           m_switch_context = true;
       }
    }

    if (core != m_core_num) // if the core being scheduled has changed, trigger interrupt 8 for the other core
    {
        m_swap_intr_core = true;
    }

}

void KScheduler::RemoveFromScheduler(KThread* sub_thread)   // add current_scheduler-> in front of all members?
{
    if (m_schedule_entries[sub_thread->m_thread_prio].next != NULL && m_schedule_entries[sub_thread->m_thread_prio].prev->m_thread_id == m_schedule_entries[sub_thread->m_thread_prio].next->m_thread_id)  // if entry->next and entry->prev are non-zero and the same, 
    {   
        if (KScheduler::GetPrioType(sub_thread->m_thread_prio) == PRIO_TYPE_HIGH)    // highest bit of prio to differentiate between high prio and low prio
        {
            bit32 r_mask = 0x80000000UL >> KScheduler::GetPrioField(sub_thread->m_thread_prio);   // shift the top bit down to the right bit in the field
            m_thread_high_prio &= ~r_mask;  // clear the bit in the mask
        }
        else
        {
            bit32 r_mask = 0x80000000UL >> KScheduler::GetPrioField(sub_thread->m_thread_prio);
            m_thread_low_prio &= ~r_mask;
        }
    }

    if (sub_thread->m_prev == NULL) // if the old thread to be removed is the first
    {
        m_schedule_entries[sub_thread->m_thread_prio].next = sub_thread->m_next;  // set entry-> next to the thread after the one being removed
    }
    else
    {
        sub_thread->m_prev->m_next = sub_thread->m_next;        // set it to skip over the old thread
    }

    if (sub_thread->m_next == NULL) // link things together without the old thread in there
    {
        sub_thread->m_next->m_prev = sub_thread->m_prev;
    }
    else
    {
        m_schedule_entries[sub_thread->m_thread_prio].prev = sub_thread->m_next;
    }

    m_scheduler_thread_count--;

    if (current_thread->m_thread_id == sub_thread->m_thread_id) // if the current thread is the one being removed, do another schedule and trigger intr 8 for the other core
    {
        m_switch_context = true;
        m_swap_intr_core = true;
    }
}

void KScheduler::AdjustScheduler(KThread* schedule_thread, bit8 scheduling_task)    // scheduling_task is schedule_thread->m_scheduling_task read just before calling this function
{
    if (scheduling_task == schedule_thread->m_scheduling_task)
    {
        return;
    }
    else if (scheduling_task == TASK_PERFORM)
    {
        KScheduler::RemoveFromScheduler(schedule_thread);
    }
    else if (schedule_thread->m_scheduling_task == TASK_PERFORM)
    {
        KScheduler::AddToScheduler(schedule_thread);
    }

}

KThread* KScheduler::ScheduleThreads(int z)
{
    int b;

    if (z == 0)
    {
        b = 1;
    }
    else
    {
        b = 0;
    }

    if (m_need_post_intr_reschedule)
    {
        if (!(current_core_first_thread->m_scheduling_task & 0xF))
        {
            KScheduler::Reschedule(current_core_first_thread, 1);
        }
    }

    if (m_unknown3)
    {
        // function here, but i don't think KScheduler+0x24 is ever set
    }

    if (z)  // CMP     R4, #0, again this is pretty much never set to anything besides 0 - every instance in the current kernel is 0
    {

    }

    KThread* load_thread = NULL;

    while (load_thread == NULL)
    {
        u32 zero_count = Common::CountLeadingZeros(m_thread_high_prio);

        if (zero_count == PRIO_FIELD_COUNT)  // did it count all the way through high prio thread bitfield without finding a 1?
        {
            zero_count = Common::CountLeadingZeros(m_thread_low_prio) + PRIO_FIELD_COUNT;  // if so, do the same with the low prio thread mask
        }

        if (zero_count > PRIO_MAX)
        {
            load_thread = m_scheduler_thread;
        }
        else
        {
            load_thread = m_schedule_entries[zero_count].next;
        }
    }


    return NULL;
}

void KScheduler::SwitchKernelContext(int z)
{
    if (!m_switch_context)
    {
        return;
    }

    m_switch_context = false;
    m_scheduler_count = 1;

    // enable intr

    /* TODO: thread context pages (0xFF4xxxxx)
    *
    *
    *
    *
    *
    */

    KThread* t_out = KScheduler::ScheduleThreads(z);

    while (1)
    {
        // disable intr
        if (t_out != NULL)
        {
            // load KThread context here
        }

        if (!m_switch_context)
        {
            m_scheduler_count = 0;
            return;
        }

        m_switch_context = false;

        // enable intr

        // TODO

        t_out = KScheduler::ScheduleThreads(0);
    }
}

void KScheduler::Reschedule(KThread* schedule_thread, bit8 lower_scheduling_mask)
{
    KScheduler::SwitchThreadContext(sc);

    bit8 t_mask = schedule_thread->m_scheduling_task;
    schedule_thread->m_scheduling_task = (t_mask & 0xF0) | (lower_scheduling_mask & 0xF);

    KScheduler::AdjustScheduler(schedule_thread, t_mask);

    KScheduler::ReturnThreadContext(sc);
}

void KScheduler::SwitchThreadContext(SwitchContext* c)
{
    if (current_thread->m_thread_id == c->current_thread_on_switch->m_thread_id)
    {
        c->running_thread_switch_count++; // count goes up when it tries to switch to the currently running thread
        return;
    }

    while (1) 
    {
        //while (c->current_thread_on_switch != NULL)
        {
            // do nothing, just wait
        }

        m_scheduler_count++;    // this is a ldrex/strex pair

        if (c->current_thread_on_switch == NULL)    // ldrex
        {
            c->current_thread_on_switch = current_thread;   // strex, but this doesn't always happen, so force a strex either way to complete the ldrex/strex pair by writing back current value
            // if this particular strex can't complete(not the one referenced above, this specific one), this is locked elsewhere and it's time to exit
            // break;
        }
        else
        {
            //c->current_thread_on_switch = c->current_thread_on_switch;  // the second strex, referenced above, to complete the pair IF c->current_thread_on_switch is NOT NULL
        }

        if (m_scheduler_count > 1)
        { 
            if (m_scheduler_count != 1)     // ldrex/strex
            {
                m_scheduler_count--;
                continue;
            }
            else
            {
                // clrex
            }
        }

        // save cpsr
        // disable intr

        m_scheduler_count--;    // ldrex/strex

        if (m_switch_context)
        {
            KScheduler::SwitchKernelContext(0);
        }

        // writeback cpsr
    }

    m_scheduler_count = 1;
}

void KScheduler::ReturnThreadContext(SwitchContext* c)
{
    if ((c->running_thread_switch_count -= 1) != 0)
    {
        return;
    }

    c->current_thread_on_switch = NULL; 

    if (m_scheduler_count > 1)
    {
        if (m_scheduler_count != 1)
        {
            return; 
        }

        // clrex - if this ends up equal to 1 the second time, another context switch has happened somewhere else and there may be another lock - clear it
    }

    if (m_swap_intr_core)
    {
        KScheduler::OtherCoreTriggerIntr8();
    }

    // save cpsr
    // disable intr

    m_scheduler_count--;    // ldrex/strex

    if (m_switch_context)
    {
        KScheduler::SwitchKernelContext(0);
    }

    // enable intr
}

void KScheduler::SoftContextSwitch(SwitchContext* c)
{
    if ((c->running_thread_switch_count -= 1) != 0)
    {
        return;
    }

    c->current_thread_on_switch = NULL;

    if (m_scheduler_count > 1)
    {
        if (m_scheduler_count != 1)     // ldrex/strex
        {
            m_scheduler_count--;
            return;
        }

        // clrex
    }

    // save cpsr
    // disable intr

    m_scheduler_count--;

    if (m_switch_context)
    {
        KScheduler::SwitchKernelContext(0);
    }

    // enable intr
}

void KScheduler::OtherCoreTriggerIntr8()
{

}

