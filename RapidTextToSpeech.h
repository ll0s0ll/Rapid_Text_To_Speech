/*
RapidTextToSpeech.h

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

#ifndef _RAPIDTEXTOSPEECH_H_
#define _RAPIDTEXTOSPEECH_H_

#include <string>
#include <pthread.h>

#include "TextToSpeech.h"

//- OpenJTalkの設定 ---------------------------------------------//

// グローバルパラメータはお好みに、
// ボイスと辞書はご使用の環境に合わせて設定してください。

// OpenJTalk global parameter
// どこかのサイトで公開されていた設定値を参考にさせていただきました。
// しかし、どこのサイトか失念してしまいました。ごめんなさい。
static const int         sampling_rate   = 16000; // Original -> 48000
static const int         fperiod         = 80; // Original -> 240
static const double      alpha           = 0.05; // Original -> 0.5
static const int         stage           = 0;
static const double      beta            = 0.0; // Original -> 0.8
static const int         audio_buff_size = 16000; // Original -> 48000
static const double      uv_threshold    = 0.0; // Original -> 0.5
static const double      gv_weight_mgc   = 1.0;
static const double      gv_weight_lf0   = 1.0;
static const double      gv_weight_lpf   = 1.0;
static const HTS_Boolean use_log_gain    = FALSE;

// ボイスのありか (引数で渡さない場合はこちらを使って）
//static const std::string voiceDir = "/path/to/mei_happy";

// 辞書のありか (引数で渡さない場合はこちらを使って）
///static const std::string dicDir = "/path/to/open_jtalk_dic_utf_8-1.05";

//---------------------------------------------------------------//


class TextToSpeech;

class RapidTextToSpeech
{
 private:

  static pthread_mutex_t mtx;
  
  TextToSpeech *m_pCurrentTTS;
  TextToSpeech *m_pReserveTTS;
  
 public:
  
  RapidTextToSpeech(std::string dicDir, std::string voiceDir);
  virtual ~RapidTextToSpeech();
  
  // Operation
  int  PauseSpeech();
  int  ResumeSpeech();
  int  Speech(const std::string& text);
  int  StopSpeech();

  // CallBack
  static void SpeechEndCallBack(void *pUserData);
  static void SpeechPauseCallBack(void *pUserData);
  static void SpeechResumeCallBack(void *pUserData);
  static void SpeechStartCallBack(void *pUserData);
  void SetSpeechEndCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechPauseCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechResumeCallBack(void (*pFunc)(void*), void *pUserData);
  void SetSpeechStartCallBack(void (*pFunc)(void*), void *pUserData);

};

#endif
