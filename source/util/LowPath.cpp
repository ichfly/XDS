#include "Util.h"
#include "Common.h"

LowPath::LowPath(u32 type, u32 size, u32 desc, u8* ptr) : m_type(type), m_size(size), m_desc(desc)
{
	m_ptr = new u8[m_size];
	memcpy(m_ptr, ptr, m_size);
}
LowPath::LowPath(LowPath &pat) : m_type(pat.m_type), m_size(pat.m_size), m_desc(pat.m_desc) 
{
	m_ptr = new u8[pat.m_size];
	memcpy(m_ptr, pat.m_ptr, pat.m_size);
}
LowPath::~LowPath() {
	delete[] m_ptr;
}
