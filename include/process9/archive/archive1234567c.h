
class Archive1234567c : public Archive
{
public:
	Archive1234567c(Process9* owner, LowPath *lowpath);
	~Archive1234567c();
	P9File* OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result) override;
	void DeleteFile(LowPath* lowpath, u32* result) override;
};


