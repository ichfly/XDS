#pragma once



class KDmaObject : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


	KDmaObject(u8 channel, u8 started, KProcess *owner);
	~KDmaObject();
    bool Synchronization(KThread* thread,u32 &error);
    virtual bool IsInstanceOf(ClassName name);

	static const ClassName name = KDmaObject_Class;

	u8 GetState() { return m_started; }

private:
    KProcess *m_owner;
    u8 m_channel;
	u8 m_started;
};


