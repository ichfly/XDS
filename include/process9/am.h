class P9AM
{
public:
	P9AM(Process9* owner);
	~P9AM();
    void Command(u32 data[],u32 numb);
private:
    Process9* m_owner;
};


