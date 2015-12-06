class DSP : public IOHW {

public:
	DSP(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
protected:
	KKernel * m_kernel;
private:
	//HW stuff
	u16 m_DSP_PDATA, m_DSP_PADR, m_DSP_PCFG, m_DSP_PSTS, m_DSP_PSEM, m_DSP_PMASK, m_DSP_PCLEAR, m_DSP_SEM;
	u16 m_DSP_CMD[3], m_DSP_REP[3];

	u16 DSPreadCMD(u8 numb);
	void DSPwriteRES(u16 data ,u8 numb);
	//HLE software stuff
	u32 m_phase;
	void Reset();
	void REPread(u8 id);
	void CMDwrite(u8 id);
};