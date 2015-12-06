#include "Kernel.h"

//tools


bool KClientPort::Synchronization(KThread* thread, u32 &error)
{
    return true; //stall
}
s32 KClientPort::connect(KClientSession* &sesion)
{
    if ( m_maxConnection > m_CurrentConnection)
    {
        //free the server so he can accept the connection
        KThread* found = m_owner->m_Server.SynGetNextPrio();
        if (found)
        {
            m_owner->m_Server.SynFree(0, found);
        }
        KSession* sesi = new KSession(m_owner);
        m_owner->m_Server.m_sessionToTake.AddItem(sesi);
        sesion = &sesi->m_Client;
        return Success;
    }
    else
    {
        return -1;
    }
}

KClientPort::KClientPort(char* name, u32 maxConnection, KPort *owner)
{
    m_maxConnection = maxConnection;
    m_CurrentConnection = 0;
    m_owner = owner;
}

bool KClientPort::IsInstanceOf(ClassName name) {
    if (name == KClientPort::name)
        return true;

    return super::IsInstanceOf(name);
}

