#pragma once



class KTimer : public KSynchronizationObject, public KTimeedEvent
{
public:

    typedef KSynchronizationObject super;


	KTimer(KProcess *owner, u32 resettype);
	~KTimer();
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);
	Result SetTimer(s64 initial, s64 interval);
    void Release();
	void Cancel();
	virtual void trigger_event();

    static const ClassName name = KTimer_Class;


private:
    KProcess *m_owner;
	bool m_Enabled;
	u32 m_ResetType;
	s64 m_Interval;
	s64 m_Initial;
	bool m_locked;
};


