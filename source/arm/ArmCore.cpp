#include "Kernel.h"

KArmCore::KArmCore(KKernel* kernel) : m_kernel(kernel), m_cpu()
{
    m_thread = NULL;
}
void KArmCore::SetThread(KThread* thread)
{
    if (m_thread)
    {
        m_cpu.SaveContext(m_thread->m_context);
    }

    m_thread = thread;

    m_cpu.state->m_currentThread = m_thread;
    m_cpu.LoadContext(m_thread->m_context);
    m_cpu.state->m_MemoryMap = &m_thread->m_owner->m_memory;
}
u64 KArmCore::RunCycles(uint cycles)
{
    return m_cpu.Run(cycles);
}
void KArmCore::ReSchedule()
{
    m_cpu.PrepareReschedule();
}
void KArmCore::SetRegister(uint id, uint data)
{
    m_cpu.SetReg(id, data);
}
void KArmCore::Addticks(int ticks)
{
	m_cpu.AddTicks(ticks);
}
s64 KArmCore::Getticks()
{
	return m_cpu.GetTicks();
}