#include "Kernel.h"


KHandleTable::KHandleTable(KProcess* process, u32 size) {
    if(size == 0)
        size = 0x28;

    m_process = process;
    m_size = size;
    m_counter = 0;
    m_next_free = NULL;

    // Allocate handle-table.
    if((m_handles = (HandleEntry*)malloc(sizeof(HandleEntry) * size)) == NULL) {
        // XXX: Panic.
        m_size = 0;
        return;
    }

    // Generate empty handle-table.
    for(u32 i=0; i<size-1; i++) {
        m_handles[i].handle = 0;
        m_handles[i].ptr.next_free = &m_handles[i+1];
    }

    m_handles[size-1].ptr.next_free = NULL;
    m_next_free = &m_handles[0];
}

Result KHandleTable::CreateHandle(Handle& handle_out, KAutoObject* obj) {
    // If we have no free, table is full.
    if(m_next_free == NULL)
        return -1;

    // Get the next free entry.
    HandleEntry* entry = m_next_free;
    u32 index = (u32)((ptrdiff_t) (entry - m_handles));

    // Move next free entry forward.
    m_next_free = entry->ptr.next_free;
	//LOG("use next %08x <- %08x", index, m_next_free - m_handles);

    // Generate handle.
    m_counter = (m_counter + 1) & 0x7FFF;
    Handle handle = (m_counter << 15) | index;
    handle_out = handle;

    // Setup entry.
    entry->handle = handle;
    entry->ptr.object = obj;

    obj->AcquireReference();
    return Success;
}

Result KHandleTable::GetHandleObject(KAutoObjectRef& obj_out, Handle handle) {
    u32 index = handle & 0x7FFF;

    if(index >= m_size)
        return -1;

    if(m_handles[index].handle == handle) {
        obj_out.SetObject(m_handles[index].ptr.object);
        return Success;
    }

    return -1;
}

Result KHandleTable::CloseHandle(Handle handle) {
    u32 index = handle & 0x7FFF;

    if(index >= m_size)
        return -1;

    if(m_handles[index].handle == handle) {
        m_handles[index].ptr.object->ReleaseReference();
		m_handles[index].ptr.object = NULL;
		m_handles[index].handle = -1; //prevent from freeing 2 times the same handle

        // Update next free entry.
		//LOG("free next %08x -> %08x", index, m_next_free - m_handles);
        m_handles[index].ptr.next_free = m_next_free;
        m_next_free = &m_handles[index];

        return Success;
    }

    return -1;
}
