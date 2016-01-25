#include "../HEAAClib/HEAAC_ENC.h"
#include "../HEAAClib/HEAAC_DEC.h"

#include "aac_thread.h"
#include "sound.h"

#include <string>
#include <list>
using namespace std;

#define CHANNELNUM		2
#define SAMPLERATE		48000
#define BITRATE			26000

#ifndef AACENC_BLOCKSIZE
#define AACENC_BLOCKSIZE    1024
#endif


typedef list<string>		DATA_LIST;

DATA_LIST					g_data_list;

RecordThread::RecordThread()
{
	record_count_ = 0;
	bandwidth_ = 0;
}

RecordThread::~RecordThread()
{
	record_count_ = 0;
}

int RecordThread::get_bandwidth(int &record_count, int &bandwidth)
{
	record_count = record_count_;
	record_count_ = 0;
	bandwidth = bandwidth_;
	bandwidth_ = 0;

	return bandwidth;
}


void RecordThread::run()
{
	IHEAAC_ENC* enc = CreateAACEnc();
	enc->Init(SAMPLERATE, CHANNELNUM, BITRATE);
	int read_size = enc->GetSampleCount() * 2;

	CSoundChannel sound_channel;
	int bits = 16;
	int channels = CHANNELNUM;

	sound_channel.m_bOpenAudio("Âó¿Ë·ç (Realtek High Definition", CSoundChannel::e_Recorder, channels, 48000, bits);
	sound_channel.m_bSetBuffers(read_size, 5);
	//sound_channel.m_bStartRecording();

	short raw[6000] = {0};
	while(m_run_flag)
	{
		if(sound_channel.m_bRead(raw, read_size))
		{ 
			string enc_string;
			if(enc->Enc(raw, enc_string))
			{
				bandwidth_ += enc_string.size();
				record_count_ ++;

				g_data_list.push_back(enc_string);
			}
		}
	}


	sound_channel.m_bCloseAudio();
	DestroyAACEnc(enc);

	m_run_flag = true;
}

PlayThread::PlayThread()
{
}

PlayThread::~PlayThread()
{
}

void PlayThread::run()
{
	IHEAAC_DEC* dec = CreateAACDec();
	dec->Init(SAMPLERATE, BITRATE);

	CSoundChannel sound_channel;
	sound_channel.m_bOpenAudio("", CSoundChannel::e_Player, CHANNELNUM, SAMPLERATE, 16);

	int write_size = AACENC_BLOCKSIZE * CHANNELNUM * 4;
	sound_channel.m_bSetBuffers(write_size, 10);

	while(m_run_flag)
	{
		if(!g_data_list.empty())
		{
			string pcmData;
			string encData = g_data_list.front();
			if(dec->Dec(encData, pcmData, true))
			{
				sound_channel.m_bWrite(pcmData.data(), pcmData.size());
			}
			else
			{
				Sleep(1);
			}

			g_data_list.pop_front();
		}
		else
		{
			Sleep(1);
		}
	}

	sound_channel.m_bCloseAudio();
	DestroyAACDec(dec);

	m_run_flag = true;
}



