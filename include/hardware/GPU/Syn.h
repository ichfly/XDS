class GPUHW;

class Syncer : public KTimeedEvent
{
public:
	Syncer(GPUHW *owner, bool bottom);
	~Syncer();
	virtual void trigger_event();

private:
	GPUHW *m_owner;
	bool m_bottom;
};


