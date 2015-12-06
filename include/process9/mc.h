class P9MC
{
public:
	P9MC(Process9* owner);
	~P9MC();
    void Command(u32 data[],u32 numb);
private:
    Process9* m_owner;
};


