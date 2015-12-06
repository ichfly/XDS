class KAutoObject {
public:
	KAutoObject();
    void AcquireReference();
    void ReleaseReference();

    virtual bool IsInstanceOf(ClassName name);
    virtual void Destroy();

    static const ClassName name = KAutoObject_Class;

private:
    u32 m_refcount;
};
