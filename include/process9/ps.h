class P9PS
{
public:
	P9PS(Process9* owner);
	~P9PS();
    void Command(u32 data[],u32 numb);
private:
    Process9* m_owner;
};


