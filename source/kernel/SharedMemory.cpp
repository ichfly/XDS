#include "Kernel.h"

KSharedMemory::KSharedMemory(u32 addr, u32 size, u32 myperm, u32 otherpem, KProcess *owner) : m_addr(addr), m_owner(owner), m_myperm(myperm), m_otherpem(otherpem)
{
	m_size = ((size / 0x1000) + 1) * 0x1000; //round always up
	if (addr == 0)
	{
		m_IsGSP = true;
		m_addr = m_owner->getMemoryMap()->AllocFreeGSP(false, size);//todo find out if the system is a new 3DS
	}
	else
	{
		m_IsGSP = false;
	}
}
s32 KSharedMemory::map(u32 addr, u32 myperm, u32 otherpem, KProcess *caller) //todo check perm
{
	if (addr == 0)
	{
		if (m_IsGSP)
		{
			if (caller == m_owner)
			{
				MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

				if (chunk == NULL) {
					free(chunk);
					return -1;
				}

				chunk->size = m_size;
				chunk->data = &Mem_FCRAM[m_addr];
				return caller->getMemoryMap()->AddPages(caller->LINEAR_memory_virtual_address_userland + m_addr, m_size, &Mem_FCRAM[m_addr], chunk, m_myperm & myperm, MEMTYPE_SHAREDMEMORY, NULL);
			}
			else
			{
				MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

				if (chunk == NULL) {
					free(chunk);
					return -1;
				}

				chunk->size = m_size;
				chunk->data = &Mem_FCRAM[m_addr];
				return caller->getMemoryMap()->AddPages(caller->LINEAR_memory_virtual_address_userland + m_addr, m_size, &Mem_FCRAM[m_addr], chunk, m_otherpem & myperm, MEMTYPE_SHAREDMEMORY, NULL);

			}
		}
		else
		{
			return -1; //todo get the correct error
		}
	}
	else
	{
		if (!m_IsGSP)
		{
			if (caller == m_owner)
			{
				return -1;
			}
			else
			{
				return m_owner->getMemoryMap()->IPCMap(addr,m_addr , m_size, m_otherpem & myperm, caller->getMemoryMap());
			}
		}
		else
		{
			if (caller == m_owner)
			{
				MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

				if (chunk == NULL) {
					free(chunk);
					return -1;
				}

				chunk->size = m_size;
				chunk->data = &Mem_FCRAM[m_addr];
				return caller->getMemoryMap()->AddPages(addr, m_size, &Mem_FCRAM[m_addr], chunk, m_myperm & myperm, MEMTYPE_SHAREDMEMORY, NULL);
			}
			else
			{
				MemChunk* chunk = (MemChunk*)malloc(sizeof(MemChunk));

				if (chunk == NULL) {
					free(chunk);
					return -1;
				}

				chunk->size = m_size;
				chunk->data = &Mem_FCRAM[m_addr];
				return caller->getMemoryMap()->AddPages(addr, m_size, &Mem_FCRAM[m_addr], chunk, m_otherpem & myperm, MEMTYPE_SHAREDMEMORY, NULL);
			}
		}
	}
	return 0;
}

bool KSharedMemory::IsInstanceOf(ClassName name) {
	if (name == KSharedMemory::name)
        return true;

    return super::IsInstanceOf(name);
}
