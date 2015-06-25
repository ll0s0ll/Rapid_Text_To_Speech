/*
TextToSpeech.h

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

#ifndef _TEXTTOSPEECH_H_
#define _TEXTTOSPEECH_H_

#include <list>
#include <iostream>
#include <string>
#include "pthread.h"

#include "HTS_engine.h"
#include "jpcommon.h"
#include "mecab.h"
#include "njd.h"

// 速度計測
//#define TIME_DEBUG

class TextToSpeech
{
 private:

  //- 各種設定項目。お好みで設定してください。------//
  
  // 保持するバッファの数。
  // 少ないと発話が止まったり、とぎれとぎれになります。
  // 多いとメモリを多く消費します。
  static const int    NUM_BUFFERS = 3;

  // 一度に処理するラベル数。
  // 値を大きくすると、読み上げの途切れが減って、なめらかな発話になります。
  // しかし、処理に時間がかかるようになるので、発話するまでの時間が増えたり、
  // 発話が止まってしまうことがあります。
  static const int    NUM_LABELS   = 50;

  // 一度に処理する文字数の上限値。
  // バイト単位なので、すべて全角だと半分の文字数になります。
  static const size_t MAXBUFLEN   = 2048;

  //-----------------------------------------------//

  // Thread
  pthread_t m_thread;

  // OpenAL
  std::list<unsigned int> m_BufferList;
  unsigned int      m_Buffers[NUM_BUFFERS];
  int               m_LabelSize;
  int               m_Pos;
  unsigned int      m_Source;
  
  // OpenJTalk
  struct OpenJTalk {
    Mecab mecab;
    NJD njd;
    JPCommon jpcommon;
    HTS_Engine engine;
  } m_OJT; 

  // CallBack
  void (*m_pSpeechEndCallBack)(void*);
  void *m_pSpeechEndCallBackUserData;

  void (*m_pSpeechPauseCallBack)(void*);
  void *m_pSpeechPauseCallBackUserData;

  void (*m_pSpeechResumeCallBack)(void*);
  void *m_pSpeechResumeCallBackUserData;

  void (*m_pSpeechStartCallBack)(void*);
  void *m_pSpeechStartCallBackUserData;

#ifdef TIME_DEBUG
  clock_t m_StartTime;
#endif

 private:

  static void *Process(void *arg);

  void Initialize(OpenJTalk* pOJT, int sampling_rate, int fperiod, double alpha,
		  int stage, double beta, int audio_buff_size, double uv_threshold,
		  HTS_Boolean use_log_gain, double gv_weight_mgc, double gv_weight_lf0,
		  double gv_weight_lpf);

  void Load(OpenJTalk* pOJT, const std::string& dn_mecab_str, const std::string& dur_pdf_fn_str,
	    const std::string& dur_tree_fn_str, const std::string& mgc_pdf_fn_str,
	    const std::string& mgc_tree_fn_str, const std::string& mgc_win1_fn_str,
	    const std::string& mgc_win2_fn_str, const std::string& mgc_win3_fn_str,
	    const std::string& lf0_pdf_fn_str,  const std::string& lf0_tree_fn_str,
	    const std::string& lf0_win1_fn_str, const std::string& lf0_win2_fn_str,
	    const std::string& lf0_win3_fn_str, const std::string& gvmgc_pdf_fn_str,
	    const std::string& gvmgc_tree_fn_str, const std::string& gvlf0_pdf_fn_str,
	    const std::string& gvlf0_tree_fn_str, const std::string& gvswitch_fn_str);

  int  Analyze(OpenJTalk* pOJT, const std::string& text);
  int  FreeProcessedBuffer(unsigned int source);
  void Synthesis(OpenJTalk* pOJT, int pos, int size, FILE *fp);

 public:

  TextToSpeech(int sampling_rate, int fperiod, double alpha, int stage,
	       double beta, int audio_buff_size, double uv_threshold,
	       double gv_weight_mgc, double gv_weight_lf0, double gv_weight_lpf,
	       HTS_Boolean use_log_gain, std::string voiceDir, std::string dic_dir);
  virtual ~TextToSpeech();

  // Operation
  int  PauseSpeech();
  int  ResumeSpeech();
  int  Speech(const std::string& text);
  int  StopSpeech();

  // CallBack
  void SetSpeechEndCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechPauseCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechResumeCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechStartCallBack(void (*pFunc)(void*), void *pUserData);
};

#endif
