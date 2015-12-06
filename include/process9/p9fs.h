
class Archive;
struct fsArchiveentry
{
    Archive * Archobj;
    u64 id;
};

struct fsFileentry
{
	P9File * Archobj;
	u64 id;
};

typedef fsArchiveentry s_fsArchiveEntry;
typedef fsFileentry s_fsFileentry;

class P9FS
{
public:
    P9FS(Process9* owner);
    ~P9FS();
    void Command(u32 data[],u32 numb);
private:
    Process9* m_owner;
    u64 lastID = 0x110000001;
    KLinkedList<s_fsArchiveEntry> m_open;
	KLinkedList<s_fsFileentry> m_fopen;
};


