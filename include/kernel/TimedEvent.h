#pragma once



class KTimeedEvent
{
public:
	KTimeedEvent(){ num_cycles_remaining = 0; }
	~KTimeedEvent(){ }
	s64 num_cycles_remaining;
	virtual void trigger_event() = 0;


private:
};


