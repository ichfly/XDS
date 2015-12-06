
class KKernel;
#define bb_map std::unordered_map<u32, int>

class KProcess : public KSynchronizationObject
{
public:
    typedef KSynchronizationObject super;

    bool Synchronization(KThread* thread, u32 &error);

	KProcess::~KProcess();

    KProcess(KCodeSet* code, u32 capabilities_num, u32* capabilities_ptr, KKernel* Kernel,bool );

    KMemoryMap* getMemoryMap();

    void AddQuickCode(u8* buf, size_t size);

    void AddThread(KThread * thread);

    const char* GetName();
    u32 GetProcessID();
    KResourceLimit* GetResourceLimit();
    void SetResourceLimit(KResourceLimit* lim);
    KHandleTable* GetHandleTable();

    virtual Result WaitSynchronization(s64 timeout);
    virtual bool IsInstanceOf(ClassName name);
    virtual void Destroy();
    bool m_systemcallmask[0x80];
    u32  m_exheader_flags;
    KMemoryMap m_memory;

    u32 LINEAR_memory_virtual_address_userland;

    static const ClassName name = KProcess_Class;
    KKernel* m_Kernel;
    u32  m_ProcessID;

	//Dyncore stuff
	char* repretBuffer;
	int repretBuffersize;
	int repretBuffertop;
	bb_map *CreamCache;

private:
    u32 static Read32(uint8_t p[4]);
    void ParseArm11KernelCaps(u32 capabilities_num, u32* capabilities_ptr);
    KLinkedList<KThread> m_Threads;
    KAutoObjectRef m_limit;
    KAutoObjectRef m_codeset;
    KHandleTable* m_handles;
    bool m_AllowedInterrupt[0x7E];
    /*
    Bit(0) Allow debug
    Bit(1) Force debug
    Bit(2) Allow non-alphanum
    Bit(3) Shared page writing
    Bit(4) Privilege priority
    Bit(5) Allow main() args <-- is ignored anyway
    Bit(6) Shared device mem <-- TODO what dose that mean??
    Bit(7) Runnable on sleep
    Bit(8-11) Memory type(0 = APPLICATION 1 = SYSTEM 2 = BASE)
    Bit(12) Special memory (map to 0x14000000 insted of 0x00100000)
    Bit(23-31) 1111 1111 0
    */
};
