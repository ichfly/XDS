class KPort;

class KSession;
class KClientSession : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KClientSession(KSession *owner);
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    static const ClassName name = KClientSession_Class;

	virtual void Destroy();
private:
    KSession *m_owner;
    u32 m_unk;
};
