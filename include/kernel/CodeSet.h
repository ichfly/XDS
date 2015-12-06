// File layout:
// - File header
// - Code, rodata and data relocation table headers
// - Code segment
// - Rodata segment
// - Loadable (non-BSS) part of the data segment
// - Code relocation table
// - Rodata relocation table
// - Data relocation table

// Memory layout before relocations are applied:
// [0..codeSegSize)             -> code segment
// [codeSegSize..rodataSegSize) -> rodata segment
// [rodataSegSize..dataSegSize) -> data segment

// Memory layout after relocations are applied: well, however the loader sets it up :)
// The entrypoint is always the start of the code segment.
// The BSS section must be cleared manually by the application.

// File header
#define _3DSX_MAGIC 0x58534433 // '3DSX'
typedef struct {
	u32 magic;
	u16 headerSize, relocHdrSize;
	u32 formatVer;
	u32 flags;

	// Sizes of the code, rodata and data segments +
	// size of the BSS section (uninitialized latter half of the data segment)
	u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;
} _3DSX_Header;

// Relocation header: all fields (even extra unknown fields) are guaranteed to be relocation counts.
typedef struct {
	u32 cAbsolute; // # of absolute relocations (that is, fix address to post-relocation memory layout)
	u32 cRelative; // # of cross-segment relative relocations (that is, 32bit signed offsets that need to be patched)
	// more?

	// Relocations are written in this order:
	// - Absolute relocs
	// - Relative relocs
} _3DSX_RelocHdr;

// Relocation entry: from the current pointer, skip X words and patch Y words
typedef struct {
	u16 skip, patch;
} _3DSX_Reloc;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define be_word(a)  __builtin_bswap32(a)
#define be_hword(a) __builtin_bswap16(a)
#define le_word(a)  (a)
#define le_hword(a) (a)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define be_word(a)  (a)
#define be_hword(a) (a)
#define le_word(a)  __builtin_bswap32(a)
#define le_hword(a) __builtin_bswap16(a)
#else
#error "What's the endianness of the platform you're targeting?"
#endif

struct _3DSX_LoadInfo {
	void* segPtrs[3]; // code, rodata & data
	u32 segAddrs[3];
	u32 segSizes[3];
};



class KCodeSet : public KAutoObject
{
public:
    typedef KAutoObject super;

    KCodeSet(u8* code_buf, u32 code_pages, u8* rodata_buf, u32 rodata_pages,
        u8* data_buf, u32 data_pages, u32 bss_pages, u64 TitleID, const char* name);
    ~KCodeSet();
    Result MapInto(KMemoryMap * map, bool spezialmem);
    const char* GetName();

    static const ClassName name = KCodeSet_Class;
    virtual bool IsInstanceOf(ClassName name);
private:
	bool Patch();
	void LoadElfFile(u8 *addr);
	int Load3DSXFile(FILE* f, u32 baseAddr);
    u8* m_code;
    u32 m_code_pages;
    u8* m_rodata;
    u32 m_rodata_pages;
    u8* m_data;
    u32 m_data_pages;
    u32 m_bss_pages;
    u64 m_TitleID;
    const char* m_name;
};
