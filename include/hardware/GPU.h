#include "hardware/GPU/Syn.h"

class GPUHW : public IOHW {

public:
	GPUHW(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
	KKernel * m_kernel;
protected:
	u32 m_data[0x8000];
	Syncer * top;
	Syncer * bot;
};