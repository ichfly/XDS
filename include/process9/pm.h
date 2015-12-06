
struct PMOpenprocess
{
    u64 title;
    u64 handle; //the most uppern bit should be cleared
};

class P9PM
{
public:
    P9PM(Process9* owner);
    ~P9PM();
    void Command(u32 data[],u32 numb);
    u64 GetTitle(u64 handle);
private:
    Process9* m_owner;
    u64 handlecount;
    KLinkedList <PMOpenprocess> m_open;
};


