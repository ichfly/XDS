#include "Kernel.h"

//tools


bool KEvent::Synchronization(KThread* thread, u32 &error)
{
	m_Mutex.Lock();
    if (m_open)
    {
        if (!m_manual)
            m_open = false;
		m_Mutex.Unlock();
        return false;
    }
	m_Mutex.Unlock();
    return true; //stall
}
void KEvent::Triggerevent()
{
	m_Mutex.Lock();
    KThread* found;
    if (m_manual)
    {
        do
        {
            found = SynGetNextPrio();
            SynFree(0, found);

        } while(found);
        m_open = true;
    }
    else
    {
        found = SynGetNextPrio();
        if (found)
            SynFree(0, found);
        else
            m_open = true;
    }
	m_Mutex.Unlock();
}
void KEvent::Clear()
{
	m_Mutex.Lock();
	m_open = false;
	m_Mutex.Unlock();
}

KEvent::KEvent(u32 priority, bool manual, KProcess *owner)
{
    m_open = false;
    m_priority = priority;
    m_manual = manual;
    m_owner = owner;
}

bool KEvent::IsInstanceOf(ClassName name) {
    if (name == KEvent::name)
        return true;

    return super::IsInstanceOf(name);
}
