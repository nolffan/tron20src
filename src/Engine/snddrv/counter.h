// counter.h: the engine's performance counters, as the sound driver sees
// them in the Final build: CounterFinal, its helpers and (in Tron's) the
// CountAdder that the sound thread times itself with.

#ifndef __COUNTER_H__
#define __COUNTER_H__

#include "ltbasetypes.h"

// A counter can be started by its constructor.
#define CSTART_NONE			0
#define CSTART_MILLI		1
#define CSTART_MICRO		2

class CounterFinal
{
public:
					CounterFinal(unsigned long startMode=CSTART_NONE);

	// milliseconds
	void			StartMS();
	unsigned long	EndMS();
	unsigned long	CountMS();

	// microseconds
	void			StartMicro();
	unsigned long	EndMicro();
	unsigned long	CountMicro();

	unsigned long m_Data[2];
};

unsigned long cnt_NumTicksPerSecond();
void cnt_StartCounterFinal(CounterFinal &cCounter);
unsigned long cnt_EndCounterFinal(CounterFinal &cCounter);

#ifdef _FINAL

// Adds the time spent in a scope to a count. RETAIL: Tron's Final build
// counts (NOLF2's compiled to nothing): Thread_Func's two (0x1000357a,
// 0x1000365a). The destructor is no inline function: its copy for the
// unwinding lies among s_dx8.cpp's code (0x10003790), not with the inline
// functions' at the end, so it is defined there.
class CountAdder
{
public:
	CountAdder(uint32 *pNum)
	{
		m_pNum = pNum;
		cnt_StartCounterFinal(m_Counter);
	}

	~CountAdder();

	CounterFinal m_Counter;
	uint32 *m_pNum;
};

#endif // _FINAL

#endif // __COUNTER_H__
