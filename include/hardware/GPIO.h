class GPIO : public IOHW {

public:
	GPIO::GPIO(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
protected:
	u32 m_IO, m_DIR;
	KKernel * m_kernel;
};