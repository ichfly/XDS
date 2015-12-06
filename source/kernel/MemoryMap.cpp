#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"

#define LOGHIGHACCESS

//#define LOGMODULE "codec"
//#define RWLOG
//#define WLOG

#define TtoTT(x)   case x: \
case x + 0x1000: \
case x + 0x2000: \
case x + 0x3000: \
case x + 0x4000: \
case x + 0x5000: \
case x + 0x6000: \
case x + 0x7000: \
case x + 0x8000: \
case x + 0x9000: \
case x + 0xA000: \
case x + 0xB000: \
case x + 0xC000: \
case x + 0xD000: \
case x + 0xE000: \
case x + 0xF000: \

#define TtoTTT(x)   TtoTT(x) \
TtoTT(x + 0x10000) \
TtoTT(x + 0x20000) \
TtoTT(x + 0x30000) \
TtoTT(x + 0x40000) \
TtoTT(x + 0x50000) \
TtoTT(x + 0x60000) \
TtoTT(x + 0x70000) \
TtoTT(x + 0x80000) \
TtoTT(x + 0x90000) \
TtoTT(x + 0xA0000) \
TtoTT(x + 0xB0000) \
TtoTT(x + 0xC0000) \
TtoTT(x + 0xD0000) \
TtoTT(x + 0xE0000) \
TtoTT(x + 0xF0000) \

KMemoryMap::KMemoryMap(KProcess* process) {
    m_process = process;
    memset(m_pages, 0, sizeof(m_pages));
    memset(m_TLSused, 0, sizeof(m_TLSused));
    memset(m_TLSpointer, 0, sizeof(m_TLSpointer));
}

Result KMemoryMap::ReadN(u32 addr, u8* out, u32 size) {

	for (u32 i = 0; i < size; i++)
	{
		s32 res = Read8(addr + i, out[i]);
		if (res != Success)
			return res;
	}

	return Success;
}

Result KMemoryMap::Read8(u32 addr, u8& out) {

    u32 offset = addr & PAGE_MASK;
    u32 page = addr / PAGE_SIZE;

    if (unlikely(page > NUM_PAGES))
        return -1;
    if (unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if (!(notcritical(m_pages[page].perm) & PERMISSION_R))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			out = m_pages[page].HW->Read8(addr);
			return Success;
		}
    }
	out = m_pages[page].data[offset];

#ifdef RWLOG
    if (strcmp(LOGMODULE, m_process->GetName()) == 0)
        LOG("read8 %08x %02x", addr, out);
#endif

#ifdef LOGHIGHACCESS

	if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("read8 %08x %02x", addr, out);
#endif

    return Success;
}

Result KMemoryMap::Read16(u32 addr, u16& out) {
    u32 page = addr/PAGE_SIZE;

    u32 offset = addr & PAGE_MASK;

    // Cross-page reading.
    if (unlikely(offset == PAGE_MASK && !((u8)(m_pages[page].state) == STATE_IO))) {
        u8 lo, hi;

        if (unlikely(Read8(addr, lo) != Success))
            return -1;
        if (unlikely(Read8(addr + 1, hi) != Success))
            return -1;

        // XXX: Verify:
        out = (lo<<8) | hi;

#ifdef LOGHIGHACCESS
		if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
            LOG("read16 %08x %04x", addr, out);
#endif

#ifdef RWLOG
        if (strcmp(LOGMODULE, m_process->GetName()) == 0)
            LOG("read16 %08x %04x", addr, out);
#endif

        return Success;
    }

    if(unlikely(page > NUM_PAGES))
        return -1;
    if(unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if(!(notcritical(m_pages[page].perm) & PERMISSION_R))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			out = m_pages[page].HW->Read16(addr);
			return Success;
		}
    }
    out = *(u16*) &m_pages[page].data[offset];

#ifdef RWLOG
    if (strcmp(LOGMODULE, m_process->GetName()) == 0)
        LOG("read16 %08x %04x", addr, out);
#endif

#ifdef LOGHIGHACCESS
	if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("read16 %08x %04x", addr, out)
#endif 

    return Success;
}

Result KMemoryMap::Read32(u32 addr, u32& out) {


    u32 page = addr/PAGE_SIZE;
    u32 offset = addr & PAGE_MASK;

    // Cross-page reading.
    if (unlikely(offset > (PAGE_MASK - 3) && !!((u8)(m_pages[page].state) == STATE_IO))) {
        // XXX: Verify: TODO speedup
        u8 B0, B1, B2, B3;

        if (unlikely(Read8(addr, B0) != Success))
            return -1;
        if (unlikely(Read8(addr + 1, B1) != Success))
            return -1;
        if (unlikely(Read8(addr + 2, B2) != Success))
            return -1;
        if (unlikely(Read8(addr + 3, B3) != Success))
            return -1;

        out = (B0 << 24) | (B1 << 16) | (B2 << 8) | (B3 << 0);
#ifdef LOGHIGHACCESS
		if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
            LOG("read32 %08x %08x", addr, out)
#endif 
#ifdef RWLOG
            if (strcmp(LOGMODULE, m_process->GetName()) == 0)
                LOG("read32 %08x %08x", addr, out)
#endif

        return Success;
    }

    if(unlikely(page > NUM_PAGES))
        return -1;
    if(unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if(!(notcritical(m_pages[page].perm) & PERMISSION_R))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			out = m_pages[page].HW->Read32(addr);
			return Success;
		}
    }

    out = *(u32*)&m_pages[page].data[offset];

#ifdef RWLOG
    if (strcmp(LOGMODULE, m_process->GetName()) == 0)
        LOG("read32 %08x %08x", addr, out)
#endif

#ifdef LOGHIGHACCESS
		if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("read32 %08x %08x", addr, out)
#endif 

    return Success;
}
Result KMemoryMap::Read64(u32 addr, u64& out) {
    u32 V1,V2;
    if (unlikely(Read32(addr, V1) != Success))
        return -1;
    if (unlikely(Read32(addr + 4, V2) != Success))
        return -1;
    out = ((u64)(V2) << 32) | (u64)V1;
    return Success;
}

Result KMemoryMap::Write8(u32 addr, u8 val) {

#ifdef LOGHIGHACCESS
	if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("write8 %08x %02x", addr, val)
#endif 
#if defined(RWLOG) || defined(WLOG)
        if (strcmp(LOGMODULE, m_process->GetName()) == 0)
            LOG("write8 %08x %02x", addr, val)
#endif

    u32 page = addr/PAGE_SIZE;
    u32 offset = addr & PAGE_MASK;

    if(unlikely(page > NUM_PAGES))
        return -1;
    if(unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if(!(notcritical(m_pages[page].perm) & PERMISSION_W))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			m_pages[page].HW->Write8(addr, val);
			return Success;
		}
    }

    m_pages[page].data[addr & PAGE_MASK] = val;
    return Success;
}

Result KMemoryMap::Write16(u32 addr, u16 val) {

#ifdef LOGHIGHACCESS
	if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("write16 %08x %04x", addr, val)
#endif 

#if defined(RWLOG) || defined(WLOG)
        if (strcmp(LOGMODULE, m_process->GetName()) == 0)
            LOG("write16 %08x %04x", addr, val)
#endif
    u32 page = addr/PAGE_SIZE;

    u32 offset = addr & PAGE_MASK;

    if (unlikely(offset == PAGE_MASK && !((u8)(m_pages[page].state) == STATE_IO))) {
        // XXX: Verify:
        u8 B0 = (u8)(val >> 8), B1 = (u8)val;

        if (unlikely(Write8(addr, B0) != Success))
            return -1;
        if (unlikely(Write8(addr + 1, B1) != Success))
            return -1;

        return Success;
    }

    if(unlikely(page > NUM_PAGES))
        return -1;
    if(unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if(!(notcritical(m_pages[page].perm) & PERMISSION_W))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			m_pages[page].HW->Write16(addr, val);
			return Success;
		}
    }

    *(u16*) &m_pages[page].data[offset] = val;
    return Success;
}

Result KMemoryMap::Write32(u32 addr, u32 val) {

#ifdef LOGHIGHACCESS
	if (addr >= 0x1FF80000 && addr <= 0x1FF81FFF)
        LOG("write32 %08x %08x", addr, val)
#endif 
#if defined(RWLOG) || defined(WLOG)
        if (strcmp(LOGMODULE, m_process->GetName()) == 0)
            LOG("write32 %08x %08x", addr, val)
#endif
    u32 page = addr/PAGE_SIZE;

    u32 offset = addr & PAGE_MASK;

    if(unlikely(offset > (PAGE_MASK-3))) {
        // XXX: Verify: TODO speedup
        u8 B0 = (u8)(val >> 24), B1 = (u8)(val >> 16), B2 = (u8)(val >> 8), B3 = (u8)val;

        if (unlikely(Write8(addr, B0) != Success && !((u8)(m_pages[page].state) == STATE_IO)))
            return -1;
        if (unlikely(Write8(addr + 1, B1) != Success))
            return -1;
        if (unlikely(Write8(addr + 2, B2) != Success))
            return -1;
        if (unlikely(Write8(addr + 3, B3) != Success))
            return -1;

        return Success;
    }

    if(unlikely(page > NUM_PAGES))
        return -1;
    if(unlikely(m_pages[page].state == STATE_FREE))
        return -1;
    if(!(notcritical(m_pages[page].perm) & PERMISSION_W))
        return -1;
    if (unlikely((u8)(m_pages[page].state) == STATE_IO))
    {
		if (m_pages[page].HW)
		{
			m_pages[page].HW->Write32(addr, val);
			return Success;
		}
    }
    *(u32*) &m_pages[page].data[offset] = val;
    return Success;
}

Result KMemoryMap::Write64(u32 addr, u64 val) {
    if (unlikely(Write32(addr, (u32)(val >> 0)) != Success))
        return -1;
    if (unlikely(Write32(addr + 4, (u32)(val >> 32)) != Success))
        return -1;
    return Success;
}

Result KMemoryMap::IPCMap(u32 addr0, u32 addr1, u32 size, MemoryPermissions perm, KMemoryMap * mapto)
{
    // Verify that addr1 is normal heap.
    if (VerifyRegionMaskState(addr1, size, STATE_IPC_ALLOWED) != 0)
        return 0xE0A01BF5;
    // Verify that addr0 is unmapped.
    if (mapto->VerifyRegionState(addr0, size, STATE_FREE) != 0)
        return -1; //this means we failed try again
    if (AddMirror(addr0, addr1, size, perm, mapto) != 0)
        return 0xE0A01BF5;
    return Success;
}

Result KMemoryMap::ControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size,
    MemoryOperation op, MemoryPermissions perm)
{
    perm = perm & PERMISSION_RW;

    switch(op & 0xFF) {
    case OPERATION_FREE:
        // Verify that addr0 is mapped heap/linear-heap.
        if(VerifyRegionMaskState(addr0, size, STATE_FREE_ALLOWED) != 0)
            return 0xE0A01BF5;
        if(RemovePages(addr0, size) != 0)
            return 0xE0A01BF5;

        *addr_out = addr0;
        break;

    case OPERATION_COMMIT:
        if(op & OPERATION_LINEAR) {
			u32 addr =AllocFreeGSP(false, size);
			addr0 = m_process->LINEAR_memory_virtual_address_userland + addr;
			// Verify that addr0 is unmapped.
			if (VerifyRegionState(addr0, size, STATE_FREE) != 0)
				return 0xE0A01BF5;
			// Create chunk.

			MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

			if (chunk == NULL) {
				return 0xE0A01BF5;
			}

			chunk->size = size;
			chunk->data = &Mem_FCRAM[addr];
			// Map it.
			if (AddPages(addr0, size, chunk->data, chunk, perm, MEMTYPE_HEAP, NULL) != 0) {
				free(chunk);
				return 0xE0A01BF5;
			}

			*addr_out = addr0;
            return 0;
        }

        MemChunk* chunk;

        // Verify that addr0 is unmapped.
        if(VerifyRegionState(addr0, size, STATE_FREE) != 0)
            return 0xE0A01BF5;
        // Create chunk.
        if(CreateChunk(&chunk, size) != 0)
            return 0xE0A01BF5;
        // Map it.
        if (AddPages(addr0, size, chunk->data, chunk, perm, MEMTYPE_HEAP, NULL) != 0) {
            free(chunk);
            return 0xE0A01BF5;
        }

        *addr_out = addr0;
        break;

    case OPERATION_MAP:
        // XXX: Perm @ addr1 bit0-1 must not be both 0.

        // Verify that addr1 is normal heap.
        if(VerifyRegionState(addr1, size, MEMTYPE_HEAP) != 0)
            return 0xE0A01BF5;
        // Verify that addr0 is unmapped.
        if(VerifyRegionState(addr0, size, STATE_FREE) != 0)
            return 0xE0A01BF5;
        if(AddMirror(addr0, addr1, size, perm) != 0)
            return 0xE0A01BF5;

        *addr_out = 0;
        break;

    case OPERATION_UNMAP:
        if(RemoveMirror(addr0, addr1, size) != 0)
            return 0xE0A01BF5;

        *addr_out = 0;
        break;

    case OPERATION_PROTECT:
        if(VerifyRegionMaskState(addr0, size, STATE_PROTECT_ALLOWED) != 0)
            return 0xE0A01BF5;

        if(Reprotect(addr0, size, perm) != 0)
            return 0xE0A01BF5;

        *addr_out = addr0; // XXX: This should be base_addr of MemQuery(addr1)
        break;
    }

    return Success;
}

Result KMemoryMap::VerifyRegionState(u32 addr, u32 size, MemoryState state) {
    MemoryInfo info;
    Result ret;

    ret = QueryMemory(&info, NULL, addr);

    if(ret != 0)
        return ret;
    if(info.state != state)
        return -1;
    if((addr + size) > (info.base_address + info.size))
        return -1;

    return Success;
}

Result KMemoryMap::VerifyRegionMaskState(u32 addr, u32 size, MemoryState state)
{
    MemoryInfo info;
    Result ret;

    ret = QueryMemory(&info, NULL, addr);

    if(ret != 0)
        return ret;
    if((info.state & state) != state)
        return -1;
    if((addr + size) > (info.base_address + info.size))
        return -1;

    return Success;
}

Result KMemoryMap::QueryMemory(MemoryInfo* mem_out, PageInfo* page_out,
    u32 addr)
{
    addr /= PAGE_SIZE;

    // Make sure we're within userspace memory.
    if(addr >= NUM_PAGES)
        return -1;

    MemPage* page = &m_pages[addr];
    MemPage* prev = page;
    u32 size = 1;

    while(1) {
        if((addr+size) >= NUM_PAGES)
            break;

        if((prev->state == page->state) && (prev->perm  == page->perm)) {
            prev = page;
            page++;
            size++;
        }
        else break;
    }

    mem_out->base_address = addr * PAGE_SIZE;
    mem_out->size = size * PAGE_SIZE;
    mem_out->perm = prev->perm;
    mem_out->state = prev->state;

    if(page_out != NULL)
        page_out->flags = 0;

    return 0;
}

Result KMemoryMap::CreateChunk(MemChunk** chunk_out, u32 size) {
    MemChunk* chunk = (MemChunk*) malloc(sizeof(MemChunk));
    u8* data = (u8*) calloc(size,sizeof(u8));

    if((chunk == NULL) || (data == NULL)) {
        free(chunk);
        free(data);
        return -1;
    }

    chunk->size = size;
    chunk->data = data;

    *chunk_out = chunk;
    return Success;
}

Result KMemoryMap::AddPages(u32 addr, u32 size, u8* data, MemChunk* chunk,
    MemoryPermissions perm, MemoryState state, IOHW *HW)
{
    addr /= PAGE_SIZE;
    size /= PAGE_SIZE;

    // Fill in page-info.
    for(u32 i=0; i<size; i++) {
        m_pages[addr+i].data = data + (i * PAGE_SIZE);
        m_pages[addr+i].chunk = chunk;
        m_pages[addr+i].state = state;
        m_pages[addr+i].perm = perm;
        m_pages[addr+i].mirrored = 0;
        m_pages[addr+i].HW = HW;
    }

    chunk->ref_count += size;
    return Success;
}

Result KMemoryMap::RemovePages(u32 addr, u32 size)
{
    addr /= PAGE_SIZE;
    size /= PAGE_SIZE;

    // Fill in page-info.
    for(u32 i=0; i<size; i++) {
        m_pages[addr+i].chunk->ref_count--;

        m_pages[addr+i].data = NULL;
        m_pages[addr+i].chunk = NULL;
        m_pages[addr+i].state = STATE_FREE;
        m_pages[addr+i].perm = PERMISSION_NONE;
        m_pages[addr+i].mirrored = 0;
    }

    return Success;
}

Result KMemoryMap::AddMirror(u32 mirror, u32 mirrored, u32 size,
    MemoryPermissions perm)
{
    mirror /= PAGE_SIZE;
    mirrored /= PAGE_SIZE;
    size /= PAGE_SIZE;

    for(u32 i=0; i<size; i++) {
        // Fill in mirror pages.
        m_pages[mirror+i].data  = m_pages[mirrored+i].data;
        m_pages[mirror+i].chunk = m_pages[mirrored+i].chunk;
        m_pages[mirror+i].state = MEMTYPE_MIRROR;
        m_pages[mirror+i].perm = perm;
        m_pages[mirror+i].mirrored = (mirrored + i) * PAGE_SIZE;
        m_pages[mirror+i].chunk->ref_count++;

        // Mark mirrored pages as mirrored.
        m_pages[mirrored+i].state = MEMTYPE_MIRRORED;
    }

    return Success;
}
Result KMemoryMap::AddMirror(u32 mirror, u32 mirrored, u32 size,
    MemoryPermissions perm, KMemoryMap * mapto)
{
    mirror /= PAGE_SIZE;
    mirrored /= PAGE_SIZE;
    size += PAGE_SIZE - 1;
    size /= PAGE_SIZE;

    for (u32 i = 0; i<size; i++) {
        // Fill in mirror pages.
		if (m_pages[mirrored + i].state != STATE_FREE)
		{
			mapto->m_pages[mirror + i].data = m_pages[mirrored + i].data;
			mapto->m_pages[mirror + i].chunk = m_pages[mirrored + i].chunk;
			mapto->m_pages[mirror + i].state = MEMTYPE_MIRROR;
			mapto->m_pages[mirror + i].perm = perm;
			mapto->m_pages[mirror + i].mirrored = (mirrored + i) * PAGE_SIZE;
			mapto->m_pages[mirror + i].chunk->ref_count++;

			// Mark mirrored pages as mirrored.
			mapto->m_pages[mirrored + i].state = MEMTYPE_MIRRORED;
		}
    }

    return Success;
}

Result KMemoryMap::RemoveMirror(u32 mirror, u32 mirrored, u32 size) {
    mirror /= PAGE_SIZE;
    mirrored /= PAGE_SIZE;
    size /= PAGE_SIZE;

    // Make sure we're within userspace memory.
    if((mirror + size) >= NUM_PAGES)
        return -1;
    if((mirrored + size) >= NUM_PAGES)
        return -1;

    // Check that the region is continously mapped.
    for(u32 i=0; i<size; i++) {
        if(m_pages[mirror+i].state != MEMTYPE_MIRROR)
            return -1;
        if(m_pages[mirrored+i].state != MEMTYPE_MIRRORED)
            return -1;
        if(m_pages[mirror+i].mirrored != (mirrored + i) * PAGE_SIZE)
            return -1;
    }
    
    for(u32 i=0; i<size; i++) {
        // Clear mirror pages.
        m_pages[mirror+i].chunk->ref_count--;
        memset(&m_pages[mirror+i], 0, sizeof(MemPage));

        // Restore state on mirrored pages.
        m_pages[mirrored+i].state = MEMTYPE_HEAP;
    }

    return Success;
}

Result KMemoryMap::Reprotect(u32 addr, u32 size, MemoryPermissions perm) {

	addr /= PAGE_SIZE;
	size /= PAGE_SIZE;

    for(u32 i=0; i<size; i++)
        m_pages[addr+i].perm = perm;

    return Success;
}

Result KMemoryMap::AddCodeSegment(u32 addr, u32 size, u8* data,
    MemoryPermissions perm)
{
    // Temporary implementation.
    MemChunk* chunk;
    if(CreateChunk(&chunk, size) != 0)
        return -1;

    memcpy(chunk->data, data, size);

    if (AddPages(addr, size, chunk->data, chunk, perm, MEMTYPE_CODE, NULL) != 0)
        return -1;

    return 0;
}
Result KMemoryMap::MapIOData(u32 address, u32 size, u8*data, MemoryPermissions perm) {
    // Temporary implementation.
    MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

    if (chunk == NULL) {
        return -1;
    }

    chunk->size = size;
	chunk->data = data;

	if (AddPages(address, size, chunk->data, chunk, perm, MEMTYPE_IO, NULL) != 0)
        return -1;

    return 0;
}
Result KMemoryMap::MapIOobj(u32 addr, u32 size, IOHW* obj, MemoryPermissions perm)
{
	// Temporary implementation.
	MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

	if (chunk == NULL) {
		return -1;
	}

	chunk->size = size;
	chunk->data = NULL;

	if (AddPages(addr, size, chunk->data, chunk, perm, MEMTYPE_IO, obj) != 0)
		return -1;

	return 0;
}
Result KMemoryMap::AddIOMem(u32 address, u32 size, MemoryPermissions perm)
{
    size &= ~0xFFF;
    address &= ~0xFFF;
    s32 ret = Success;
    for (u32 i = address; i < address + size; i+= 0x1000)
    {
        switch (i)
        {
		case 0x1ec01000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_hash1, perm) != Success)
				ret = -1;
			break;
		case 0x1ec40000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_PDN, perm) != Success)
				ret = -1;
			break;
		case 0x1ec42000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_SPI0, perm) != Success)
				ret = -1;
			break;
		case 0x1ec43000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_SPI1, perm) != Success)
				ret = -1;
			break;
		case 0x1ec44000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_I2C2, perm) != Success)
				ret = -1;
			break;
		case 0x1ec46000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_HID, perm) != Success)
				ret = -1;
			break;
		case 0x1ec47000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_GPIO, perm) != Success)
				ret = -1;
			break;
		case 0x1ec48000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_I2C3, perm) != Success)
				ret = -1;
			break;
		case 0x1ec60000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_SPI2, perm) != Success)
				ret = -1;
			break;
		case 0x1ec61000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_I2C1, perm) != Success)
				ret = -1;
			break;
		case 0x1ec62000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_MIC, perm) != Success)
				ret = -1;
			break;
		case 0x1ec63000:
            if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_p9,perm) != Success)
                ret = -1;
            break;
		case 0x1ed03000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_DSP, perm) != Success)
				ret = -1;
			break;
		case 0x1ee01000:
			if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_hash2, perm) != Success)
				ret = -1;
			break;
		TtoTT(0x1ef00000)
		TtoTT(0x1ef10000)
				if (MapIOobj(i, 0x1000, m_process->m_Kernel->m_GPU, perm) != Success)
					ret = -1;
			break;
		TtoTTT(0x1f000000)
		TtoTTT(0x1f100000)
		TtoTTT(0x1f200000)
		TtoTTT(0x1f300000)
		TtoTTT(0x1f400000)
		TtoTTT(0x1f500000)
		if (MapIOData(i, 0x1000, &Mem_VRAM[i - 0x1f000000], perm) != Success)
			ret = -1;
		break;

		TtoTT(0x1ff00000)
		TtoTT(0x1ff10000)
		TtoTT(0x1ff20000)
		TtoTT(0x1ff30000)
		TtoTT(0x1ff40000)
		TtoTT(0x1ff50000)
		TtoTT(0x1ff60000)
		TtoTT(0x1ff70000)
		if (MapIOData(i, 0x1000, &Mem_DSP[i - 0x1ff00000], perm) != Success)
				ret = -1;
			break;

        default:
            ret = -1;
            LOG("IO mem mapping of %08x is not yet supported",i);
            break;
        }
    }
    return ret;
}
Result KMemoryMap::AddTLS(u32* out_3DSAddr, u8** out_TLSpointer)
{
    //find a free slot
    int i = 0; //can never hit the end because of ResourceLimit
    for (; i < sizeof(m_TLSused) / sizeof(bool); i++)
    {
        if (!m_TLSused[i])
            break;
    }
    m_TLSused[i] = true;

    //allocate if not already done
    if (m_TLSpointer[i / 8] == NULL)
    {
		m_TLSpointer[i / 8] = (u8*)calloc(0x1000, sizeof(u8));
        if (m_TLSpointer[i / 8] == NULL)
        {
            return -1;
        }
        auto chunk = new MemChunk();
        chunk->data = m_TLSpointer[i / 8];
        chunk->size = 0x1000;
        AddPages(i * 0x200 + TLS_OFFSET, chunk->size, chunk->data, chunk, PERMISSION_RW, MEMTYPE_TLS, NULL);
    }

    *out_TLSpointer = m_TLSpointer[i / 8] + (i % 0x8) * 0x200;
    *out_3DSAddr = i * 0x200 + TLS_OFFSET;

    return Success;
}
Result KMemoryMap::RemoveTLS(u32 DSAddr)
{
    u32 offset = (DSAddr - TLS_OFFSET) / 0x200;
    if (m_TLSused[offset])
        return -1;
    m_TLSused[offset] = false;

    //remove the Page if the Page is empty
    bool empty = true;
    for (int i = 0; i < 8; i++)
    {
        if (m_TLSused[(offset / 8) * 8 + i])
            empty = false;
    }
    if (empty)
    {
        RemovePages((offset / 8) * 8 + TLS_OFFSET, 0x1000);
    }
    return Success;
}
s32 KMemoryMap::AllocFreeGSP(bool new3DS,u32 size)
{
	int start = new3DS ? 0x10000000 : 0x8000000;
	int found = -1;
	int needed = (size+0xFFF) / 0x1000;
	for (; 0 < start; start -= 0x1000)
	{
		if (!MEM_FCRAM_Used[start / 0x1000])
		{
			if (needed == 0)
			{
				found = start;
				break;
			}
			needed--;
		}
		else
			needed = (size + 0xFFF) / 0x1000;
	}
	if (found != -1)
	{
		needed = (size + 0xFFF) / 0x1000;
		for (int poi = found / 0x1000; needed > 0; needed--)
		{
			MEM_FCRAM_Used[poi] = true;
			poi++;
		}
	}
	return found;
}