

class KInterrupt : public KAutoObject
{
public:
    typedef KAutoObject super;

    KInterrupt(KSynchronizationObject* syncObject, s32 priority, bool isManualClear);
    KAutoObjectRef* GetObjRef();
    ~KInterrupt();
    void fire();


    bool IsInstanceOf(ClassName name);
    static const ClassName name = KInterrupt_Class;

private:
    KAutoObjectRef m_syncObject;
    s32 m_priority;
    bool m_isManualClear;
};