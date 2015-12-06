#include "Kernel.h"

//tools
#define Write32(p,d)     \
p[0] = d & 0xFF;         \
p[1] = (d >> 8) & 0xFF;  \
p[2] = (d >> 16) &0xFF;  \
p[3] = (d >> 24) &0xFF;  \

u8* Mem_VRAM  = NULL;
u8* Mem_DSP   = NULL;
u8* Mem_FCRAM = NULL;
u8* Mem_Configuration = NULL;
u8* Mem_Shared = NULL;
bool* MEM_FCRAM_Used = NULL;

MemChunk* chunk_Configuration;
MemChunk* chunk_Shared;

void Mem_Init(bool new3ds)
{
	Mem_VRAM = (u8*)calloc(0x600000, sizeof(u8));
	Mem_DSP = (u8*)calloc(0x80000, sizeof(u8));
	if (new3ds)
	{
		Mem_FCRAM = (u8*)calloc(0x10000000, sizeof(u8));
		MEM_FCRAM_Used = (bool*)calloc((0x10000000 / 0x1000), sizeof(bool));
	}
	else
	{
		Mem_FCRAM = (u8*)calloc(0x8000000,sizeof(u8));
		MEM_FCRAM_Used = (bool*)calloc((0x8000000 / 0x1000),sizeof(bool));
	}
}
u8* Mem_GetPhysicalPointer(u32 addr) //this is unsave citra stuff
{
	if (0x18000000 <= addr && addr <= 0x18600000)
		return Mem_VRAM + (addr - 0x18000000);
	if (0x20000000 <= addr && addr <= 0x30000000)
		return Mem_FCRAM + (addr - 0x20000000);
	return NULL;
}
void Mem_SharedMemInit()
{
	Mem_Configuration = (u8*)calloc(0x1000, sizeof(u8));
	Mem_Shared = (u8*)calloc(0x1000, sizeof(u8));

    //configure the Configuration Memory mem to strart up normaly no need to configure Mem_Shared that is done by the modules
    //the configuration is from 4.1 (Firm v7712)
    Mem_Configuration[0x00] = 0x00;                  //KERNEL_? (Firm v7712)
    Mem_Configuration[0x01] = 0x00;                  //KERNEL_VERSIONREVISION (Firm v7712)
    Mem_Configuration[0x02] = 0x22;                  //KERNEL_VERSIONMINOR (Firm v7712)
    Mem_Configuration[0x03] = 0x02;                  //KERNEL_VERSIONMAJOR (Firm v7712)
    Write32((&Mem_Configuration[0x04]), 0x0);        //UPDATEFLAG (no update)
    Write32((&Mem_Configuration[0x08]), 0x00008002); //NSTID (NS)
    Write32((&Mem_Configuration[0x0C]), 0x00040130); //NSTID (NS)
    Write32((&Mem_Configuration[0x10]), 0x00000002); //SYSCOREVER (NATIVE_FIRM)
    Mem_Configuration[0x14] = 0x01;                  //UNITINFO (1 for retail 2 for debug other?) if set to Bit(0) is 0 ErrDisp will display development error info
    Mem_Configuration[0x15] = 0x00;                  //BOOT_FIRM (Bit(0) for debug Bit(1) for JTAG connected)
    Mem_Configuration[0x16] = 0x00;                  //PREV_FIRM 0 for cold boot?
    Write32((&Mem_Configuration[0x1C]), 0x0000BA0E); //KERNEL_CTRSDKVERSION (Firm v7712)
    Write32((&Mem_Configuration[0x30]), 0x00000000); //APPMEMTYPE (0-5 for 3DS 0-7 for New3DS)
    Write32((&Mem_Configuration[0x40]), 0x04000000); //APPMEMALLOC (APPMEMTYPE)
    Write32((&Mem_Configuration[0x44]), 0x02C00000); //SYSMEMALLOC (APPMEMTYPE)
    Write32((&Mem_Configuration[0x48]), 0x01400000); //BASEMEMALLOC (APPMEMTYPE)
    Mem_Configuration[0x60] = 0x00;                  //KERNEL_? (Firm v7712)
    Mem_Configuration[0x61] = 0x00;                  //KERNEL_VERSIONREVISION (Firm v7712)
    Mem_Configuration[0x62] = 0x22;                  //KERNEL_VERSIONMINOR (Firm v7712)
    Mem_Configuration[0x63] = 0x02;                  //KERNEL_VERSIONMAJOR (Firm v7712)
    Write32((&Mem_Configuration[0x64]), 0x00000002); //FIRM_SYSCOREVER (Firm v7712 (no update))
    Write32((&Mem_Configuration[0x68]), 0x0000BA0E); //FIRM_CTRSDKVERSION (Firm v7712 (no update))

    chunk_Configuration = new MemChunk();
    chunk_Shared = new MemChunk();
    chunk_Configuration->data = Mem_Configuration;
    chunk_Configuration->size = 0x1000;
    chunk_Shared->data = Mem_Shared;
    chunk_Shared->size = 0x1000;
}
