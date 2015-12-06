#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"

#define LOGP9COM

Process9::Process9(KKernel* kernel) :HWIPC(kernel), m_FS(this), m_PM(this), m_PS(this), m_MC(this), m_AM(this)
{
    m_IPCSYNCP9 = 1; // init step
    m_IntiHadData = false;
    m_datarecved = 0;
}
Process9::~Process9()
{
}
void Process9::FIFOReadBackCall()
{
    if (m_IPCSYNCP9 == 1 && m_IntiHadData) // initing
    {
        if (m_RECVFIFOSTAT_ERROR & 0x1) //empty
        {
            m_IPCSYNCP9 = 2; // ready for data
        }
        if (!(m_SENDFIFOSTAT & 0x1))
        {
            u32 temp = FIFOp9read();
            FIFOp9write(temp);
        }
    }
    else if(m_IPCSYNCP9 == 2) //this is send data
    {
        u32 translated = m_datasend[1] & 0x3F;
        u32 nomal = (m_datasend[1] >> 6) & 0x3F;
        u32 size = translated + nomal;
        while (!(m_SENDFIFOSTAT & 0x1) && m_datasended <= size + 1)
        {
            FIFOp9write(m_datasend[m_datasended++]);
        }
    }
}
void Process9::FIFOWriteBackCall()
{
    if (m_IPCSYNCP9 == 1) //init
    {
        if (!(m_RECVFIFOSTAT_ERROR & 0x2))
        {
            u32 temp = FIFOp9read();
            FIFOp9write(temp);
        }
        else
        {
            m_IntiHadData = true;
        }
    }
    else if (m_IPCSYNCP9 == 2)
    {
        //print what you get
        u32 temp = FIFOp9read();
        m_datarecv[m_datarecved] = temp;
        m_datarecved++;
        if (m_datarecved > 1)
        {
            u32 translated = m_datarecv[1] & 0x3F;
            u32 nomal = (m_datarecv[1] >> 6) & 0x3F;
            u32 size = translated + nomal;
            if (m_datarecved > 1 + size)
            {
#ifdef LOGP9COM
                for (u32 i = 1; i < m_datarecved; i++)
                    LOG("recv: %08x", m_datarecv[i]);
#endif
                switch (m_datarecv[0])
                {
				case 0:
					m_MC.Command(&m_datarecv[1], m_datarecv[0]);
					break;
                case 1:
#ifdef LOGP9COM
                    LOG("P9 FS0");
#endif
					m_FS.Command(&m_datarecv[1], m_datarecv[0]);
                    break;
                case 2:
#ifdef LOGP9COM
                    LOG("P9 FS1");
#endif
					m_FS.Command(&m_datarecv[1], m_datarecv[0]);
                    break;
                case 3:
#ifdef LOGP9COM
                    LOG("P9 FS2");
#endif
					m_FS.Command(&m_datarecv[1], m_datarecv[0]);
                    break;
                case 4:
#ifdef LOGP9COM
                    LOG("P9 FS3");
#endif
					m_FS.Command(&m_datarecv[1], m_datarecv[0]);
                    break;
                case 5:
					m_PM.Command(&m_datarecv[1], m_datarecv[0]);
                    break;
				case 7:
					m_AM.Command(&m_datarecv[1], m_datarecv[0]);
					break;
				case 8:
					m_PS.Command(&m_datarecv[1], m_datarecv[0]);
					break;
                default:
                    LOG("P9 data to unknown %08x", m_datarecv[0]);
                }

                m_datarecved = 0;
            }
        }
    }
}
void Process9::FIFOIRQ()
{

}
void Process9::FIFOIRQOLD() //this is unused on the 3DS
{

}
void Process9::Sendresponds(u32 myid, u32 data[])
{
    u32 translated = data[0] & 0x3F;
    u32 nomal = (data[0] >> 6) & 0x3F;
    u32 size = translated + nomal;
    m_datasended = 0;
    m_datasend[0] = myid;

    memcpy(&m_datasend[1], data, (size + 1)*sizeof(u32));
    while (!(m_RECVFIFOSTAT_ERROR & 0x2) && m_datasended <= size + 1)
    {
        FIFOp9write(m_datasend[m_datasended++]);
    }

    m_kernel->FireInterrupt(0x50); //send the pix everything ready to read
}

u64 Process9::GetTitleFromPM(u64 handle)
{
    return m_PM.GetTitle(handle);
}
