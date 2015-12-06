

extern u8* Mem_VRAM;
extern u8* Mem_DSP;
extern u8* Mem_FCRAM;
extern u8* Mem_Configuration;
extern u8* Mem_Shared;
extern MemChunk* chunk_Configuration;
extern MemChunk* chunk_Shared;
extern bool* MEM_FCRAM_Used;
void Mem_Init(bool new3ds);

void Mem_SharedMemInit();

u8* Mem_GetPhysicalPointer(u32 addr);
