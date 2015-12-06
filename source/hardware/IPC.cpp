#include "Kernel.h"
#include "Hardware.h"

//todo Interrupt

HWIPC::HWIPC(KKernel * kernel)
{
    m_kernel = kernel;
    m_IPCSYNCP9 = 0;
    m_IPCSYNCP11 = 0;
    m_IPCIRQ = 0;
    m_SENDFIFOSTAT = 1; //Send Fifo Empty
    m_RECVFIFOSTAT_ERROR = 1; //Receive Fifo Empty
    m_recvarm9.data[0] = 0;
    m_recvarm9.size = 0;
    m_recvarm9.nextwrite = 0;
    m_recvarm9.nextread = 0;
    m_sendarm9.data[0] = 0;
    m_sendarm9.size = 0;
    m_sendarm9.nextwrite = 0;
    m_sendarm9.nextread = 0;
}
u32 HWIPC::FIFOget(struct FIFO* FIFO) //nin fifo with its own funtions
{
    s32 readfrom;
    if (FIFO->size == 0)
    {
        readfrom = FIFO->nextread - 1;
    }
    else
    {
        readfrom = FIFO->nextread;
        FIFO->size--;
        FIFO->nextread = (FIFO->nextread + 1) < FIFOSIZE ? (FIFO->nextread + 1) : 0;
    }
    readfrom = readfrom < 0 ? FIFOSIZE - 1 : readfrom ;
    return FIFO->data[readfrom];
}
void HWIPC::FIFOset(struct FIFO* FIFO, u32 data) //nin fifo with its own funtions
{
    if (FIFO->size == FIFOSIZE)
    {
        return;
    }
    else
    {
        FIFO->data[FIFO->nextwrite] = data;
        FIFO->size++;
        FIFO->nextwrite = (FIFO->nextwrite + 1) < FIFOSIZE ? (FIFO->nextwrite + 1) : 0;
    }
}
u8 HWIPC::Read8(u32 addr)
{
    switch (addr & 0xFFF)
    {
    case 0:
        return m_IPCSYNCP9;
    case 1:
        return m_IPCSYNCP11;
    case 2:
        return m_IPCIRQ;
    case 3:
        return 0;
    case 4:
        return m_SENDFIFOSTAT;
    case 5:
        return m_RECVFIFOSTAT_ERROR;
    case 6:
        return 0;
    case 7:
        return 0;
    default:
        LOG("IPC u8 read from %08x", addr);
        return 0;
    }
}
u16 HWIPC::Read16(u32 addr)
{
    if ((addr & 0xFFF) <= 6)
        return (Read8(addr) | ((u16)Read8(addr + 1) << 8));
    else
        LOG("IPC u16 read from %08x", addr);
    return 0;
}
u32 HWIPC::Read32(u32 addr)
{
    if ((addr &0xFFF) <= 4)
        return (Read8(addr) | ((u32)Read8(addr + 1) << 8) | ((u32)Read8(addr + 2) << 16) | ((u32)Read8(addr + 3) << 24));
    else if ((addr & 0xFFF) == 0xC)
    {
        if (m_recvarm9.size == 0)
        {
            m_RECVFIFOSTAT_ERROR |= 0x40; //error
            LOG("IPC u32 reading empty fifo");
        }

        u32 ret = FIFOget(&m_recvarm9);

        if (m_recvarm9.size == 0)
            m_RECVFIFOSTAT_ERROR |= 0x1; //Empty
        m_RECVFIFOSTAT_ERROR &= ~0x2; //not Full

        FIFOReadBackCall();

        return ret;
    }
    else
        LOG("IPC u32 read from %08x", addr);
    return 0;
}

void HWIPC::Write8(u32 addr,u8 data)
{
    switch (addr & 0xFFF)
    {
    case 0:
        break;
    case 1:
        m_IPCSYNCP11 = data&0x4F;
        if (data & 0x20)
            FIFOIRQOLD();
        break;
    case 2:
        if (data & 0x40) //irq enable bit possition changed on 3DS (cmp to DS)
            FIFOIRQ();
        m_IPCIRQ = data & 0x80; //irq bit possition changed on 3DS (cmp to DS)
        break;
    case 3:
        break;
    case 4:
        if (data & 0x8)
        {
            m_sendarm9.data[0] = 0;
            m_sendarm9.size = 0;
            m_sendarm9.nextwrite = 0;
            m_sendarm9.nextread = 0;
            m_SENDFIFOSTAT = 1; //Send Fifo Empty
            FIFOReadBackCall();
        }
        m_SENDFIFOSTAT = (m_SENDFIFOSTAT & ~0x4) | data & 0x4;
       break;
    case 5:
        m_RECVFIFOSTAT_ERROR = (m_RECVFIFOSTAT_ERROR & ~0x84) | data & 0x84;
        if (data & 0x40)
            m_RECVFIFOSTAT_ERROR &= 0x40;
        break;
    case 6:
        break;
    case 7:
        break;
    default:
        LOG("IPC u8 write %08x (%02x)", addr, data);
    }
}
void HWIPC::Write16(u32 addr, u16 data)
{
    if ((addr & 0xFFF) <= 6)
    {
        Write8(addr, (u8)data);
        Write8(addr + 1, (u8)(data << 8));
    }
    else
        LOG("IPC u16 write %08x (%04x)", addr, data);
}
void HWIPC::Write32(u32 addr, u32 data)
{
    if ((addr & 0xFFF) <= 4)
    {
        Write8(addr, data);
        Write8(addr + 1, data << 8);
        Write8(addr + 2, data << 16);
        Write8(addr + 3, data << 24);
    }
    else if ((addr & 0xFFF) == 0x8)
    {
        if (m_sendarm9.size == FIFOSIZE)
        {
            m_RECVFIFOSTAT_ERROR |= 0x40; //error
            LOG("IPC u32 writing full fifo");
        }

        FIFOset(&m_sendarm9,data);

        if (m_sendarm9.size == FIFOSIZE)
            m_SENDFIFOSTAT |= 0x2; //Full
        m_SENDFIFOSTAT &= ~0x1; //not Empty

        FIFOWriteBackCall();

    }
    else
        LOG("IPC u32 write %08x (%08x)", addr, data);
}
u32 HWIPC::FIFOp9read()
{
    u32 ret = FIFOget(&m_sendarm9);

    if (m_sendarm9.size == 0)
    {
        m_SENDFIFOSTAT |= 0x1; //Empty
        m_kernel->FireInterrupt(0x53); //Send Fifo Empty IRQ
    }
    m_SENDFIFOSTAT &= ~0x2; //not Full

    return ret;
}
void HWIPC::FIFOp9write(u32 data)
{
    FIFOset(&m_recvarm9, data);

    if (m_recvarm9.size == FIFOSIZE)
        m_RECVFIFOSTAT_ERROR |= 0x2; //Full
    m_RECVFIFOSTAT_ERROR &= ~0x1; //not Empty
    m_kernel->FireInterrupt(0x52); //Receive Fifo Not Empty
}