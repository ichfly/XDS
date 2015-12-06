#include "Kernel.h"

//tools

KMutex::KMutex(KProcess *owner, bool locked) : m_locked(locked), m_owner(owner), m_lockedThread(NULL)
{

}
void KMutex::Release()
{
    m_Mutex.Lock();
	KThread* found;
	found = SynGetNextPrio();
	if (found)
		SynFree(0, found);
	else
		m_locked = false;
    m_Mutex.Unlock();
}
bool KMutex::Synchronization(KThread* thread,u32 &error)
{
    m_Mutex.Lock();
    if (m_locked)
    {
        if (m_lockedThread == thread) //todo check if this is true...
        {
            m_Mutex.Unlock();
            return false;
        }
        m_Mutex.Unlock();
        return true;
    }
    else
    {
        m_lockedThread = thread;
        m_locked = true;
        m_Mutex.Unlock();
        return false;
    }

}

bool KMutex::IsInstanceOf(ClassName name) {
    if (name == KMutex::name)
        return true;

    return super::IsInstanceOf(name);
}
