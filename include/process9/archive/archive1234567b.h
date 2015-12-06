
class Archive1234567b : public Archive
{
public:
	Archive1234567b(Process9* owner, LowPath *lowpath);
	P9File* OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result);
	~Archive1234567b();
};


