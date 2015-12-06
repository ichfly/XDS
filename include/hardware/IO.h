class IOHW {

public:
    virtual u8 Read8(u32 addr) = 0;
    virtual u16 Read16(u32 addr) = 0;
    virtual u32 Read32(u32 addr) = 0;
    virtual void Write8(u32 addr, u8 data) = 0;
    virtual void Write16(u32 addr, u16 data) = 0;
    virtual void Write32(u32 addr, u32 data) = 0;

};