/*
RapidTextToSpeech.cpp

Copyright (C) 2015 Shun ITO <movingentity@gmail.com>
	      
This file is part of RapidTextToSpeech.
RapidTextToSpeech is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

RapidTextToSpeech is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RapidTextToSpeech.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "RapidTextToSpeech.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

using namespace std;


pthread_mutex_t RapidTextToSpeech::mtx = PTHREAD_MUTEX_INITIALIZER;


RapidTextToSpeech::RapidTextToSpeech(string dicDir, string voiceDir)
{
  m_pCurrentTTS = new TextToSpeech(sampling_rate, fperiod, alpha, stage,
				   beta, audio_buff_size, uv_threshold,
				   gv_weight_mgc, gv_weight_lf0, gv_weight_lpf,
				   use_log_gain, voiceDir, dicDir);
  m_pReserveTTS = new TextToSpeech(sampling_rate, fperiod, alpha, stage,
				   beta, audio_buff_size, uv_threshold,
				   gv_weight_mgc, gv_weight_lf0, gv_weight_lpf,
				   use_log_gain, voiceDir, dicDir);
}


RapidTextToSpeech::~RapidTextToSpeech()
{
  delete m_pCurrentTTS;
  delete m_pReserveTTS;
}


//- Operation -------------------------------------------------------------------------//

int RapidTextToSpeech::PauseSpeech()
{
  return m_pCurrentTTS->PauseSpeech();
}


int RapidTextToSpeech::ResumeSpeech()
{
  return m_pCurrentTTS->ResumeSpeech();
}


int RapidTextToSpeech::Speech(const string& text)
{  
  int r;

  // 処理が終わるまで連続で次の呼び出しの処理を行わないようにする。
  r = pthread_mutex_lock(&mtx);
  if (r !=0) {
    cerr << "ERROR: RapidTextToSpeech::Speech() pthread_mutex_lock" << endl;
    return -1;
  }

  // 現在再生中のスピーチを停止
  r = m_pCurrentTTS->StopSpeech();
  if (r !=0) {
    cerr << "ERROR: RapidTextToSpeech::Speech() StopSpeech()" << endl;
    return -1;
  }
  
  // 2番手のTextToSpeechを使って読み上げ開始
  r = m_pReserveTTS->Speech(text);
  if (r !=0) {
    cerr << "ERROR: RapidTextToSpeech::Speech() Speech()" << endl;
    return -1;
  }
  
  // 新しく読み上げを開始したTextoToSpeechを、現在のTextToSpeechに設定。
  // 読み上げが終わったTextToSpeechは、2番手に設定。次の読み上げを待つ。
  TextToSpeech *tmp = m_pCurrentTTS;
  m_pCurrentTTS = m_pReserveTTS;
  m_pReserveTTS = tmp;

  // アンロック
  r = pthread_mutex_unlock(&mtx);
  if (r !=0) {
    cout << "ERROR: RapidTextToSpeech::Speech() pthread_mutex_unlock" << endl;
    m_pCurrentTTS->StopSpeech();
    return -1;
  }

  return 0;
}


int RapidTextToSpeech::StopSpeech()
{
  return m_pCurrentTTS->StopSpeech();
}


//- CallBack --------------------------------------------------------------------------//

void RapidTextToSpeech::SetSpeechEndCallBack(void (*pFunc)(void*), void *pUserData)
{  
  m_pCurrentTTS->SetSpeechEndCallBack(pFunc, pUserData);
  m_pReserveTTS->SetSpeechEndCallBack(pFunc, pUserData);
}


void RapidTextToSpeech::SetSpeechPauseCallBack(void (*pFunc)(void*), void *pUserData)
{  
  m_pCurrentTTS->SetSpeechPauseCallBack(pFunc, pUserData);
  m_pReserveTTS->SetSpeechPauseCallBack(pFunc, pUserData);
}


void RapidTextToSpeech::SetSpeechResumeCallBack(void (*pFunc)(void*), void *pUserData)
{  
  m_pCurrentTTS->SetSpeechResumeCallBack(pFunc, pUserData);
  m_pReserveTTS->SetSpeechResumeCallBack(pFunc, pUserData);
}


void RapidTextToSpeech::SetSpeechStartCallBack(void (*pFunc)(void*), void *pUserData)
{  
  m_pCurrentTTS->SetSpeechStartCallBack(pFunc, pUserData);
  m_pReserveTTS->SetSpeechStartCallBack(pFunc, pUserData);
}


void RapidTextToSpeech::SpeechEndCallBack(void *pUserData)
{
  // beep x2
  cout << "\007" << "\007" << flush;
}


void RapidTextToSpeech::SpeechPauseCallBack(void *pUserData)
{
  // beep
  cout << "\007" << flush;
}


void RapidTextToSpeech::SpeechResumeCallBack(void *pUserData)
{
  // beep
  cout << "\007" << flush;
}


void RapidTextToSpeech::SpeechStartCallBack(void *pUserData)
{
  // beep
  cout << "\007" << flush;
}


int main(int argc, char *argv[])
{

  int opt;
  char *pStr;
  string dicDir;
  string voiceDir;

  // 引数解析
  while ((opt = getopt(argc, argv, "d:v:")) != -1) {
    switch (opt) {
    case 'd':
      {
	// 辞書データ
	pStr = optarg;
	
	if (pStr !=  NULL) {

	  struct stat sb;
	  if (stat(pStr, &sb) == -1) {
	    cout << "Not found such a directory. (" << pStr << ")" << endl;
	    exit(-1);
	  }	  
	  dicDir = pStr;

	} else {
	  cout << "Unknow option argument" << endl;
	  exit(-1);
	}
	break;
      }
    case 'v':
      {
	// ボイスデータ
	pStr = optarg;
	
	if (pStr !=  NULL) {

	  struct stat sb;
	  if (stat(pStr, &sb) == -1) {
	    cout << "Not found such a directory. (" << pStr << ")" << endl;
	    exit(-1);
	  }	  
	  voiceDir = pStr;

	} else {
	  cout << "Unknow option argument" << endl;
	  exit(-1);
	}
	break;
      }
    case ':': ; // 引数が足りない
    case '?': ; // 不明なオプション
    default: exit(-1);
    }    
  }

  if (dicDir.empty()) {
    cout << "エラー 辞書データのあるディレクトリが見つかりません。-d オプションで指定してください。" << endl;
    exit(-1);  
  }

  if (voiceDir.empty()) {
    cout << "エラー ボイスデータのあるディレクトリが見つかりません。-v オプションで指定してください。" << endl;
    exit(-1);  
  }


  RapidTextToSpeech *pRtts = new RapidTextToSpeech(dicDir, voiceDir);
  pRtts->SetSpeechEndCallBack(pRtts->SpeechEndCallBack, NULL);
  pRtts->SetSpeechPauseCallBack(pRtts->SpeechPauseCallBack, NULL);
  pRtts->SetSpeechResumeCallBack(pRtts->SpeechResumeCallBack, NULL);
  pRtts->SetSpeechStartCallBack(pRtts->SpeechStartCallBack, NULL);

  string in = "";

  while (1) {

    cout << ">" << flush;
    cin >> in;
    
    if (in == "exit") {
      break;
    } else if (in == "pause") {
      pRtts->PauseSpeech();
    } else if (in == "resume") {
      pRtts->ResumeSpeech();
    } else if (in == "stop") {
      pRtts->StopSpeech();
    } else {
      int r = pRtts->Speech(in);
      if (r != 0 ) { cerr << "ERROR. Speech()" << endl; break; }
    }
  }
  
  delete pRtts;

  return 0;
}
