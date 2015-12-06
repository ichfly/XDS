#include "Kernel.h"

//tools


bool KDmaObject::Synchronization(KThread* thread, u32 &error)
{
    return false;
}

KDmaObject::KDmaObject(u8 channel, u8 started, KProcess *owner)
{
	m_channel = channel;
	m_started = started;
    m_owner = owner;
}

KDmaObject::~KDmaObject()
{
}

bool KDmaObject::IsInstanceOf(ClassName name) {
	if (name == KDmaObject::name)
        return true;

    return super::IsInstanceOf(name);
}
