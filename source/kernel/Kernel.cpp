#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"


KKernel::KKernel() : m_core0(this), m_core1(this), m_core2(this), m_core3(this), m_NextProcessID(0), m_NextThreadID(0), tempsh(), m_numbFirmProcess(0)
{
    memset(m_FIRM_Launch_Parameters, 0, sizeof(m_FIRM_Launch_Parameters));
    for (int i = 0; i < sizeof(m_Interrupt) / sizeof(KLinkedList<KInterrupt>*); i++)
        m_Interrupt[i] = new KLinkedList<KInterrupt>();
    m_p9 = new Process9(this);
	m_hash1 = new HWHASH(this);
	m_hash2 = new HWHASH2(this, m_hash1);
	m_I2C1 = new HWBUS1(this);
	m_I2C2 = new HWBUS2(this);
	m_I2C3 = new HWBUS3(this);
	m_GPIO = new GPIO(this);
	m_PDN = new PDN(this);
	m_SPI0 = new SPI(this);
	m_SPI1 = new SPI(this);
	m_SPI2 = new SPI(this);
	m_DSP = new DSP(this);
	m_GPU = new GPUHW(this);
	m_HID = new HID(this);
	m_MIC = new MIC(this);
}

void KKernel::AddProcess(KProcess* p, bool is_firm_process)
{
    if (is_firm_process)
        m_numbFirmProcess++;
    m_processes.AddItem(p);
}

void KKernel::AddQuickCodeProcess(u8* buf, size_t size) {
    //KCodeSet* codeset = new KCodeSet(buf, (size + 0xFFF) / 0x1000, NULL, 0, NULL, 0, 0, 0x1, "Code");
    //auto p = new KProcess(codeset, 0, NULL);

    //m_processes.AddItem(p);
}
void KKernel::StartThread(KThread* p)
{
    tempsh.AddItem(p);
}
void KKernel::ReScheduler()
{
    m_core0.ReSchedule();
}
void KKernel::StopThread(KThread* p)
{
    KLinkedListNode<KThread> *temp = tempsh.list;
    while (temp)
    {
        if (temp->data == p)
            break;
        temp = temp->next;
        if (!temp->next)
            break;
    }
    if (temp)
        tempsh.RemoveItem(temp);
}
u64 KKernel::FindTimedEventWithSmallestCyclesRemaining()
{
	int min = 1000;
	KLinkedListNode<KTimeedEvent> *t = m_Timedevent.list;
	while (t)
	{
		if (t->data->num_cycles_remaining != 0)
		{
			min = min > t->data->num_cycles_remaining ? t->data->num_cycles_remaining : min;
		}
		t = t->next;
	}
	return min + 1;
}

extern "C" void citraFireInterrupt(int id);

void KKernel::ThreadsRunTemp()
{
	KLinkedListNode<KThread> *temp = tempsh.list;
	while (true)
	{
		KThread * current = temp->data;
		if (current->m_running)
		{
			m_core0.SetThread(current);
			if (!current->Threadwaitlist || !current->Threadwaitlist->list)
			{
				current->m_core = &m_core0;

				u64 min_cycles = FindTimedEventWithSmallestCyclesRemaining();
				u64 cycles_run = m_core0.RunCycles((uint)min_cycles);
				KLinkedListNode<KTimeedEvent> *t = m_Timedevent.list;
				while (t)
				{
					if (t->data->num_cycles_remaining != 0)
					{
						t->data->num_cycles_remaining -= cycles_run;
						if (t->data->num_cycles_remaining <= 0)
						{
							t->data->num_cycles_remaining = 0;
							t->data->trigger_event();
						}
					}
					t = t->next;
				}


				current->m_core = NULL;
			}
		}
		else
		{
			StopThread(current);
		}
		if (!temp->next)
		{
			temp = tempsh.list;

		//fallback todo increas cycels
		u64 min_cycles = FindTimedEventWithSmallestCyclesRemaining();

		m_core0.Addticks(min_cycles);
		m_core1.Addticks(min_cycles);
		m_core2.Addticks(min_cycles);
		m_core3.Addticks(min_cycles);

		KLinkedListNode<KTimeedEvent> *t = m_Timedevent.list;
		while (t)
		{
			if (t->data->num_cycles_remaining != 0)
			{
				t->data->num_cycles_remaining -= min_cycles;
				if (t->data->num_cycles_remaining <= 0)
				{
					t->data->num_cycles_remaining = 0;
					t->data->trigger_event();
				}
			}
			t = t->next;
		}

		}
		else
			temp = temp->next;



	}
}
u32 KKernel::GetNextThreadID()
{
    return m_NextThreadID++;
}
u32 KKernel::GetNextProcessID()
{
    return m_NextProcessID++;
}
s32 KKernel::RegisterInterrupt(u32 name, KSynchronizationObject* syncObject, s32 priority, bool isManualClear)
{
    if (name < 0x80)
    {
        //check if already registered
        KLinkedListNode<KInterrupt> *node = m_Interrupt[name]->list;
        while (node)
        {
            if (**(node->data->GetObjRef()) == syncObject)
                return -1;
            node = node->next;
        }

        KInterrupt* inter = new KInterrupt(syncObject, priority, isManualClear);
        m_Interrupt[name]->AddItem(inter);
        return Success;
    }
    else
        return -1;
}
s32 KKernel::UnRegisterInterrupt(u32 name, KSynchronizationObject* syncObject)
{
    KLinkedListNode<KInterrupt> *node = m_Interrupt[name]->list;
    while (node)
    {
        if (**(node->data->GetObjRef()) == syncObject)
        {
            m_Interrupt[name]->RemoveItem(node);
            return 1;
        }
        node = node->next;
    }
    return -1;

}
void KKernel::FireNextTimeEvent(KTimeedEvent* eve, u64 ticks)
{
	eve->num_cycles_remaining = ticks; //todo make more acurate
}
void KKernel::FireInterrupt(u32 name)
{
    KLinkedListNode<KInterrupt> *node = m_Interrupt[name]->list;
    while (node)
    {
        node->data->fire();
        node = node->next;
    }
}
