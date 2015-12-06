
class KThread;

class KSynchronizationObject : public KAutoObject
{
public:
    typedef KAutoObject super;

    KSynchronizationObject();

    virtual bool Synchronization(KThread* thread,u32 &error) = 0;

    virtual bool IsInstanceOf(ClassName name);
    bool Syn(KThread* thread, u32 &error);
    void SynRemove(KThread* thread);

    static const ClassName name = KAutoObject_Class;
    void SynFree(u32 errorCode, KThread* thread);
    KThread* SynGetNextPrio();
    void SynFreeAll(u32 errorCode);

	bool m_killed;

private:
    KLinkedList<KThread> waiting;

};
