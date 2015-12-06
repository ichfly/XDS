

class KAddressArbiter : public KAutoObject
{
public:
    typedef KAutoObject super;

    KAddressArbiter(KProcess* owner);
    ~KAddressArbiter();

    Result ArbitrateAddress(u32 addr, u32 type, s32 val, s64 time, KThread * caller);



    bool IsInstanceOf(ClassName name);
    static const ClassName name = KAddressArbiter_Class;

private:
    KProcess* m_owner;
    KLinkedList<KThread> arbiterlist;
    PMutex m_Mutex;
};