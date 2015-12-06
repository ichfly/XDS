#pragma once



class KSharedMemory : public KAutoObject
{
public:

	typedef KAutoObject super;


	KSharedMemory(u32 addr, u32 size,u32 myperm,u32 otherpem, KProcess *owner);
	~KSharedMemory();
    virtual bool IsInstanceOf(ClassName name);
	s32 map(u32 addr, u32 myperm, u32 otherpem, KProcess *caller);

	static const ClassName name = KSharedMemory_Class;

private:
    KProcess *m_owner;
	u32 m_addr;
	u32 m_size;
	u32 m_myperm;
	u32 m_otherpem;
	bool m_IsGSP;
};


