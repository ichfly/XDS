#include "Kernel.h"

//tools

void KServerSession::Destroy() {
	delete m_owner;
	// Empty. Overridden.
}
bool KServerSession::Synchronization(KThread* thread, u32 &error)
{
    KThread * tnew = m_owner->m_Client.SynGetNextPrio();
    if (tnew && !m_owner->m_Server.m_waitingForCmdResp)
    {
        m_owner->m_Server.m_waitingForCmdResp = tnew;
        m_owner->m_Server.m_processingCmd = thread;
        m_owner->Communicate(tnew, thread, false);
        return false;
    }

    return true; //stall
}

KServerSession::KServerSession(KSession *owner)
{
    m_owner = owner;
    m_processingCmd = NULL;
    m_waitingForCmdResp = NULL;
}
KServerSession::~KServerSession()
{

}
s32 KServerSession::reply(KThread * sender)
{
	if (!m_waitingForCmdResp)
	{
		LOG("error responding to no existing thread");
		return -1;
	}
	if (!m_processingCmd)
	{
		LOG("error responding to no existing cmd");
		return -1;
	}

    m_owner->Communicate(sender, m_waitingForCmdResp,true);

    m_owner->m_Client.SynFree(0, m_waitingForCmdResp);

    m_processingCmd = NULL;
    m_waitingForCmdResp = NULL;
    return 0;
}
bool KServerSession::IsInstanceOf(ClassName name) {
    if (name == KServerSession::name)
        return true;

    return super::IsInstanceOf(name);
}
