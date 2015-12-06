class KPort;

class KClientPort : public KSynchronizationObject
{
public:

    typedef KSynchronizationObject super;


    KClientPort(char* name, u32 maxconnection, KPort *owner);
    bool Synchronization(KThread* thread, u32 &error);
    virtual bool IsInstanceOf(ClassName name);

    s32 connect(KClientSession* &sesion);

    static const ClassName name = KClientPort_Class;


private:
    KPort *m_owner;
    s16 m_maxConnection;
    s16 m_CurrentConnection;
};
