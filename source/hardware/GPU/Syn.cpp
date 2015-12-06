#include "Kernel.h"
#include "Hardware.h"

extern "C" void VBlankCallback();

Syncer::Syncer(GPUHW *owner, bool bottom) : m_owner(owner), m_bottom(bottom)
{
	m_owner->m_kernel->m_Timedevent.AddItem(this);
	m_owner->m_kernel->FireNextTimeEvent(this, 4468724); //60 times per sec
}
Syncer::~Syncer()
{
	KLinkedListNode<KTimeedEvent> *t = m_owner->m_kernel->m_Timedevent.list;
	while (t)
	{
		if (t->data == this)
			m_owner->m_kernel->m_Timedevent.RemoveItem(t);
		t = t->next;
	}
}
void Syncer::trigger_event()
{
	if (m_bottom)
		m_owner->m_kernel->FireInterrupt(0x2B); //PDC1/VBlankBottom
	else
	{
		VBlankCallback();
		m_owner->m_kernel->FireInterrupt(0x2A); //PDC0/VBlankTop
		m_owner->m_kernel->FireInterrupt(0x64); //HID interrupt to signal new buttonpressed data is here
	}
	m_owner->m_kernel->FireNextTimeEvent(this, 4468724); //60 times per sec
}