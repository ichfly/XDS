
class Archive2345678a : public Archive
{
public:
	Archive2345678a(Process9* owner, LowPath *lowpath);
	~Archive2345678a();
	P9File* OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result) override;
};


