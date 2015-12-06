class MIC : public IOHW {

public:
	MIC::MIC(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
protected:
	u16 m_CNT;
	/*
	Bit15 running
	Bit14 unknown set at starting mic clear while pause
	Bit13 unknown set at starting mic clear while pause
	Bit12 clear error or clear fifo full flag?
	Bit11 error or fifo full flag
	Bit8 Fifo not empty
	Bit2-3 sample_rate
	Bit1 unknown set at at starting mic
	*/
	KKernel * m_kernel;
};