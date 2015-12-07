class KAutoObjectRef {
public:
	KAutoObjectRef(const KAutoObjectRef &obj);
    KAutoObjectRef();
    KAutoObjectRef(KAutoObject* object);
    ~KAutoObjectRef();
    void SetObject(KAutoObject* object);
    KAutoObject* operator*();
private:
    KAutoObject* m_object;
};
