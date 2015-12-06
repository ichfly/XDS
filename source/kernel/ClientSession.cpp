#include "Kernel.h"

//tools

void KClientSession::Destroy() {
	m_owner->m_Server.SynFreeAll(0xC920181A);
	m_owner->m_Server.m_killed = true;
}
bool KClientSession::Synchronization(KThread* thread, u32 &error)
{
    KThread * tnew = m_owner->m_Server.SynGetNextPrio();
    if (tnew && !m_owner->m_Server.m_processingCmd)
    {
        m_owner->m_Server.m_waitingForCmdResp = thread;
        m_owner->m_Server.m_processingCmd = tnew;
        m_owner->Communicate(thread,tnew , false);
        m_owner->m_Server.SynFree(0, tnew);
    }
    return true; //stall
}

KClientSession::KClientSession(KSession *owner)
{
    m_owner = owner;
}

bool KClientSession::IsInstanceOf(ClassName name) {
    if (name == KClientSession::name)
        return true;

    return super::IsInstanceOf(name);
}
