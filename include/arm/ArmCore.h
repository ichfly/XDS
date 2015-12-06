enum ArmCoreState {
    STOPPED=0,
    RUNNING=1,
    UNHANDLED_EXCEPTION=2,
    KERNEL_PANIC=3,
    PAUSED_DEBUG=4
};

class KKernel;

class KArmCore {
public:
    KArmCore(KKernel* kernel);

    u64 RunCycles(uint cycles);
    ArmCoreState GetState();
    u32 GetRegister(uint id);
    void SetRegister(uint id,uint data);
    void SetThread(KThread* thread);
    void ReSchedule();
	void Addticks(int ticks);
	s64 Getticks();

private:
    ARM_DynCom m_cpu;
    KKernel* m_kernel;
    KThread* m_thread;
};
