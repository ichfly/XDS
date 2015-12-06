class PDN : public IOHW {

public:
	PDN::PDN(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
	u16 m_SPI_CNT;
protected:
	KKernel * m_kernel;
};