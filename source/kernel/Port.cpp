#include "Kernel.h"

//tools


KPort::KPort(char* name, u32 maxconnection) : m_Client(name, maxconnection, this), m_Server(name, maxconnection, this)
{
    strncpy(m_Name, name, 8);
}
KPort::~KPort()
{

}

bool KPort::IsInstanceOf(ClassName name) {
    if (name == KPort::name)
        return true;

    return super::IsInstanceOf(name);
}
