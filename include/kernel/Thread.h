#pragma once

class KArmCore;

enum SchedulingTask : bit8 {

    TASK_PERFORM    = 1,
    TASK_IGNORE     = 2     // TASK_ABANDON?

};

class KThread : public KSynchronizationObject,public KTimeedEvent
{
public:

    typedef KSynchronizationObject super;


    KThread(s32 core,KProcess *owner);
    ~KThread();

    bool Synchronization(KThread* thread, u32 &error);
    void SyncFree(s32 errorCode, KSynchronizationObject* obj);
    void SyncStall(KLinkedList<KSynchronizationObject>* objects, bool waitAll);
	void stop();
	void trigger_event();
    bool m_waitAll;
    KLinkedList<KSynchronizationObject> *Threadwaitlist; //if there is nothing inside the thread is freed

    virtual bool IsInstanceOf(ClassName name);
    virtual void Destroy();


	bool m_running;

    bit8 m_scheduling_task;
    s32 m_thread_prio;
    s32 m_creator_core;
    s32 m_corenumb;
    KArmCore* m_core;
    u32 m_thread_id;

    KThread* m_prev;
    KThread* m_next;

    u32 m_TSL3DS;
    u8* m_TSLpointer;
    KProcess* m_owner;

    u32 arb_addr;

    ThreadContext m_context;

    static const ClassName name = KThread_Class;


private:

};


