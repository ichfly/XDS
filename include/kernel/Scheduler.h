#pragma once 


#define PRIO_FIELD_COUNT    0x20
#define PRIO_FIELD_MASK     0x1F
#define PRIO_TYPE_MASK      0x20
#define PRIO_COUNT          0x40
#define PRIO_MAX            PRIO_COUNT - 1
#define PRIO_HIGH           0
#define PRIO_LOW            PRIO_MAX
#define PRIO_TYPE_LOW       1
#define PRIO_TYPE_HIGH      0


struct SwitchContext {

    KThread* current_thread_on_switch;
    u32 running_thread_switch_count;
};

struct ThreadScheduleList {

    KThread* prev;      // this may be first/last, not prev/next
    KThread* next;

};

class KScheduler
{
public:
    KScheduler();
    ~KScheduler();

    void AddToScheduler(KThread* add_thread);
    void RemoveFromScheduler(KThread* sub_thread);
    void AdjustScheduler(KThread* schedule_thread, bit8 scheduling_mask);
    KThread* ScheduleThreads(int z);
    void Reschedule(KThread* schedule_thread, bit8 lower_scheduling_mask);

    void SwitchKernelContext(int z);
    void SwitchThreadContext(SwitchContext* c);     // switchA
    void ReturnThreadContext(SwitchContext* c);     // switchB
    void SoftContextSwitch(SwitchContext* c);       // switchC

    inline s32 GetPrioType(s32 prio)    { return (prio & PRIO_TYPE_MASK) >> 5; }
    inline s32 GetPrioField(s32 prio)   { return prio & PRIO_FIELD_MASK; }

    void OtherCoreTriggerIntr8();

private:

    u32 m_scheduler_count;              //used in the 3 context switch functions
    bool m_switch_context;
    bool m_intr_thread_in_progress;
    bool m_swap_intr_core;
    bool m_need_post_intr_reschedule;
    s16 m_core_num;
    u16 m_scheduler_thread_count;
    bit32 m_thread_high_prio;           // high priority thread mask (0x0-0x1F)
    bit32 m_thread_low_prio;            // low priority thread mask (0x20-0x3F)
    KThread* m_scheduler_thread;
    u32 m_unknown2;
    u32 m_unknown3;
    ThreadScheduleList m_schedule_entries[64];  // 1 per prio, 0-3F

};

/*
priority is handled from most significant bit to least significant bit:

ex.
m_thread_high_prio:    bit31, prio 0                bit 0, prio 31
                        |                              |
                        00000000000000000000000000000000

highest bit of priority determines between high and low priority:

ex. 
prio 0x11 vs prio 0x31: prio 0x11 is high prio bit 14, prio 0x31 is low prio bit 14
*/