/******************************************************************************
���ߣ�YRX
���ڣ�2004-09-14
��������Ҫ�ǿ��������������Ĳ��š���˷����ݵĲɼ������������õ�
******************************************************************************/
#ifndef __SOUND_HPP
#define __SOUND_HPP

//����WINDOWSϵͳ��ͷ�ļ�
#include <windows.h>
#include <Mmsystem.h>

#include "Mutex.h"

//����STL��
#include <string>
#include <vector>
#include <map>
using namespace std;

typedef vector<string> SStringArray;

//������ʽ����
class CSoundFormat
{
public:
	CSoundFormat();
	~CSoundFormat();

	//�������캯��
	CSoundFormat(const CSoundFormat& sfAudioFormat);
	//=�ŵ�����
	CSoundFormat& operator=(const CSoundFormat& sfAudioFormat);
	//->�ŵ�����
	WAVEFORMATEX* operator->() const;
	//*�ŵ�����
	WAVEFORMATEX& operator*() const;
	
	operator WAVEFORMATEX *() const {return m_pWaveFormat;}

	//SET�ӿ�
	void m_vSetAudioFormat(unsigned uChannel, unsigned uSampleRate, unsigned uBitsPer);
	void m_vSetAudioFormat(const void* pData, int iSize);

	bool  m_vSetSize(int iSize);
	int   m_iGetSize() const;

protected:
	int m_iSize;					//��ʽ���ݵĳ���
	WAVEFORMATEX* m_pWaveFormat;	//��ʽ����	
};

class CSoundChannel;

//����BUFFER��
class CWaveBuffer
{
public:
	CWaveBuffer();
	virtual ~CWaveBuffer();

private:
	DWORD m_dwPrepare(HWAVEOUT hWaveOut, int& iCount);
	DWORD m_dwPrepare(HWAVEIN hWaveIn);
	DWORD m_dwRelease();

	void m_vPrepareCommon(int iCount);

	void m_vSetSize(int iSize);
	int  m_iGetSize();
	void* m_pGetPointer();

	HWAVEOUT m_hWaveOut;
    HWAVEIN  m_hWaveIn;
    WAVEHDR  m_header;

	char* m_pData;		//�������ݿ�
	int m_iSize;		//���ݿ鳤��

	friend CSoundChannel;
};

typedef map<int, CWaveBuffer*, less<int> > CWaveBufferArray;

//����������
class CSoundChannel
{
public:
	//��¼�����ǲ�������
	enum EDIRECTIONS
	{
		e_Player,
		e_Recorder
	};

	CSoundChannel();
	CSoundChannel(const char* pcDevName,
				  EDIRECTIONS eDir,
				  unsigned uChannel = 1,
				  unsigned uSampleRate = 8000,
				  unsigned uBitPer = 8);

	~CSoundChannel();
	
	//ȡ�����������豸����
	static SStringArray m_saGetDevNames(EDIRECTIONS eDir);
	//ȡ��Ĭ�������豸����
	static string m_sGetDefaultDevice(EDIRECTIONS eDir);
	//��ָ������
	bool m_bOpenAudio(const char* strDevName,
					  EDIRECTIONS eDir,
					  unsigned uChannel = 1,
					  unsigned uSampleRate = 8000,
					  unsigned uBitPer = 8);

	bool m_bOpenAudio(const char* strDevName,
					  EDIRECTIONS eDir,
					  const CSoundFormat& sfAudioFormat);
	//�رմ򿪵�����
	bool m_bCloseAudio();

	bool m_bSetFormat(unsigned uChannel = 1,
					  unsigned uSampleRate = 8000,
					  unsigned uBitPer = 8);

	bool m_bSetBuffers(int iSize, int iCount = 5);
	bool m_bGetBuffers(int& iSize, int& iCount);
	
	//�������ݵĲ�����ɼ��ӿ�
	bool m_bWrite(const void* pData, int iSize);
	bool m_bRead(void* pData, int iSize);

	int m_iLastReadCount;
	int m_iLastWriteCount;

protected:
	string m_strDevName;
	EDIRECTIONS m_eDir;

	HWAVEOUT m_hWaveOut;
	HWAVEIN  m_hWaveIn;
	CSoundFormat m_sfAudioFormat;
	HANDLE m_hEventDone;
	
	CWaveBufferArray m_Buffers;
	int m_iBufIndex;
	int m_iBufByteOffset;
	
	CMutex m_Mutex;

	bool m_bClose;

	int m_uOldHandle;
private:
	//��һ�������豸
	bool m_bOpenDevice(unsigned uID);
	//ȡһ������������ID
	bool m_bGetDeviceID(const char* strDevName, EDIRECTIONS eDir, unsigned& uID);

	void m_vConstruct();
	bool m_bAbort();

	bool m_bWaitForRecordBufFull();
	bool m_bIsRecordBufFull();
	bool m_bStartRecording();

	//���m_Buffers
	void m_vClearBuffers();

	unsigned short m_iPacket;	//
};



#endif
