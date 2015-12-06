#pragma once



class KSemaphore : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KSemaphore(u32 count,u32 maxcount , KProcess *owner);
    ~KSemaphore();
    bool Synchronization(KThread* thread,u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    s32 ReleaseSemaphore(u32 releaseCount, u32 &count);

    static const ClassName name = KSemaphore_Class;


private:
    KProcess *m_owner;
    volatile u32 m_count;
    u32 m_maxcount;
    PMutex m_Mutex;
};


