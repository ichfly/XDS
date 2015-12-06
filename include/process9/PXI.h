
class Process9 : public HWIPC
{
public:
    Process9(KKernel* kernel);
    ~Process9();
    void FIFOReadBackCall(); //everything even flash
    void FIFOWriteBackCall(); //everything even flash
    void FIFOIRQ();
    void FIFOIRQOLD(); //this is unused on the 3DS
    void Sendresponds(u32 myid,u32 data[]);
    u64 GetTitleFromPM(u64 handle);
private:
	P9MC m_MC;
	P9FS m_FS;
    P9PM m_PM;
	P9PS m_PS;
	P9AM m_AM;
    u32 m_datarecved;
    u32 m_datasended;
    u32 m_datarecv[0x200]; //this is more than enough
    u32 m_datasend[0x200]; //this is more than enough
    bool m_IntiHadData;
};


