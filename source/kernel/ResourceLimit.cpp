#include "Util.h"
#include "Kernel.h"

const char* g_ResourceNames[] =
{
    "Priority",
    "Memory",
    "Thread",
    "Event",
    "Mutex",
    "Semaphore",
    "Timer",
    "Shared Memory",
    "Address Arbiter",
    "CPU Time"
};

bool KResourceLimit::UseResource(u32 resource_id, u32 num)
{
    m_Mutex.Lock();
    m_CurrentUsedResource[resource_id] += num;
    
    if (m_CurrentUsedResource[resource_id] > m_MaxResource[resource_id])
    {
        m_CurrentUsedResource[resource_id] -= num;
        LOG("Out of resource '%s'", g_ResourceNames[resource_id]);
        m_Mutex.Unlock();
        return false;
    }
    m_Mutex.Unlock();
    return true;
}

void KResourceLimit::FreeResource(u32 resource_id, u32 num)
{
    m_Mutex.Lock();
    m_CurrentUsedResource[resource_id] -= num;
    m_Mutex.Unlock();
}

s64 KResourceLimit::GetCurrentValue(u32 resource_id)
{
    return (s64)m_CurrentUsedResource[resource_id];
}

s64 KResourceLimit::GetMaxValue(u32 resource_id)
{
    return (s64)m_MaxResource[resource_id];
}

void KResourceLimit::SetMaxValue(u32 resource_id, s64 number)
{
    m_Mutex.Lock();
    m_MaxResource[resource_id] = (s32)number;
    m_Mutex.Unlock();
}

KResourceLimit::KResourceLimit() : m_Mutex()
{
    memset((void*)m_CurrentUsedResource, 0, sizeof(m_CurrentUsedResource));
    memset((void*)m_MaxResource, 0, sizeof(m_MaxResource));
}

KResourceLimit::~KResourceLimit()
{
}

bool KResourceLimit::IsInstanceOf(ClassName name) {
    if (name == KResourceLimit::name)
        return true;

    return super::IsInstanceOf(name);
}
