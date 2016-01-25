/******************************************************************************
作者：YRX
日期：2004-09-14
描述：主要是控制声卡，声音的播放、麦克风数据的采集、音量的设置等
******************************************************************************/
#ifndef __SOUND_HPP
#define __SOUND_HPP

//加入WINDOWS系统的头文件
#include <windows.h>
#include <Mmsystem.h>

#include "Mutex.h"

//加入STL库
#include <string>
#include <vector>
#include <map>
using namespace std;

typedef vector<string> SStringArray;

//声卡格式对象
class CSoundFormat
{
public:
	CSoundFormat();
	~CSoundFormat();

	//拷贝构造函数
	CSoundFormat(const CSoundFormat& sfAudioFormat);
	//=号的重载
	CSoundFormat& operator=(const CSoundFormat& sfAudioFormat);
	//->号的重载
	WAVEFORMATEX* operator->() const;
	//*号的重载
	WAVEFORMATEX& operator*() const;
	
	operator WAVEFORMATEX *() const {return m_pWaveFormat;}

	//SET接口
	void m_vSetAudioFormat(unsigned uChannel, unsigned uSampleRate, unsigned uBitsPer);
	void m_vSetAudioFormat(const void* pData, int iSize);

	bool  m_vSetSize(int iSize);
	int   m_iGetSize() const;

protected:
	int m_iSize;					//格式数据的长度
	WAVEFORMATEX* m_pWaveFormat;	//格式数据	
};

class CSoundChannel;

//声音BUFFER类
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

	char* m_pData;		//声音数据块
	int m_iSize;		//数据块长度

	friend CSoundChannel;
};

typedef map<int, CWaveBuffer*, less<int> > CWaveBufferArray;

//声卡控制类
class CSoundChannel
{
public:
	//是录音还是播发类型
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
	
	//取得所有声卡设备名称
	static SStringArray m_saGetDevNames(EDIRECTIONS eDir);
	//取得默认声卡设备名称
	static string m_sGetDefaultDevice(EDIRECTIONS eDir);
	//打开指定声卡
	bool m_bOpenAudio(const char* strDevName,
					  EDIRECTIONS eDir,
					  unsigned uChannel = 1,
					  unsigned uSampleRate = 8000,
					  unsigned uBitPer = 8);

	bool m_bOpenAudio(const char* strDevName,
					  EDIRECTIONS eDir,
					  const CSoundFormat& sfAudioFormat);
	//关闭打开的声卡
	bool m_bCloseAudio();

	bool m_bSetFormat(unsigned uChannel = 1,
					  unsigned uSampleRate = 8000,
					  unsigned uBitPer = 8);

	bool m_bSetBuffers(int iSize, int iCount = 5);
	bool m_bGetBuffers(int& iSize, int& iCount);
	
	//声音数据的播放与采集接口
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
	//打开一个声卡设备
	bool m_bOpenDevice(unsigned uID);
	//取一个可用声卡的ID
	bool m_bGetDeviceID(const char* strDevName, EDIRECTIONS eDir, unsigned& uID);

	void m_vConstruct();
	bool m_bAbort();

	bool m_bWaitForRecordBufFull();
	bool m_bIsRecordBufFull();
	bool m_bStartRecording();

	//清空m_Buffers
	void m_vClearBuffers();

	unsigned short m_iPacket;	//
};



#endif
