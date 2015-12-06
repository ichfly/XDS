class HWHASH2;

static const unsigned int SHA256_BLOCK_SIZE = (512 / 8);

class HWHASH : public IOHW {

public:
	HWHASH(KKernel * kernel);
    u8 Read8(u32 addr);
    u16 Read16(u32 addr);
    u32 Read32(u32 addr);
    void Write8(u32 addr, u8 data);
    void Write16(u32 addr, u16 data);
    void Write32(u32 addr, u32 data);
	void finalise();
	void transform(const u8 *message, u32 block_nb);
	void update(const u8 *message, u32 len);
    KKernel * m_kernel;
private:
	u32 HASH_CNT;
	u32 m_total_len;
	u32 m_len;
	u8 m_block[2 * SHA256_BLOCK_SIZE];
	u32 m_h[8];
	HWHASH2* m_HASH2;
	friend class HWHASH2;
};
class HWHASH2 : public IOHW {

public:
	HWHASH2(KKernel * kernel, HWHASH * hash1);
	u8 Read8(u32 addr);
	u16 Read16(u32 addr);
	u32 Read32(u32 addr);
	void Write8(u32 addr, u8 data);
	void Write16(u32 addr, u16 data);
	void Write32(u32 addr, u32 data);
	void flush();

	KKernel * m_kernel;
	HWHASH * m_hash1;
private:
	u8 m_buffer[0x40];
	u32 m_curret;
};