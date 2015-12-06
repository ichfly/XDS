class IOHW;

/* MemoryPermissions */
typedef u8 MemoryPermissions;

#define PERMISSION_NONE 0
#define PERMISSION_R    1
#define PERMISSION_W    2
#define PERMISSION_RW   3
#define PERMISSION_X    4
#define PERMISSION_RX   5
#define PERMISSION_WX   6
#define PERMISSION_RWX  7

/* MemoryOperation */
typedef u32 MemoryOperation;

#define OPERATION_FREE    1
#define OPERATION_RESERVE 2
#define OPERATION_COMMIT  3
#define OPERATION_MAP     4
#define OPERATION_UNMAP   5
#define OPERATION_PROTECT 6
#define OPERATION_LINEAR  (1 << 16)

#define TLS_OFFSET 0x1FF82000

/* MemoryState */
typedef u16 MemoryState;

#define STATE_FREE       0
#define STATE_RESERVED   1
#define STATE_IO         2
#define STATE_STATIC     3
#define STATE_CODE       4
#define STATE_PRIVATE    5
#define STATE_SHARED     6
#define STATE_CONTINOUS  7
#define STATE_ALIASED    8
#define STATE_ALIAS      9
#define STATE_ALIASCODE  10
#define STATE_LOCKED     11

// Internal flags.
#define STATE_FREE_ALLOWED      0x100
#define STATE_PROTECT_ALLOWED   0x200
#define STATE_DEBUGSVCS_ALLOWED 0x400
#define STATE_IPC_ALLOWED       0x800
#define STATE_DMASVCS_ALLOWED   0x1000
#define STATE_UNKNOWN1          0x2000
#define STATE_UNKNOWN2          0x4000
#define STATE_UNKNOWN3          0x8000

//// Normal heap - 0xBB05
#define MEMTYPE_HEAP                                            \
    (STATE_PRIVATE|STATE_FREE_ALLOWED|STATE_PROTECT_ALLOWED|    \
     STATE_IPC_ALLOWED|STATE_DMASVCS_ALLOWED|STATE_UNKNOWN1|    \
     STATE_UNKNOWN3)

//// Linear heap - 0x3907
#define MEMTYPE_LINEARHEAP                                      \
    (STATE_CONTINOUS|STATE_FREE_ALLOWED|STATE_IPC_ALLOWED|      \
     STATE_DMASVCS_ALLOWED|STATE_UNKNOWN1)

//// Mirror mem - 0x1A09
#define MEMTYPE_MIRROR                                          \
    (STATE_ALIAS|STATE_PROTECT_ALLOWED|STATE_IPC_ALLOWED|       \
     STATE_DMASVCS_ALLOWED)

//// Mirrored mem - 0x3A08
#define MEMTYPE_MIRRORED                                        \
    (STATE_ALIASED|STATE_PROTECT_ALLOWED|STATE_IPC_ALLOWED|     \
     STATE_DMASVCS_ALLOWED|STATE_UNKNOWN1)

//// Code segment - 0xBC04
#define MEMTYPE_CODE                                            \
    (STATE_CODE|STATE_DEBUGSVCS_ALLOWED|STATE_IPC_ALLOWED|      \
     STATE_DMASVCS_ALLOWED|STATE_UNKNOWN1|STATE_UNKNOWN3)

// Tls:         has 0x380B
#define MEMTYPE_TLS                                            \
    (STATE_LOCKED|STATE_IPC_ALLOWED|                           \
     STATE_DMASVCS_ALLOWED|STATE_UNKNOWN1)
// Alias-code   has 0xBC0A (MapProcessMemory)
// SharedMemory has 0x5806
#define MEMTYPE_SHAREDMEMORY                                    \
    (STATE_SHARED|STATE_IPC_ALLOWED|                            \
     STATE_DMASVCS_ALLOWED|STATE_UNKNOWN3)
// Vram         has 0x1003
// IO mapping   has 0x1002
#define MEMTYPE_IO                                            \
    (STATE_IO|                                                \
     STATE_DMASVCS_ALLOWED)

/* PageFlags */
typedef u8 PageFlags;

struct MemoryInfo {
    u32 base_address;
    u32 size;
    u32 perm;
    u32 state;
};

struct PageInfo {
    u32 flags;
};

struct MemChunk {
    u32 size;
    u8* data;
    u32 ref_count;
};

struct MemPage {
    u8* data;
    MemChunk* chunk;
    MemoryState state;
    MemoryPermissions  perm;
    PageFlags flags;
    u32 mirrored; // If mirror-page, then this is original addr.
    IOHW *HW;
};

class KProcess;
class KMemoryBlock;

#define NUM_PAGES                                                       0x40000
#define PAGE_SIZE                                                        0x1000
#define PAGE_MASK                                                         0xFFF
#define HEAP_VA_START                                                0x08000000

class KMemoryMap {
public:
    KMemoryMap(KProcess* process);

    Result Read8 (u32 addr, u8&  out);
    Result Read16(u32 addr, u16& out);
    Result Read32(u32 addr, u32& out);
    Result Read64(u32 addr, u64& out);
	Result ReadN(u32 addr, u8* out, u32 size);
    Result Write8 (u32 addr, u8  val);
    Result Write16(u32 addr, u16 val);
    Result Write32(u32 addr, u32 val);
    Result Write64(u32 addr, u64 val);

    Result IPCMap(u32 addr0, u32 addr1, u32 size, MemoryPermissions perm, KMemoryMap * mapto);

    Result ControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size,
        MemoryOperation op, MemoryPermissions perm);
    Result QueryMemory(MemoryInfo* mem_out, PageInfo* page_out, u32 addr);
    Result MapMemoryBlock(KMemoryBlock& block, u32 addr,
        MemoryPermissions local_perms, MemoryPermissions remote_perms);
    Result UnmapMemoryBlock(KMemoryBlock& block, u32 addr);

    Result AddCodeSegment(u32 addr, u32 size, u8* data,
        MemoryPermissions perm);
    Result AddIOMem(u32 address, u32 size, MemoryPermissions perm);

    Result AddPages(u32 addr, u32 size, u8* data, MemChunk* chunk,
        MemoryPermissions perm, MemoryState state, IOHW *HW);

    Result AddTLS(u32* out_3DSAddr,u8** out_TLSpointer);
    Result RemoveTLS(u32 DSAddr);
    Result MapIOobj(u32 address, u32 size, IOHW* obj, MemoryPermissions perm);
    Result RemovePages(u32 addr, u32 size);
	s32 AllocFreeGSP(bool new3DS, u32 size);
	Result MapIOData(u32 address, u32 size,u8*data, MemoryPermissions perm);
    KProcess* m_process;
#ifndef XDS_TEST
private:
#endif
    Result VerifyRegionState(u32 addr, u32 size, MemoryState state);
    Result VerifyRegionMaskState(u32 addr, u32 size, MemoryState state);
    Result Reprotect(u32 addr, u32 size, MemoryPermissions perm);
    Result CreateChunk(MemChunk** chunk_out, u32 size);

    Result AddMirror(u32 mirror, u32 mirrored, u32 size,
        MemoryPermissions perm);
    Result AddMirror(u32 mirror, u32 mirrored, u32 size,
        MemoryPermissions perm, KMemoryMap * mapto);
    Result RemoveMirror(u32 mirror, u32 mirrored, u32 size);

    MemPage m_pages[NUM_PAGES];
    bool m_TLSused[0x100]; //the maximum number of TLS that are possible because of the ResourceLimit
    u8* m_TLSpointer[0x100/8];
};

// TODO: Free chunks when ref_count == 0.
