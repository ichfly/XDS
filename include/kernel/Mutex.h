#pragma once



class KMutex : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KMutex(KProcess *owner,bool locked);
    ~KMutex();
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    void Release();

    static const ClassName name = KEvent_Class;


private:
    KProcess *m_owner;
    KThread *m_lockedThread;
    bool m_locked;
    PMutex m_Mutex;
};


