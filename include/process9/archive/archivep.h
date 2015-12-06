#undef DeleteFile

class Archive
{
public:
    Archive(Process9* owner, LowPath *lowpath);
    virtual char* init() {
		LOG("Archive stub");
		return NULL;
	};
	virtual P9File* OpenFile(LowPath* lowpath, u32 flags, u32 attributes, u32* result) {
		LOG("OpenFile stub");
		return NULL; 
	};
    virtual void CloseFile() {
		LOG("CloseFile stub");
	};
	virtual void DeleteFile(LowPath* lowpath, u32* result) { 
		LOG("DeleteFile stub");
	};
	virtual void RenameFile() { 
		LOG("RenameFile stub");
	};
    ~Archive();
protected:
	LowPath m_lowpath;
    Process9* m_owner;
private:
};


