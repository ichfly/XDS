#define FIFOSIZE 0x10
struct FIFO {
    u32     data[FIFOSIZE];
    int     nextwrite;
    int     nextread;
    int     size;
};

//IRQ 50 "new 3DS IRQ interrupt from IPCSYNC"
//IRQ 51 "maybe the old IRQ stuff but not sure"
//IRQ 52 "Receive Fifo Not Empty"
//IRQ 53 "Send Fifo Empty IRQ"

class HWIPC : public IOHW {

public:
    HWIPC(KKernel * kernel);
    u8 Read8(u32 addr);
    u16 Read16(u32 addr);
    u32 Read32(u32 addr);
    void Write8(u32 addr, u8 data);
    void Write16(u32 addr, u16 data);
    void Write32(u32 addr, u32 data);

    u32 FIFOp9read();
    void FIFOp9write(u32 data);

    u8 m_IPCSYNCP9;
    u8 m_IPCSYNCP11;
    u8 m_IPCIRQ;
    u8 m_SENDFIFOSTAT;
    u8 m_RECVFIFOSTAT_ERROR;

    virtual void FIFOReadBackCall() = 0; //everything even flash
    virtual void FIFOWriteBackCall() = 0; //everything even flash
    virtual void FIFOIRQ() = 0;
    virtual void FIFOIRQOLD() = 0; //this is unused on the 3DS
    KKernel * m_kernel;
private:

    u32 FIFOget(struct FIFO* FIFO);
    void FIFOset(struct FIFO* FIFO, u32 data);
    struct FIFO m_recvarm9;
    struct FIFO m_sendarm9;
};