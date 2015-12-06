#include "Kernel.h"


KInterrupt::KInterrupt(KSynchronizationObject* syncObject, s32 priority, bool isManualClear) : m_syncObject(syncObject), m_priority(priority), m_isManualClear(isManualClear)
{

}

KInterrupt::~KInterrupt()
{

}
void KInterrupt::fire()
{
    if ((*m_syncObject)->IsInstanceOf(KEvent_Class))
    {
        KEvent *eve = (KEvent *)*m_syncObject;
        eve->m_open = true;
    }
    KSynchronizationObject* obj = (KSynchronizationObject*)(*m_syncObject);
    obj->SynFreeAll(Success); //hope that is correct...
}
bool KInterrupt::IsInstanceOf(ClassName name) {
    if (name == KInterrupt::name)
        return true;

    return super::IsInstanceOf(name);
}
KAutoObjectRef *KInterrupt::GetObjRef()
{
    return &m_syncObject;
}