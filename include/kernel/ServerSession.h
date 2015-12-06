class KSession;

class KServerSession : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KServerSession(KSession *owner);
    ~KServerSession();
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    static const ClassName name = KServerSession_Class;

    s32 reply(KThread * sender);
	void Destroy();
    KThread*  m_processingCmd;
    KThread*  m_waitingForCmdResp;

private:
    KSession *m_owner;
};
