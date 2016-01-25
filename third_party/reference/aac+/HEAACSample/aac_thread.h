#ifndef __AAC_THREAD_H
#define __AAC_THREAD_H

#include "win32_thread.h"

class RecordThread : public CWin32_Thread
{
public:
	RecordThread();
	virtual ~RecordThread();

	int				get_bandwidth(int &record_count, int &bandwidth);
protected:
	void			run();

	int				record_count_;
	int				bandwidth_;
};


class PlayThread : public  CWin32_Thread
{
public:
	PlayThread();
	virtual ~PlayThread();

protected:
	void			run();
};
#endif