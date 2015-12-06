class KPort;

class KServerPort : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KServerPort(char* name, u32 maxconnection, KPort *owner);
    ~KServerPort();
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    KServerSession * AcceptSesion();

    static const ClassName name = KServerPort_Class;

    KLinkedList<KSession> m_sessionToTake;
private:
    KPort *m_owner;
    KLinkedList<KSession> m_sessions;
};
