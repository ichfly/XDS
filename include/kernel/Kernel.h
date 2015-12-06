#include <array>
#include "Hardware.h"

#define SERVICECORE 1


class Process9;
class KPort;
class KInterrupt;

class KKernel {
public:
    KKernel();
    void AddQuickCodeProcess(u8* buf, size_t size); // TEMP
    void AddProcess(KProcess* p, bool is_firm_process);
    void StartThread(KThread* p);
    void StopThread(KThread* p);
    void ReScheduler();
    void ThreadsRunTemp();
    u32 GetNextThreadID();
    u32 GetNextProcessID();
    s32 RegisterInterrupt(u32 name, KSynchronizationObject* syncObject, s32 priority, bool isManualClear);
    s32 UnRegisterInterrupt(u32 name, KSynchronizationObject* syncObject);
    void FireInterrupt(u32 name);
	void FireNextTimeEvent(KTimeedEvent* eve, u64 ticks);
    u32 m_numbFirmProcess;
    KLinkedList<KPort> m_Portlist;
    KLinkedRefList<KProcess> m_processes;
    KMemoryMap* m_IPCFIFOAdresses[0xF];
    bool m_IPCFIFOAdressesRO[0xF];
    u8 m_FIRM_Launch_Parameters[0x1000];

    Process9* m_p9;
	HWHASH* m_hash1;
	HWHASH2* m_hash2;
	IOI2C * m_I2C1;
	IOI2C * m_I2C2;
	IOI2C * m_I2C3;
	GPIO * m_GPIO;
	PDN * m_PDN;
	SPI * m_SPI0;
	SPI * m_SPI1;
	SPI * m_SPI2;
	DSP * m_DSP;
	GPUHW * m_GPU;
	HID * m_HID;
	MIC * m_MIC;
	KLinkedList<KTimeedEvent> m_Timedevent;


private:
	u64 FindTimedEventWithSmallestCyclesRemaining();
    KArmCore m_core0;
    KArmCore m_core1;
    KArmCore m_core2;
    KArmCore m_core3;
    KLinkedList<KThread> tempsh;
    u32 m_NextProcessID;
    u32 m_NextThreadID;
    KLinkedList<KInterrupt> *m_Interrupt[0x80];
};
