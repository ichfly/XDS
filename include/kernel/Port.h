
class KPort : public KAutoObject
{
public:

    typedef KAutoObject super;


    KPort(char* name, u32 maxconnection);
    ~KPort();
    virtual bool IsInstanceOf(ClassName name);

    static const ClassName name = KPort_Class;

    KClientPort m_Client;
    KServerPort m_Server;
    char m_Name[9]; //this is not like on the 3DS but we use it like that for debugging 
private:
};


