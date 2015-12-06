#include "Kernel.h"

//tools

KTimer::KTimer(KProcess *owner, u32 resettype) : m_ResetType(resettype), m_owner(owner), m_Enabled(false), m_locked(true)
{
	m_owner->m_Kernel->m_Timedevent.AddItem(this);
}
KTimer::~KTimer()
{
	KLinkedListNode<KTimeedEvent> *t = m_owner->m_Kernel->m_Timedevent.list;
	while (t)
	{
		if (t->data == this)
			m_owner->m_Kernel->m_Timedevent.RemoveItem(t);
		t = t->next;
	}
	
}
void KTimer::trigger_event()
{
	KThread* found;
	found = SynGetNextPrio();
	if (found)
		SynFree(0, found);
	else
		if (m_ResetType != 2) //pulse
			m_locked = false;
	m_owner->m_Kernel->FireNextTimeEvent(this, m_Interval);
}
Result KTimer::SetTimer(s64 initial, s64 interval)
{
	m_Enabled = true;
	m_Initial = initial;
	m_Interval = interval;
	m_owner->m_Kernel->FireNextTimeEvent(this, m_Initial + 1);
	return Success;
}
void KTimer::Cancel()
{
	m_Enabled = false;
	num_cycles_remaining = 0;
}
bool KTimer::Synchronization(KThread* thread, u32 &error)
{
	
	bool temp = m_locked;
	if (m_ResetType != 1)
		m_locked = true;
	return temp;
}

bool KTimer::IsInstanceOf(ClassName name) {
	if (name == KTimer::name)
        return true;

    return super::IsInstanceOf(name);
}
