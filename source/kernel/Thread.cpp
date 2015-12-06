#include "Kernel.h"

KThread::~KThread()
{
    
}

KThread::KThread(s32 core, KProcess *owner) : m_running(true)
{
    m_owner = owner;
    memset(&m_context, 0, sizeof(m_context));
    m_context.mode = RESUME;

    m_thread_id = owner->m_Kernel->GetNextThreadID();

    Threadwaitlist = NULL;
    m_corenumb = core;
}
void KThread::stop()
{
	SynFreeAll(0);
	m_running = false;
}
void KThread::trigger_event()
{
	KLinkedListNode<KTimeedEvent> *t = m_owner->m_Kernel->m_Timedevent.list;
	while (t)
	{
		if (t->data == this)
			m_owner->m_Kernel->m_Timedevent.RemoveItem(t);
		t = t->next;
	}
	SynFree(0, this);//the system is waiting for itself so free it
}
bool KThread::IsInstanceOf(ClassName name) {
    if (name == KThread::name)
        return true;

    return super::IsInstanceOf(name);
}

void KThread::Destroy() {
    // temporary
}

bool KThread::Synchronization(KThread* thread, u32 &error)
{
	return m_running;
}

void KThread::SyncStall(KLinkedList<KSynchronizationObject>* objects, bool waitAll) //todo stop if a error happen
{
    if (objects->list)
    {
        int sorce = 0; //check if the obj is not desolate
        KLinkedListNode<KSynchronizationObject>* current = objects->list;// get the last
        while (current->next != NULL)
        {
            current = current->next;
        }
		while (current != NULL) //check if this is correct but it looks like it is TODO
		{
			if (current->data->m_killed == true)
			{
				//this must search for the core
				if (m_core)
				{
					m_core->SetRegister(1, sorce);
					m_core->SetRegister(0, 0xC920181A); //error closed
					return;
				}
				else
				{
					m_context.cpu_registers[1] = sorce;
					m_context.cpu_registers[0] = 0xC920181A; //error closed
				}
			}
			sorce++;
			current = current->prev;
		}
		
		m_waitAll = waitAll;
        m_owner->m_Kernel->ReScheduler();

        if (Threadwaitlist)
        {
            delete Threadwaitlist;
            Threadwaitlist = NULL;
        }
        Threadwaitlist = objects;

        KLinkedList<KSynchronizationObject> free;


        u32 errorCode = 0;
		//check for 0

		current = objects->list;
		while (current->next != NULL)
		{
			current = current->next;
		}
        while (current != NULL) //check if this is correct but it looks like it is TODO
        {
            bool locked = current->data->Syn(this, errorCode);
            if (!locked)
            {
                free.AddItem(current->data);
                if (!waitAll)
                    break;
            }
            current = current->prev;
        }
        while (free.list)
        {
            free.list->data->SynFree(errorCode, this);
            free.RemoveItem(free.list);
        }
    }

}
void KThread::SyncFree(s32 errorCode, KSynchronizationObject* obj)
{

    //find src
    u32 sorce = 0;

    KLinkedListNode<KSynchronizationObject>* current;// get the last
    if (Threadwaitlist->list) //only if the thread is waiting
    {
        current = Threadwaitlist->list;
        bool found = false;

        while (current->prev != NULL)
        {
            current = current->prev;
        }

        while (current != NULL)
        {
            if (found)
                sorce++;
            if (current->data == obj)
                found = true;

            if (m_waitAll)
                Threadwaitlist->RemoveItem(current);
            current = current->next;
        }

        if (!m_waitAll)
        {
            while (Threadwaitlist->list != NULL)
            {
                Threadwaitlist->list->data->SynRemove(this);
                Threadwaitlist->RemoveItem(Threadwaitlist->list);
            }
        }
        if (!Threadwaitlist->list || errorCode) //check if the error code stuff is correct TODO
        {
            //this must search for the core
            if (m_core)
            {
                if (errorCode || !m_waitAll)
                    m_core->SetRegister(1,sorce);
                m_core->SetRegister(0, errorCode);
            }
            else
            {
                if (errorCode || !m_waitAll)
                    m_context.cpu_registers[1] = sorce;
                m_context.cpu_registers[0] = errorCode;
            }
			LOG("free %s thread %d %08x %08x", m_owner->GetName(), m_thread_id, errorCode, sorce);
            //todo speek to Scheduler here
        }
    }
}