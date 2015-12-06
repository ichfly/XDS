#include "Kernel.h"
#include "Hardware.h"

static const unsigned int sha256_k[64] = //UL = uint32
{ 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };


#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (u8) ((x)      );       \
    *((str) + 2) = (u8) ((x) >>  8);       \
    *((str) + 1) = (u8) ((x) >> 16);       \
    *((str) + 0) = (u8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((u32) *((str) + 3)      )    \
           | ((u32) *((str) + 2) <<  8)    \
           | ((u32) *((str) + 1) << 16)    \
           | ((u32) *((str) + 0) << 24);   \
}


HWHASH::HWHASH(KKernel * kernel) : m_kernel(kernel), HASH_CNT(0)
{

}
u8 HWHASH::Read8(u32 addr)
{
	LOG("HASH u8 read from %08x", addr);
	return 0;
}
u16 HWHASH::Read16(u32 addr)
{
	LOG("HASH u16 read from %08x", addr);
    return 0;
}
u32 HWHASH::Read32(u32 addr)
{
	switch (addr & 0xFFF)
	{
	case 0:
		return HASH_CNT;
	case 0x4:
		return m_total_len;
	case 0x40:
		LOG("");
		return m_h[0];
	case 0x44:
		return m_h[1];
	case 0x48:
		return m_h[2];
	case 0x4C:
		return m_h[3];
	case 0x50:
		return m_h[4];
	case 0x54:
		return m_h[5];
	case 0x58:
		return m_h[6];
	case 0x5C:
		return m_h[7];
	default:
	LOG("HASH u32 read from %08x", addr);
		break;
	}
    return 0;
}

void HWHASH::Write8(u32 addr, u8 data)
{
	LOG("IPC u8 write %08x (%02x)", addr, data);
}
void HWHASH::Write16(u32 addr, u16 data)
{
	LOG("HASH u16 write %08x (%04x)", addr, data);
}
void HWHASH::Write32(u32 addr, u32 data)
{
	switch (addr & 0xFFF)
	{
	case 0: //HASH_CNT
		if (data & 0x1) //init
		{
			m_h[0] = 0x6a09e667;
			m_h[1] = 0xbb67ae85;
			m_h[2] = 0x3c6ef372;
			m_h[3] = 0xa54ff53a;
			m_h[4] = 0x510e527f;
			m_h[5] = 0x9b05688c;
			m_h[6] = 0x1f83d9ab;
			m_h[7] = 0x5be0cd19;
			m_len = 0;
			m_total_len = 0;
		}
		if (data & 0x2)
		{
			m_HASH2->flush();
			finalise();
		}
		if ((data & ~0x3) != 0x8)
			LOG("HASH HASH_CNT (%08x)", data);
		HASH_CNT = data & ~0x3;
		break;
	case 0x4:
		m_total_len = data;
		break;
	case 0x40:
		m_h[0] = data;
		break;
	case 0x44:
		m_h[1] = data;
		break;
	case 0x48:
		m_h[2] = data;
		break;
	case 0x4C:
		m_h[3] = data;
		break;
	case 0x50:
		m_h[4] = data;
		break;
	case 0x54:
		m_h[5] = data;
		break;
	case 0x58:
		m_h[6] = data;
		break;
	case 0x5C:
		m_h[7] = data;
		break;
	default:
		LOG("HASH u32 write %08x (%08x)", addr, data);
		break;
	}
}
void HWHASH::finalise()
{
	unsigned int block_nb;
	unsigned int pm_len;
	unsigned int len_b;
	int i;
	block_nb = (1 + ((SHA256_BLOCK_SIZE - 9)
		< (m_len % SHA256_BLOCK_SIZE)));
	len_b = (m_total_len + m_len) << 3;
	pm_len = block_nb << 6;
	memset(m_block + m_len, 0, pm_len - m_len);
	m_block[m_len] = 0x80;
	SHA2_UNPACK32(len_b, m_block + pm_len - 4);
	transform(m_block, block_nb);
	for (i = 0; i < 8; i++) {
		u32 temp = m_h[i];
		SHA2_UNPACK32(temp, (u8*)&m_h[i]);
	}
}
void HWHASH::transform(const u8 *message, u32 block_nb)
{
	u32 w[64];
	u32 wv[8];
	u32 t1, t2;
	const unsigned char *sub_block;
	int i;
	int j;
	for (i = 0; i < (int)block_nb; i++) {
		sub_block = message + (i << 6);
		for (j = 0; j < 16; j++) {
			SHA2_PACK32(&sub_block[j << 2], &w[j]);
		}
		for (j = 16; j < 64; j++) {
			w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
		}
		for (j = 0; j < 8; j++) {
			wv[j] = m_h[j];
		}
		for (j = 0; j < 64; j++) {
			t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
				+ sha256_k[j] + w[j];
			t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
			wv[7] = wv[6];
			wv[6] = wv[5];
			wv[5] = wv[4];
			wv[4] = wv[3] + t1;
			wv[3] = wv[2];
			wv[2] = wv[1];
			wv[1] = wv[0];
			wv[0] = t1 + t2;
		}
		for (j = 0; j < 8; j++) {
			m_h[j] += wv[j];
		}
	}
}
void HWHASH::update(const u8 *message, u32 len)
{
	unsigned int block_nb;
	unsigned int new_len, rem_len, tmp_len;
	const unsigned char *shifted_message;
	tmp_len = SHA256_BLOCK_SIZE - m_len;
	rem_len = len < tmp_len ? len : tmp_len;
	memcpy(&m_block[m_len], message, rem_len);
	if (m_len + len < SHA256_BLOCK_SIZE) {
		m_len += len;
		return;
	}
	new_len = len - rem_len;
	block_nb = new_len / SHA256_BLOCK_SIZE;
	shifted_message = message + rem_len;
	transform(m_block, 1);
	transform(shifted_message, block_nb);
	rem_len = new_len % SHA256_BLOCK_SIZE;
	memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
	m_len = rem_len;
	m_total_len += (block_nb + 1) << 6;
}

HWHASH2::HWHASH2(KKernel * kernel, HWHASH * hash1) : m_kernel(kernel), m_hash1(hash1), m_curret(0)
{
	hash1->m_HASH2 = this;
}
u8 HWHASH2::Read8(u32 addr)
{
	LOG("HASH u8 read from %08x", addr);
	return 0;
}
u16 HWHASH2::Read16(u32 addr)
{
	LOG("HASH u16 read from %08x", addr);
	return 0;
}
u32 HWHASH2::Read32(u32 addr)
{
	LOG("HASH u32 read from %08x", addr);
	return 0;
}

void HWHASH2::Write8(u32 addr, u8 data)
{
	LOG("IPC u8 write %08x (%02x)", addr, data);
}
void HWHASH2::Write16(u32 addr, u16 data)
{
	LOG("HASH u16 write %08x (%04x)", addr, data);
}
void HWHASH2::Write32(u32 addr, u32 data)
{
	if ((addr & 0xFFF) <= 0x40)
	{
		//printf("%08x", data);
		m_buffer[(m_curret << 2) + 3] = data >> 24;
		m_buffer[(m_curret << 2) + 2] = data >> 16;
		m_buffer[(m_curret << 2) + 1] = data >> 8;
		m_buffer[(m_curret << 2) + 0] = data;
		m_curret++;
		if (m_curret == 0x10)
		{
			m_curret = 0;
			m_hash1->update(m_buffer, 0x40);
		}
	}
	else
		LOG("HASH u32 write %08x (%08x)", addr, data);
}
void HWHASH2::flush()
{
	if (m_curret != 0)
		m_hash1->update(m_buffer, m_curret * 4);
	m_curret = 0;
}