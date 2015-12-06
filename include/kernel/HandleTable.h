class KProcess;

struct HandleEntry {
    u32 handle; // 0 if free.

    union {
        HandleEntry* next_free;
        KAutoObject* object;
    } ptr;
};

class KHandleTable {
public:
    KHandleTable(KProcess* process, u32 size);

    template<class T> KAutoObjectRef GetHandle(Handle h) {
        KAutoObjectRef ref;
        s32 ret = GetHandleObject(ref, h);
        if (ret != Success)
        {
            return NULL;
        }

        KAutoObject* obj = *ref;
        if (obj == NULL) return NULL;
        if (!obj->IsInstanceOf(T::name)) return NULL;
        return ref;
    }

    Result CreateHandle(Handle& handle_out, KAutoObject* obj);
    Result GetHandleObject(KAutoObjectRef& obj_out, Handle handle);
    Result CloseHandle(Handle handle);
private:
    KProcess* m_process;
    HandleEntry* m_handles;
    HandleEntry* m_next_free;
    u32 m_counter;
    u32 m_size;
};
