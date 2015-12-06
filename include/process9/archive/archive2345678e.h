
class Archive2345678e : public Archive
{
public:
    Archive2345678e(Process9* owner, LowPath *lowpath);
    ~Archive2345678e();
	P9File* OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result) override;
private:
    u64 m_title;
};


