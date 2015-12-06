#pragma once



class KEvent : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KEvent(u32 priority, bool manual, KProcess *owner);
    ~KEvent();
    bool Synchronization(KThread* thread, u32 &error);
	void Clear();
    virtual bool IsInstanceOf(ClassName name);

    void Triggerevent();

    static const ClassName name = KEvent_Class;

    bool m_open;
private:
    KProcess *m_owner;
    bool m_manual;
    u32 m_priority;
	PMutex m_Mutex;
};


