/*********************************************************************/
#include <windows.h>
#include <assert.h>
#include "Mutex.h"

CMutex::CMutex()
{
	m_Handle = ::CreateMutex(NULL, FALSE, NULL);
	assert(m_Handle != NULL);
}

CMutex::~CMutex()
{
	if(m_Handle != NULL)
		CloseHandle(m_Handle);
}

void CMutex::m_vSignal()
{
	assert(::ReleaseMutex(m_Handle));
}

void CMutex::m_vWait()
{
	assert(WaitForSingleObject(m_Handle, INFINITE) != WAIT_FAILED);
}

//////////////////////////////////////////////////////////////////////
CWaitAndSignal::CWaitAndSignal(CMutex& Mutex) : m_Mutex(Mutex)
{
	m_Mutex.m_vWait();
}

CWaitAndSignal::~CWaitAndSignal()
{
	m_Mutex.m_vSignal();
}

/*********************************************************************/

