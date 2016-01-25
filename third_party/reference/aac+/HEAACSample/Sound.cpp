/*********************************************************************************************************************
���ߣ�YRX
���ڣ�2004-09-14
������ʵ��CSoundFormat��CWaveBuffer��CSoundChannel����
��־��2004-09-14		yrx����(ʵ�ֻ�����������ʼ������д���������õ�)
	  2004-09-27		yrx���붪��������ֹ��ʱ
*********************************************************************************************************************/
#include <assert.h>
//#include "stdafx.h"
#include "Sound.h"


CSoundFormat::CSoundFormat()
{
	m_iSize = 0;
	m_pWaveFormat = NULL;
}

CSoundFormat::CSoundFormat(const CSoundFormat& sfAudioFormat)
{
	m_iSize = sfAudioFormat.m_iSize;

	m_pWaveFormat = (WAVEFORMATEX *)malloc(m_iSize);
	assert(m_pWaveFormat != NULL);

	memcpy(m_pWaveFormat, sfAudioFormat.m_pWaveFormat, m_iSize);
}

CSoundFormat::~CSoundFormat()
{
	if(m_pWaveFormat != NULL)
		free(m_pWaveFormat);
}

CSoundFormat& CSoundFormat::operator =(const CSoundFormat& sfAudioFormat)
{
	if(this == &sfAudioFormat)
		return *this;

	if(m_pWaveFormat != NULL)
		free(m_pWaveFormat);

	//���ݿ���
	m_iSize = sfAudioFormat.m_iSize;
	m_pWaveFormat = (WAVEFORMATEX *)malloc(m_iSize);
	assert(m_pWaveFormat != NULL);

	memcpy(m_pWaveFormat, sfAudioFormat.m_pWaveFormat, m_iSize);

	return *this;
}

WAVEFORMATEX* CSoundFormat::operator ->() const
{
	return m_pWaveFormat;
}

WAVEFORMATEX& CSoundFormat::operator*() const
{
	return *m_pWaveFormat;
}

bool CSoundFormat::m_vSetSize(int iSize)
{
	if(m_pWaveFormat != NULL)
		free(m_pWaveFormat);

	if(iSize == 0)
		m_pWaveFormat = NULL;
	else
	{
		if(iSize < sizeof(WAVEFORMATEX))
			iSize = sizeof(WAVEFORMATEX);

		m_pWaveFormat = (WAVEFORMATEX *)calloc(iSize, 1);
		m_pWaveFormat->cbSize =(WORD)(iSize - sizeof(WAVEFORMATEX));
	}

	return m_pWaveFormat!=NULL ? true : false;
}

int CSoundFormat::m_iGetSize() const
{
	return m_iSize;
}

void CSoundFormat::m_vSetAudioFormat(unsigned uChannel, unsigned uSampleRate, unsigned uBitsPer)
{
	//ͨ��ֻ��1��2ͨ��
	assert(uChannel == 1 || uChannel == 2);
	//����λֻ��8��16λ
	assert(uBitsPer == 8 || uBitsPer == 16);

	if(m_pWaveFormat != NULL)
		free(m_pWaveFormat);

	m_iSize = sizeof(WAVEFORMATEX);
	m_pWaveFormat = (WAVEFORMATEX *)malloc(m_iSize);
	assert(m_pWaveFormat != NULL);

	//����������ʽ
	m_pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	m_pWaveFormat->nChannels = uChannel;
	m_pWaveFormat->wBitsPerSample = uBitsPer;
	m_pWaveFormat->nSamplesPerSec = uSampleRate;
	m_pWaveFormat->nBlockAlign = (WORD)(uChannel*(uBitsPer)/8);
	m_pWaveFormat->nAvgBytesPerSec = m_pWaveFormat->nSamplesPerSec * m_pWaveFormat->nBlockAlign;
	m_pWaveFormat->cbSize = 0;
}

void CSoundFormat::m_vSetAudioFormat(const void* pData, int iSize)
{
	m_iSize = iSize;
	memcpy(m_pWaveFormat, pData, iSize);
}

//////////////////////////////////////////CWaveBuffer////////////////////////////////////////////////
CWaveBuffer::CWaveBuffer()
{
	m_hWaveOut = NULL;
	m_hWaveIn = NULL;
	m_header.dwFlags = WHDR_DONE;
	m_pData = NULL;
	m_iSize = 0;
}

CWaveBuffer::~CWaveBuffer()
{
	m_dwRelease();
	if(m_pData != NULL)
		free(m_pData);
}

void CWaveBuffer::m_vSetSize(int iSize)
{
	if(iSize != 0)
	{
		if(m_pData != NULL)
			free(m_pData);

		m_pData = (char *)malloc(iSize + 3);
		assert(m_pData != NULL);

		memset(m_pData, 0, iSize);
		
		m_iSize = iSize;
	}
}

int CWaveBuffer::m_iGetSize()
{
	return m_iSize;
}

void* CWaveBuffer::m_pGetPointer()
{
	return (void *)m_pData;
}

DWORD CWaveBuffer::m_dwPrepare(HWAVEOUT hWaveOut, int& iCount)
{
	if(iCount > m_iSize)
		iCount = m_iSize;

	m_vPrepareCommon(iCount);
	m_hWaveOut = hWaveOut;
	return waveOutPrepareHeader(m_hWaveOut, &m_header, sizeof(m_header));
}

DWORD CWaveBuffer::m_dwPrepare(HWAVEIN hWaveIn)
{
	m_vPrepareCommon(m_iSize);
	m_hWaveIn = hWaveIn;
	
	return waveInPrepareHeader(m_hWaveIn, &m_header, sizeof(m_header));
}

void CWaveBuffer::m_vPrepareCommon(int iCount)
{
	m_dwRelease();

	memset(&m_header, 0, sizeof(m_header));
	m_header.lpData = (char *)m_pData;
	m_header.dwBufferLength = iCount;
	m_header.dwUser = (DWORD)this;
}

DWORD CWaveBuffer::m_dwRelease()
{
	DWORD dwErr = MMSYSERR_NOERROR;

	if(m_hWaveOut != NULL)
	{
		if((dwErr = waveOutUnprepareHeader(m_hWaveOut, &m_header, sizeof(m_header))) == WAVERR_STILLPLAYING)
			return dwErr;

		m_hWaveOut = NULL;
	}
	
	if(m_hWaveIn != NULL)
	{
		if((dwErr = waveInUnprepareHeader(m_hWaveIn, &m_header, sizeof(m_header))) == WAVERR_STILLPLAYING)
			return dwErr;

		m_hWaveIn = NULL;
	}

	m_header.dwFlags |= WHDR_DONE;
	return MMSYSERR_NOERROR;
}

///////////////////////////////////////////CSoundChannel/////////////////////////////////////////////
CSoundChannel::CSoundChannel()
{
	m_vConstruct();
}

CSoundChannel::CSoundChannel(const char* pcDevName, EDIRECTIONS eDir, unsigned uChannel, 
							 unsigned uSampleRate, unsigned uBitPer)
{
	m_vConstruct();
	m_sfAudioFormat.m_vSetAudioFormat(uChannel, uSampleRate, uBitPer);
}

CSoundChannel::~CSoundChannel()
{
	if(!m_bClose)
	{
		m_bCloseAudio();
	}
	if(m_hEventDone != NULL)
		CloseHandle(m_hEventDone);

	m_vClearBuffers();
}

void CSoundChannel::m_vConstruct()
{
	m_eDir = e_Player;
	m_hWaveOut = NULL;
	m_hWaveIn = NULL;
	m_hEventDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	m_sfAudioFormat.m_vSetAudioFormat(1, 8000, 16);
	
	m_iBufByteOffset = 0x7fffffff;
	m_uOldHandle = -1;
	m_iPacket = 1;

	m_bClose = true;
}

SStringArray CSoundChannel::m_saGetDevNames(EDIRECTIONS eDir)
{
	SStringArray strArray;
	strArray.clear();

	unsigned uNumDevs, uID;
	switch(eDir)
	{
	case e_Player:	//ȡ�ò����豸����
		uNumDevs = waveOutGetNumDevs();
		for(uID = 0; uID < uNumDevs; uNumDevs++)
		{
			WAVEOUTCAPS capTmp;
			if(waveOutGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0)
			{
				string strTmp = capTmp.szPname;
				strArray.push_back(strTmp);
			}
		}
		break;

	case e_Recorder:	//ȡ��¼���豸����
		uNumDevs = waveInGetNumDevs();
		for(uID = 0; uID < uNumDevs; uNumDevs++)
		{
			WAVEINCAPS capTmp;
			if(waveInGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0)
			{
				string strTmp = capTmp.szPname;
				strArray.push_back(strTmp);
			}
		}
		break;
	}

	return strArray;
}

string CSoundChannel::m_sGetDefaultDevice(EDIRECTIONS eDir)
{
	string strDevName;

	if(eDir == e_Player)
	{
		WAVEOUTCAPS capTmp;
		if(waveOutGetDevCaps(0, &capTmp, sizeof(capTmp)) == 0)
			strDevName = capTmp.szPname;
	}
	else
	{
		WAVEINCAPS capTmp;
		if(waveInGetDevCaps(0, &capTmp, sizeof(capTmp)) == 0)
			strDevName = capTmp.szPname;
	}

	return strDevName;
}

bool CSoundChannel::m_bGetDeviceID(const char* strDevName, EDIRECTIONS eDir, unsigned& uID)
{
	if(strDevName == NULL)
		return false;
	
	bool bRet = false;

	if(strDevName == "")
	{
		switch(eDir)
		{
		case e_Player:
			if(uID < waveOutGetNumDevs())
			{
				WAVEOUTCAPS capTmp;
				if(waveOutGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0)
				{
					bRet = true;
					m_strDevName = capTmp.szPname;
				}
			}
			break;

		case e_Recorder:
			if(uID < waveInGetNumDevs())
			{
				WAVEINCAPS capTmp;
				if(waveInGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0)
				{
					bRet = true;
					m_strDevName = capTmp.szPname;
				}
			}
			break;
		}
	}
	else
	{
		switch(eDir)
		{
		case e_Player:
			for(uID=0; uID < waveOutGetNumDevs(); uID++)
			{
				WAVEOUTCAPS capTmp;
				if(waveOutGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0 && stricmp(strDevName, capTmp.szPname)==0)
				{
					bRet = true;
					m_strDevName = capTmp.szPname;
					break;
				}
			}
			break;

		case e_Recorder:
			WAVEINCAPS capTmp;
			for(uID=0; uID < waveInGetNumDevs(); uID++)
			{				
				if(waveInGetDevCaps(uID, &capTmp, sizeof(capTmp)) == 0 && stricmp(strDevName, capTmp.szPname)==0)
				{
					bRet = true;
					m_strDevName = capTmp.szPname;
					break;
				}
			}
			break;
		}
	}

	return bRet;
}

bool CSoundChannel::m_bOpenAudio(const char* strDevName, EDIRECTIONS eDir, const CSoundFormat& sfAudioFormat)
{
	m_bCloseAudio();

	unsigned uID = 0;
	if(!m_bGetDeviceID(strDevName, eDir, uID))
		return false;

	m_sfAudioFormat = sfAudioFormat;
	m_eDir = eDir;
	
	return m_bOpenDevice(uID);
}

bool CSoundChannel::m_bOpenAudio(const char* strDevName, EDIRECTIONS eDir, unsigned uChannel, 
								 unsigned uSampleRate, unsigned uBitPer)
{
	m_bCloseAudio();

	unsigned uID = 0;
	if(!m_bGetDeviceID(strDevName, eDir, uID))
		return false;

	m_sfAudioFormat.m_vSetAudioFormat(uChannel, uSampleRate, uBitPer);
	m_eDir = eDir;

	return m_bOpenDevice(uID);
}

bool CSoundChannel::m_bOpenDevice(unsigned uID)
{
	CWaitAndSignal mutex(m_Mutex);

	m_iBufByteOffset = 0x7fffffff;
	m_iBufIndex = 0;

	WAVEFORMATEX* waveFormat = (WAVEFORMATEX *)m_sfAudioFormat;

	DWORD dwErr = MMSYSERR_BADDEVICEID;
	switch(m_eDir)
	{
	case e_Player:
		//�򿪲����豸
		dwErr = waveOutOpen(&m_hWaveOut, uID, waveFormat, (DWORD)m_hEventDone, NULL, CALLBACK_EVENT);
		break;

	case e_Recorder:
		//��¼���豸
		dwErr = waveInOpen(&m_hWaveIn, uID, waveFormat, (DWORD)m_hEventDone, NULL, CALLBACK_EVENT);
		break;
	}
	if(dwErr != MMSYSERR_NOERROR)
		return false;

	m_uOldHandle = (int)uID;
	m_bClose = false;
	return true;
}

bool CSoundChannel::m_bSetFormat(unsigned uChannel, unsigned uSampleRate, unsigned uBitPer)
{
	//m_bAbort();

	m_bCloseAudio();

	m_sfAudioFormat.m_vSetAudioFormat(uChannel, uSampleRate, uBitPer);

	return true;//m_bOpenDevice(m_uOldHandle);
}

bool CSoundChannel::m_bCloseAudio()
{
	if(m_bClose)
		return m_bClose;
	
	m_bAbort();

	if(m_hWaveOut != NULL)
	{
		while(waveOutClose(m_hWaveOut) == WAVERR_STILLPLAYING)	//�ر�WAVEOUT�豸
			waveOutReset(m_hWaveOut);
		m_hWaveOut = NULL;
	}

	if(m_hWaveIn != NULL)
	{
		while(waveInClose(m_hWaveIn) == WAVERR_STILLPLAYING)	//�ر�WAVEIN�豸
			waveInReset(m_hWaveIn);
		m_hWaveIn = NULL;
	}

	m_bAbort();

	m_uOldHandle = -1;
	m_bClose = true;
	return true;
}

bool CSoundChannel::m_bSetBuffers(int iSize, int iCount)
{
	assert(iSize>0 && iCount>0);

	m_bAbort();
	
	CWaitAndSignal mutex(m_Mutex);
	
	m_vClearBuffers();
	for(int i=0; i<iCount; i++)
	{
		CWaveBuffer* tmpBuffer = new CWaveBuffer;
		tmpBuffer->m_vSetSize(iSize);
		m_Buffers[i] = tmpBuffer;	
	}
	
	m_iBufByteOffset = 0x7fffffff;
	m_iBufIndex = 0;
	return true;
}

bool CSoundChannel::m_bGetBuffers(int& iSize, int& iCount)
{
	CWaitAndSignal mutex(m_Mutex);

	iCount = m_Buffers.size();
	if(iCount == 0)
		iSize = 0;
	else
		iSize = m_Buffers[0]->m_iGetSize();

	return true;
}

bool CSoundChannel::m_bAbort()
{
	DWORD dwErr = MMSYSERR_NOERROR;

	if(m_hWaveIn != NULL)
		dwErr = waveInReset(m_hWaveIn);
	
	if(m_hWaveOut != NULL)
		dwErr = waveOutReset(m_hWaveOut);

	CWaitAndSignal mutex(m_Mutex);
	for(int i=0; i<m_Buffers.size(); i++)
	{
		while(m_Buffers[i]->m_dwRelease() == WAVERR_STILLPLAYING)
		{
			if(m_hWaveIn != NULL)
				dwErr = waveInReset(m_hWaveIn);
			
			if(m_hWaveOut != NULL)
				dwErr = waveOutReset(m_hWaveOut);
		}
	}
	
	m_iBufByteOffset = 0x7fffffff;
	m_iBufIndex = 0;
	m_iPacket = 1;

	SetEvent(m_hEventDone);

	if(dwErr != MMSYSERR_NOERROR)
		return false;
	else
		return true;
}

bool CSoundChannel::m_bWrite(const void* pData, int iSize)
{
	if(pData == NULL || m_hWaveOut == NULL)
		return false;

	m_iLastWriteCount = 0;
	
	const char* pWaveData = (const char*)pData;
	
	m_Mutex.m_vWait();

	DWORD dwErr = MMSYSERR_NOERROR;

	while(iSize > 0)
	{
		CWaveBuffer* buffer = m_Buffers[m_iBufIndex];
		while((buffer->m_header.dwFlags & WHDR_DONE) == 0)
		{
			m_Mutex.m_vSignal();
			//���е�BUFFER��Ԫȫ�����ڲ���״̬
			if(WaitForSingleObject(m_hEventDone, INFINITE) != WAIT_OBJECT_0)
				return false;

			m_Mutex.m_vWait();
		}

		int iCount = iSize;
		if((dwErr = buffer->m_dwPrepare(m_hWaveOut, iCount)) != MMSYSERR_NOERROR)
			break;

		memcpy(buffer->m_pGetPointer(), pWaveData, iSize);
		//���������ݷ����豸��������
		if((dwErr = waveOutWrite(m_hWaveOut, &buffer->m_header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
			break;
		
		m_iBufIndex = (m_iBufIndex + 1)%m_Buffers.size();
		m_iLastWriteCount += iCount;

		iSize -= iCount;
		pWaveData += iCount;
	} //end of while(iSize > 0)

	m_Mutex.m_vSignal();

	if(iSize != 0)
		return false;
	else
		return true;
}

bool CSoundChannel::m_bRead(void* pData, int iSize)
{
	if(pData == NULL || m_hWaveIn == NULL)
		return false;

	m_iLastReadCount = 0;
	if(!m_bWaitForRecordBufFull())
		return false;

	CWaitAndSignal mutex(m_Mutex);
	if(m_iBufByteOffset == 0x7fffffff)
		return false;

	CWaveBuffer* buffer = m_Buffers[m_iBufIndex];
	m_iLastReadCount = buffer->m_header.dwBytesRecorded - m_iBufByteOffset;

	if(m_iLastReadCount > iSize)
		m_iLastReadCount = iSize;

	memcpy(pData, (char *)buffer->m_pGetPointer() + m_iBufByteOffset, m_iLastReadCount);
	m_iBufByteOffset += m_iLastReadCount;
	if(m_iBufByteOffset >= (int)buffer->m_header.dwBytesRecorded)	//������ȡ��,���°�BUFFER��Ԫ���óɿ���
	{
		DWORD dwErr = MMSYSERR_NOERROR;
		if((dwErr = buffer->m_dwPrepare(m_hWaveIn)) != MMSYSERR_NOERROR)
			return false;

		if((dwErr = waveInAddBuffer(m_hWaveIn, &buffer->m_header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
			return false;

		m_iBufIndex = (m_iBufIndex + 1)%m_Buffers.size();
		m_iBufByteOffset = 0;
	}

	return true;
}

bool CSoundChannel::m_bWaitForRecordBufFull()
{
	if(!m_bStartRecording())
		return false;

	while(!m_bIsRecordBufFull())
	{
		if(WaitForSingleObject(m_hEventDone, INFINITE) != WAIT_OBJECT_0)
			return false;
		
		CWaitAndSignal mutex(m_Mutex);
		if(m_iBufByteOffset == 0x7fffffff)
			return false;
	}
	return true;
}

bool CSoundChannel::m_bIsRecordBufFull()
{
	CWaitAndSignal mutex(m_Mutex);

	return (m_Buffers[m_iBufIndex]->m_header.dwFlags & WHDR_DONE)!=0 &&
		m_Buffers[m_iBufIndex]->m_header.dwBytesRecorded > 0;
}

bool CSoundChannel::m_bStartRecording()
{
	CWaitAndSignal mutex(m_Mutex);
	
	if(m_iBufByteOffset != 0x7fffffff)	//�Ѿ���ʼRECORD��
		return true;

	//�����е�BUFFER���뵽RECORD�豸��ȥ�ɼ�����
	for(int i=0; i<m_Buffers.size(); i++)
	{
		CWaveBuffer* buffer = m_Buffers[i];
		if(buffer->m_dwPrepare(m_hWaveIn) != MMSYSERR_NOERROR)
			return false;

		if(waveInAddBuffer(m_hWaveIn, &buffer->m_header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			return false;
	}//end of for
	m_iBufByteOffset = 0;
	
	//��ʼ¼��
	if(waveInStart(m_hWaveIn) == MMSYSERR_NOERROR)
		return true;

	m_iBufByteOffset = 0x7fffffff;
	return false;

}

void CSoundChannel::m_vClearBuffers()
{
	if(m_Buffers.size() > 0)
	{
		CWaveBufferArray::iterator it;
		for(it=m_Buffers.begin(); it!=m_Buffers.end(); it++)
		{
			delete it->second;
		}

		m_Buffers.clear();
	}
}
/********************************************************************************************************************/
