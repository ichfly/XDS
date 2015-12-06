#include "Kernel.h"

//tools


bool KSemaphore::Synchronization(KThread* thread, u32 &error)
{
    m_Mutex.Lock();
    if (m_count < m_maxcount)
    {
        m_count++;
        m_Mutex.Unlock();
        return false;
    }
    m_Mutex.Unlock();
    return true;
}
s32 KSemaphore::ReleaseSemaphore(u32 releaseCount, u32 &count)
{
    m_Mutex.Lock();
    if (m_count < releaseCount)
    {
        count = m_count;
        m_count = 0;
    }
    else
    {
        count = releaseCount;
        m_count -= releaseCount;
    }
    if (releaseCount > 0)
    {
        KThread *found;
        do
        {
            found = SynGetNextPrio();
            if (found)
            {
                SynFree(0, found);
                m_count++;
            }

        } while (found && m_count < m_maxcount);
    }

    m_Mutex.Unlock();
    return Success;
}
KSemaphore::KSemaphore(u32 count, u32 maxcount, KProcess *owner) :m_Mutex()
{
    m_count = count;
    m_maxcount = maxcount;
    m_owner = owner;
}

bool KSemaphore::IsInstanceOf(ClassName name) {
    if (name == KSemaphore::name)
        return true;

    return super::IsInstanceOf(name);
}
