#include "Kernel.h"

//tools

bool KServerPort::Synchronization(KThread* thread, u32 &error)
{
    if (m_sessionToTake.list) //start now
    {
        return false;
    }
    return true; //stall
}

KServerPort::KServerPort(char* name, u32 maxconnection, KPort *owner) : m_sessionToTake()
{
    m_owner = owner;
}
KServerPort::~KServerPort()
{
}
KServerSession * KServerPort::AcceptSesion()
{
    KSession *s= m_sessionToTake.list->data;
    m_sessionToTake.RemoveItem(m_sessionToTake.list);
    if (s == NULL)
        return NULL;
    return &s->m_Server;
}

bool KServerPort::IsInstanceOf(ClassName name) {
    if (name == KServerPort::name)
        return true;

    return super::IsInstanceOf(name);
}
