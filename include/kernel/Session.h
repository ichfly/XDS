
class KSession : public KAutoObject
{
public:

    typedef KAutoObject super;


    KSession(KPort * owner = NULL);
    ~KSession();
    virtual bool IsInstanceOf(ClassName name);

    s32 Communicate(KThread* sender, KThread* recver,bool IsResponse);

    static const ClassName name = KSession_Class;

    KServerSession m_Server;
    KClientSession m_Client;
    KPort * m_owner; //this is for debugging
    KLinkedList<u32> m_openMemAddr;
    KLinkedList<u32> m_openMemSize;
private:
};


