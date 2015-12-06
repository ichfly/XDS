/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
            Version 2, December 2004

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

    DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include <string.h>

#include <3ds.h>

#include "stdio.h"

#include "svc.h"
Result srvRegisterService(Handle* out, const char* name, int maxSessions);

extern char* fake_heap_start;
extern char* fake_heap_end;
extern u32 __linear_heap;
extern u32 __heapBase;
extern u32 __heap_size, __linear_heap_size;
void __system_allocateHeaps();
void stub(u32* cmdbuf);

void __system_allocateHeaps() {
        u32 tmp=0;

        // Allocate the application heap
        __heapBase = 0x08000000;
        svcControlMemory(&tmp, __heapBase, 0x0, 0x10000, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);

        // Allocate the linear heap
        svcControlMemory(&__linear_heap, 0x0, 0x0, 0x10000, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);
        // Set up newlib heap
        fake_heap_start = (char*)__heapBase;
        fake_heap_end = fake_heap_start + 0x10000;

}

void __appInit() {
        // Initialize services
        srvInit();
}
void __libctru_init(void (*retAddr)(void))
{
    __system_allocateHeaps();
}
void  __attribute__((noreturn)) __libctru_exit(int rc)
{
        svcExitProcess();
}
void __appExit() {
        // Exit services
        srvExit();
}
typedef void (*cmdHandlerFunction)(u32* cmdbuf);


#define NUM_CMD (0x21)
void ConvertProcessAddressFromDspDram(u32* cmdbuf);
void ReadPipeIfPossible(u32* cmdbuf);
void LoadComponent(u32* cmdbuf);
void GetVirtualAddress(u32* cmdbuf);
void GetHeadphoneStatus(u32* cmdbuf);
void RegisterInterruptEvents(u32* cmdbuf);
void GetSemaphoreEventHandle(u32* cmdbuf);
cmdHandlerFunction commandHandlers[NUM_CMD]={
    stub,//RecvData,
    stub,//RecvDataIsReady,
    stub,//SendData ,
    stub,//SendDataIsEmpty,
    stub,//SendFifoEx,
    stub,//RecvFifoEx,
    stub,//WriteReg0x10,
    stub,//GetSemaphore,
    stub,//ClearSemaphore,
    stub,//MaskSemaphore,
    stub,//CheckSemaphoreRequest,
    ConvertProcessAddressFromDspDram  ,
    stub,//WriteProcessPipe  ,
    stub,//ReadPipe,
    stub,//GetPipeReadableSize,
    ReadPipeIfPossible ,
    LoadComponent,
    stub,//UnloadComponent  ,
    stub,//FlushDataCache  ,
    stub,//InvalidateDCache  ,
    RegisterInterruptEvents,
    GetSemaphoreEventHandle  ,
    stub,//SetSemaphoreMask  ,
    stub,//GetPhysicalAddress,
    GetVirtualAddress ,
    stub,//SetIirFilterI2S1,
    stub,//SetIirFilterI2S2,
    stub,//SetIirFilterEQ,
    stub,//ReadMultiEx_SPI2,
    stub,//WriteMultiEx_SPI2,
    GetHeadphoneStatus,
    stub,//ForceHeadphoneOut,
    stub//GetIsDspOccupied

};


u8 data = 0;
u32 sessionHandles[7];
u32 currentHandleIndex = 2;
u32 numSessionHandles = 2;

u32 event;
u32 myeventhandel = 0;
int main()
{
    consoleDebugInit(debugDevice_3DMOO);
    fprintf(stderr,"DSP MODULE LIVE");
    memset(sessionHandles,0,sizeof(sessionHandles));

    srvRegisterService(&sessionHandles[0], "dsp::DSP", 4);

    svcCreateTimer(&sessionHandles[1],0);
    svc_SetTimer(sessionHandles[1],0,40000000);

    svcCreateEvent(&event,1);

    u32 ret = 0;
    while(1)
    {
            ret=svc_replyAndReceive((s32*)&currentHandleIndex, sessionHandles, numSessionHandles, sessionHandles[currentHandleIndex]);
            if(ret==0xc920181a)
            {
                    //close session handle
                    svcCloseHandle(sessionHandles[currentHandleIndex]);
                    sessionHandles[currentHandleIndex]=sessionHandles[numSessionHandles];
                    sessionHandles[numSessionHandles]=0x0;
                    currentHandleIndex=numSessionHandles--; //we want to have replyTarget=0x0
            }else{
                    switch(currentHandleIndex)
                    {
                            case 0:
                                    {
                                            // receiving new session
                                            svc_acceptSession(&sessionHandles[numSessionHandles], sessionHandles[currentHandleIndex]);
                                            numSessionHandles++;
                                            currentHandleIndex = numSessionHandles;
                                            sessionHandles[currentHandleIndex] = 0; //we want to have replyTarget=0x0
                                    }
                                    break;
                            case 1:
                            {
                                    //send event at some points
                                    if(myeventhandel)
                                    {
                                        svcSignalEvent(myeventhandel);
                                    }
                                    currentHandleIndex = numSessionHandles;
                                    sessionHandles[currentHandleIndex] = 0; //we want to have replyTarget=0x0
                            }
                            break;
                            default:
                                    {
                                            //receiving command from ongoing session
                                            u32* cmdbuf=getThreadCommandBuffer();
                                            u8 cmdIndex=cmdbuf[0]>>16;
                                            if(cmdIndex<=NUM_CMD && cmdIndex>0)
                                            {
                                                    commandHandlers[cmdIndex-1](cmdbuf);
                                            }
                                            else
                                            {
                                                    cmdbuf[0] = (cmdbuf[0] & 0x00FF0000) | 0x40;
                                                    cmdbuf[1] = 0xFFFFFFFF;
                                            }
                                    }
                                    break;
                    }
            }
    }

    return 0;
}


static u32 ReadPipeIfPossibleCount = 0;
u32 ReadPipeIfPossibleResp[] = {
    0x000F, //Number of responses
    0xBFFF,
    0x9E8E,
    0x8680,
    0xA78E,
    0x9430,
    0x8400,
    0x8540,
    0x948E,
    0x8710,
    0x8410,
    0xA90E,
    0xAA0E,
    0xAACE,
    0xAC4E,
    0xAC58
};

#define DSPramaddr 0x1FF00000




//the real stuff

void stub(u32* cmdbuf)
{
    fprintf(stderr,"NOT IMPLEMENTED, cid=%08x\n", cmdbuf[0]);
    //just respond with SUCESS and nothing else
    cmdbuf[0]=(cmdbuf[0] & 0x00FF0000) | 0x40;
    cmdbuf[1]=0x00000000;
}
void LoadComponent(u32* cmdbuf)
{
    cmdbuf[0]= 0x00110080;
    cmdbuf[1]= 0x00000000;
    cmdbuf[2]= 0x00000001;
}
void RegisterInterruptEvents(u32* cmdbuf)
{
    if(cmdbuf[0] != 0x00150082)
    {
        cmdbuf[0]= 0x00150040;
        cmdbuf[1]= 0xFFFFFFFF;
        return;
    }
    myeventhandel = cmdbuf[4];
    cmdbuf[0]= 0x00150040;
    cmdbuf[1]= 0x00000000;
}
void GetSemaphoreEventHandle(u32* cmdbuf)
{
    cmdbuf[0]= 0x00160042;
    cmdbuf[1]= 0x00000000; //Sucess
    cmdbuf[2]= 0x00000000; //duplicate
    cmdbuf[3]= event;
}
u16 buffer[0x100];
void ReadPipeIfPossible(u32* cmdbuf)
{
    u32 i = 0;
    u32 unk1 = cmdbuf[1];
    u32 unk2 = cmdbuf[2];
    u32 size = cmdbuf[3] & 0xFFFF;
    u32 initialSize = ReadPipeIfPossibleCount;

    for (i = 0; i < size; i += 2) {
        if (ReadPipeIfPossibleCount < 16) {
            *(buffer + i/2) = ReadPipeIfPossibleResp[ReadPipeIfPossibleCount];
        }
        ReadPipeIfPossibleCount++;
    }
    cmdbuf[0] = 0x00100082;
    cmdbuf[1] = 0; //no error
    cmdbuf[2] = (ReadPipeIfPossibleCount - initialSize) * 2; //no error
    cmdbuf[3] = 0x00000002 | (cmdbuf[2] << 14);
    cmdbuf[4] = buffer;

}

void ConvertProcessAddressFromDspDram(u32* cmdbuf)
{
    u32 addrin = cmdbuf[1];
    cmdbuf[0]= 0x000c0080;
    cmdbuf[1]= 0x00000000;
    cmdbuf[2]= DSPramaddr + addrin*2 + 0x40000;
}
void GetVirtualAddress(u32* cmdbuf)
{
    u32 addr = cmdbuf[1];
    u32 ret = 0;
    if ((addr & 0xF8000000) == 0x20000000) ret = 0x14000000 + addr - 0x20000000;
    if ((addr & 0xFFF80000) == 0x1FF00000) ret = addr;

    cmdbuf[0]= 0x00190080;
    cmdbuf[1]= 0x00000000;
    cmdbuf[2]= ret;
}
void GetHeadphoneStatus(u32* cmdbuf)
{
    cmdbuf[0]= 0x001F0080;
    cmdbuf[1]= 0x00000000;
    cmdbuf[2]= 0x00000000;
}
