class IOI2C : public IOHW {

public:
	IOI2C(KKernel * kernel);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
protected:
	virtual bool Read(u8 &data, u8 device, bool end, bool& noack) = 0;
	virtual bool Write(u8 &data, u8 device, bool end, bool& noack) = 0;
	KKernel* m_kernel;
	u8 m_buffer[0x10000];
	int m_index;
	u8 m_CNT;
	u8 m_data;
	u8 m_deviceID;
};

#include "i2c/Bus1.h"
#include "i2c/Bus2.h"
#include "i2c/Bus3.h"