#include "Kernel.h"

KSynchronizationObject::KSynchronizationObject() : waiting(), m_killed(false)
{

}

void KSynchronizationObject::SynFreeAll(u32 errorCode) {

    while (waiting.list != NULL)
    {
        KThread* thr = waiting.list->data;
        thr->SyncFree(errorCode, this);
        //waiting.RemoveItem(waiting.list);
    }
}

void KSynchronizationObject::SynFree(u32 errorCode, KThread* thread)
{
    KLinkedListNode<KThread>* current = waiting.list;
    while (current != NULL)
    {
        if (current->data == thread)
        {
			KThread* thr = current->data;
            waiting.RemoveItem(current);
            thr->SyncFree(errorCode, this);
        }
        current = current->next;
    }
}
void KSynchronizationObject::SynRemove(KThread* thread)
{
    KLinkedListNode<KThread>* current = waiting.list;
    while (current != NULL)
    {
        if (current->data == thread)
        {
            KThread* thr = waiting.list->data;
            waiting.RemoveItem(current);
        }
        current = current->next;
    }
}

bool KSynchronizationObject::Syn(KThread* thread, u32 &error)
{
    waiting.AddItem(thread);
    if (Synchronization(thread, error) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}
KThread* KSynchronizationObject::SynGetNextPrio() //the threads have 2 prioritys don't know what the different is we only use one here so TODO change that also what happens when 2 have the same prio
{
    KThread* found = NULL;
    KLinkedListNode<KThread>* current = waiting.list;
    while (current)
    {
        KThread* thr = waiting.list->data;
        if (found)
        {
            if (found->m_thread_prio < thr->m_thread_prio)
                found = thr;
        }
        else
        {
            found = thr;
        }
        current = current->next;
    }
    return found;
}

bool KSynchronizationObject::IsInstanceOf(ClassName name) {
    if (name == KSynchronizationObject_Class)
        return true;

    return super::IsInstanceOf(name);
}
