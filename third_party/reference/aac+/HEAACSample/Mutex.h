/*********************************************************************
线程信号量的定义
*********************************************************************/
#ifndef __MUTEX_H
#define __MUTEX_H

class CMutex
{
public:
	CMutex();
	~CMutex();

	void m_vSignal();
	void m_vWait();

protected:
	HANDLE m_Handle;
};

class CWaitAndSignal
{
public:
	CWaitAndSignal(CMutex& Mutex);
	~CWaitAndSignal();
	
protected:
	CMutex& m_Mutex;
};

#endif
/**********************************************************************/