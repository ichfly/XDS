//error codes
#define SVCERROR_INVALID_OPERATION   0xE0E01BEE
#define SVCERROR_OUT_OF_RANGE        0xE0E01BFD
#define SVCERROR_INVALID_HANDLE      0xD9001BF7
#define SVCERROR_INVALID_PARAMS      0xE0E01BF5
#define SVCERROR_INVALID_SIZE        0xE0E01BF2
#define SVCERROR_ALIGN_ADDR          0xE0E01BF1
#define SVCERROR_INVALID_POINTER     0xD8E007F6
#define SVCERROR_INVALID_ENUM_VALUE  0xD8E007ED
#define SVCERROR_CREATE_HANLE        0xD8A093F8
#define SVCERROR_RESOURCE_LIMIT      0xC860180A

void ProcessSwi(u8 swi, u32* Reg, KThread * currentThread);
u32 ControlMemory_swi(u32* address, u32 addr0, u32 addr1, u32 size, u32 op, u32 permissions, KThread * currentThread);
