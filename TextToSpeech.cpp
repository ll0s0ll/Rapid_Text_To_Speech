/*
TextToSpeech.cpp

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

#include "TextToSpeech.h"

#include "OpenALSound.h"

#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

using namespace std;

TextToSpeech::TextToSpeech(int sampling_rate, int fperiod, double alpha, int stage,
			   double beta, int audio_buff_size, double uv_threshold,
			   double gv_weight_mgc, double gv_weight_lf0, double gv_weight_lpf,
			   HTS_Boolean use_log_gain, string voiceDir, string dic_dir)
{
  // OpenALを初期化しておく。
  OpenALSound::Instance();

  // ソースを作成。
  if ((m_Source = OpenALSound::Instance()->GenerateSource()) == 0) {
    cerr << "Error GenerateSource()" << endl;
    exit(1);
  } 

  // NUM_BUFFERSの数だけバッファを作成。
  // 繰り返し使用するので、m_BufferListに格納して、
  // 使用する時に取り出し、使用後は戻すようにする。
  OpenALSound::Instance()->GenerateBuffers(m_Buffers, NUM_BUFFERS);
  int i;
  for (i=0; i<NUM_BUFFERS; ++i) {
    m_BufferList.push_front(m_Buffers[i]);
  }

  // dictionary
  string dn_mecab_str = dic_dir;

  // Duration
  string dur_pdf_fn_str  = voiceDir + "/dur.pdf";
  string dur_tree_fn_str = voiceDir + "/tree-dur.inf";

  // MGC
  string mgc_pdf_fn_str  = voiceDir + "/mgc.pdf";
  string mgc_tree_fn_str = voiceDir + "/tree-mgc.inf";

  // Window
  string mgc_win1_fn_str = voiceDir + "/mgc.win1";
  string mgc_win2_fn_str = voiceDir + "/mgc.win2";
  string mgc_win3_fn_str = voiceDir + "/mgc.win3";

  // lf0
  string lf0_pdf_fn_str  = voiceDir + "/lf0.pdf";
  string lf0_tree_fn_str = voiceDir + "/tree-lf0.inf";
  string lf0_win1_fn_str = voiceDir + "/lf0.win1";
  string lf0_win2_fn_str = voiceDir + "/lf0.win2";
  string lf0_win3_fn_str = voiceDir + "/lf0.win3";

  // GVMGC
  string gvmgc_pdf_fn_str  = voiceDir + "/gv-mgc.pdf";
  string gvmgc_tree_fn_str = voiceDir + "/tree-gv-mgc.inf";

  //GV-lf0
  string gvlf0_pdf_fn_str  = voiceDir + "/gv-lf0.pdf";
  string gvlf0_tree_fn_str = voiceDir + "/tree-gv-lf0.inf";

  //GV-Switch
  string gvswitch_fn_str   = voiceDir + "/gv-switch.inf";


  // OpenJTalkを設定
  Initialize(&m_OJT, sampling_rate, fperiod, alpha, stage, beta, audio_buff_size,
	     	     uv_threshold, use_log_gain, gv_weight_mgc, gv_weight_lf0, gv_weight_lpf);

  Load(&m_OJT,            dn_mecab_str,     dur_pdf_fn_str,    dur_tree_fn_str, mgc_pdf_fn_str,
       mgc_tree_fn_str,   mgc_win1_fn_str,  mgc_win2_fn_str,   mgc_win3_fn_str, lf0_pdf_fn_str,
       lf0_tree_fn_str,   lf0_win1_fn_str,  lf0_win2_fn_str,   lf0_win3_fn_str, gvmgc_pdf_fn_str,
       gvmgc_tree_fn_str, gvlf0_pdf_fn_str, gvlf0_tree_fn_str, gvswitch_fn_str);
}


TextToSpeech::~TextToSpeech()
{
  // OpenAL
  // On Source
  StopSpeech();
  FreeProcessedBuffer(m_Source);
  // Buffers
  m_BufferList.clear();
  OpenALSound::Instance()->DeleteBuffers(m_Buffers, NUM_BUFFERS);
  OpenALSound::Instance()->DeleteSource(m_Source);
  
  // OpenJTalk
  Mecab_clear(&m_OJT.mecab);
  NJD_clear(&m_OJT.njd);
  JPCommon_clear(&m_OJT.jpcommon);
  HTS_Engine_clear(&m_OJT.engine);
}


void TextToSpeech::Initialize(OpenJTalk* pOJT, int sampling_rate, int fperiod, double alpha,
			      int stage, double beta, int audio_buff_size,
			      double uv_threshold, HTS_Boolean use_log_gain,
			      double gv_weight_mgc, double gv_weight_lf0, double gv_weight_lpf)
{
  Mecab_initialize(&pOJT->mecab);
  NJD_initialize(&pOJT->njd);
  JPCommon_initialize(&pOJT->jpcommon);
  HTS_Engine_initialize(&pOJT->engine, 2);
  HTS_Engine_set_sampling_rate(&pOJT->engine, sampling_rate);
  HTS_Engine_set_fperiod(&pOJT->engine, fperiod);
  HTS_Engine_set_alpha(&pOJT->engine, alpha);
  HTS_Engine_set_gamma(&pOJT->engine, stage);
  HTS_Engine_set_log_gain(&pOJT->engine, use_log_gain);
  HTS_Engine_set_beta(&pOJT->engine, beta);
  HTS_Engine_set_audio_buff_size(&pOJT->engine, audio_buff_size);
  HTS_Engine_set_msd_threshold(&pOJT->engine, 1, uv_threshold);
  HTS_Engine_set_gv_weight(&pOJT->engine, 0, gv_weight_mgc);
  HTS_Engine_set_gv_weight(&pOJT->engine, 1, gv_weight_lf0);
}


void TextToSpeech::Load(OpenJTalk* pOJT, const string& dn_mecab_str, const string& dur_pdf_fn_str,
			const string& dur_tree_fn_str, const string& mgc_pdf_fn_str,
			const string& mgc_tree_fn_str, const string& mgc_win1_fn_str,
			const string& mgc_win2_fn_str, const string& mgc_win3_fn_str,
			const string& lf0_pdf_fn_str,  const string& lf0_tree_fn_str,
			const string& lf0_win1_fn_str, const string& lf0_win2_fn_str,
			const string& lf0_win3_fn_str, const string& gvmgc_pdf_fn_str,
			const string& gvmgc_tree_fn_str, const string& gvlf0_pdf_fn_str,
			const string& gvlf0_tree_fn_str, const string& gvswitch_fn_str)
{

  // dictionary
  char dn_mecab[dn_mecab_str.size()+1];
  strcpy(dn_mecab, dn_mecab_str.c_str());
  Mecab_load(&pOJT->mecab, dn_mecab);

  // Duration
  // PDF 
  char dur_pdf_fn[dur_pdf_fn_str.size() + 1];
  strcpy(dur_pdf_fn, dur_pdf_fn_str.c_str());
  char* dur_pdf_fns[] = {dur_pdf_fn};

  // Tree
  char dur_tree_fn[dur_tree_fn_str.size() + 1];
  strcpy(dur_tree_fn, dur_tree_fn_str.c_str());
  char* dur_tree_fns[] = {dur_tree_fn};
 
  HTS_Engine_load_duration_from_fn(&pOJT->engine, dur_pdf_fns, dur_tree_fns, 1);


  // MGC
  // PDF
  char mgc_pdf_fn[mgc_pdf_fn_str.size() + 1];
  strcpy(mgc_pdf_fn, mgc_pdf_fn_str.c_str());
  char* mgc_pdf_fns[] = {mgc_pdf_fn};

  // Tree
  char mgc_tree_fn[mgc_tree_fn_str.size() + 1];
  strcpy(mgc_tree_fn, mgc_tree_fn_str.c_str());
  char* mgc_tree_fns[] = {mgc_tree_fn};

  // Window
  char mgc_win1_fn[mgc_win1_fn_str.size() + 1];
  strcpy(mgc_win1_fn, mgc_win1_fn_str.c_str());

  char mgc_win2_fn[mgc_win2_fn_str.size() + 1];
  strcpy(mgc_win2_fn, mgc_win2_fn_str.c_str());

  char mgc_win3_fn[mgc_win3_fn_str.size() + 1];
  strcpy(mgc_win3_fn, mgc_win3_fn_str.c_str());

  char* mgc_win_fns[] = {mgc_win1_fn, mgc_win2_fn, mgc_win3_fn};
  
  int mgc_win_num = 3;

  HTS_Engine_load_parameter_from_fn(&pOJT->engine, mgc_pdf_fns, mgc_tree_fns,
				    mgc_win_fns, 0, FALSE, mgc_win_num, 1);

  

  // lf0
  // PDF
  char lf0_pdf_fn[lf0_pdf_fn_str.size() + 1];
  strcpy(lf0_pdf_fn, lf0_pdf_fn_str.c_str());
  char* lf0_pdf_fns[] = {lf0_pdf_fn};
  
  // Tree
  char lf0_tree_fn[lf0_tree_fn_str.size() + 1];
  strcpy(lf0_tree_fn, lf0_tree_fn_str.c_str());
  char* lf0_tree_fns[] = {lf0_tree_fn};

  // Window
  char lf0_win1_fn[lf0_win1_fn_str.size() + 1];
  strcpy(lf0_win1_fn, lf0_win1_fn_str.c_str());

  char lf0_win2_fn[lf0_win2_fn_str.size() + 1];
  strcpy(lf0_win2_fn, lf0_win2_fn_str.c_str());

  char lf0_win3_fn[lf0_win3_fn_str.size() + 1];
  strcpy(lf0_win3_fn, lf0_win3_fn_str.c_str());

  char* lf0_win_fns[] = {lf0_win1_fn, lf0_win2_fn, lf0_win3_fn};

  int lf0_win_num = 3;

  HTS_Engine_load_parameter_from_fn(&pOJT->engine, lf0_pdf_fns, lf0_tree_fns,
				    lf0_win_fns, 1, TRUE, lf0_win_num, 1);


  //lpf
  /*char* lpf_pdf_fns[0];
  char* lpf_tree_fns[0];
  char* lpf_win_fns[0];
  int lpf_win_num = 0;

  if (HTS_Engine_get_nstream(&pOJT->engine) == 3)
    HTS_Engine_load_parameter_from_fn(&pOJT->engine, lpf_pdf_fns, lpf_tree_fns,
				      lpf_win_fns, 2, FALSE, lpf_win_num, 1);
  */


  // GVMGC
  char gvmgc_pdf_fn[gvmgc_pdf_fn_str.size() + 1];
  strcpy(gvmgc_pdf_fn, gvmgc_pdf_fn_str.c_str());
  char* gvmgc_pdf_fns[] = {gvmgc_pdf_fn};

  char gvmgc_tree_fn[gvmgc_tree_fn_str.size() + 1];
  strcpy(gvmgc_tree_fn, gvmgc_tree_fn_str.c_str());
  char* gvmgc_tree_fns[] = {gvmgc_tree_fn};

  HTS_Engine_load_gv_from_fn(&pOJT->engine, gvmgc_pdf_fns, gvmgc_tree_fns, 0, 1);


  // GVL
  /*char* gvl_pdf_fns[0];
  char* gvl_tree_fns[0];
  HTS_Engine_load_gv_from_fn(&pOJT->engine, gvl_pdf_fns, gvl_tree_fns, 1, 1);*/


  //GV-lf0
  char gvlf0_pdf_fn[gvlf0_pdf_fn_str.size() + 1];
  strcpy(gvlf0_pdf_fn, gvlf0_pdf_fn_str.c_str());
  char* gvlf0_pdf_fns[] = {gvlf0_pdf_fn};

  char gvlf0_tree_fn[gvlf0_tree_fn_str.size() + 1];
  strcpy(gvlf0_tree_fn, gvlf0_tree_fn_str.c_str());
  char* gvlf0_tree_fns[] = {gvlf0_tree_fn};

  if (HTS_Engine_get_nstream(&pOJT->engine) == 3) {
    HTS_Engine_load_gv_from_fn(&pOJT->engine, gvlf0_pdf_fns, gvlf0_tree_fns, 2, 1);
  }


  //GV-Switch
  char gvswitch_fn[gvswitch_fn_str.size() + 1];
  strcpy(gvswitch_fn, gvswitch_fn_str.c_str());

  HTS_Engine_load_gv_switch_from_fn(&pOJT->engine, gvswitch_fn);

}

int TextToSpeech::Analyze(OpenJTalk* pOJT, const string& text)
{
  // 文字数チェック
  if (strlen(text.c_str()) > MAXBUFLEN) {
    cerr << "[ERROR] テキストが長すぎます。MAXBUFLENの設定を増やすか、テキストを短くしてください。" << endl;
    return -1;
  }
 
  char buff[MAXBUFLEN];

  // 既存データをクリア
  JPCommon_refresh(&pOJT->jpcommon);

  text2mecab(buff, text.c_str());
  Mecab_analysis(&pOJT->mecab, buff);
  mecab2njd(&pOJT->njd, Mecab_get_feature(&pOJT->mecab), Mecab_get_size(&pOJT->mecab));
  Mecab_refresh(&pOJT->mecab);

  njd_set_pronunciation(&pOJT->njd);
  njd_set_digit(&pOJT->njd);
  njd_set_accent_phrase(&pOJT->njd);
  njd_set_accent_type(&pOJT->njd);
  njd_set_unvoiced_vowel(&pOJT->njd);
  njd_set_long_vowel(&pOJT->njd);
  njd2jpcommon(&pOJT->jpcommon, &pOJT->njd);
  NJD_refresh(&pOJT->njd);

  JPCommon_make_label(&pOJT->jpcommon);

  return JPCommon_get_label_size(&pOJT->jpcommon);
}


int TextToSpeech::FreeProcessedBuffer(ALuint source)
{
  // Sourceに残っている、再生済みバッファを削除する。
    
  // Processedバッファの数を取得
  int numOfProcessedBuffer = OpenALSound::Instance()->GetNumOfProcessedBuffer(source);
  //cout << numOfProcessedBuffer << " Buffer(s) Processed." << endl;
  if (numOfProcessedBuffer <= 0) { return 0; }
    
  // 再生キューからバッファを取り除く
  ALuint buffers[numOfProcessedBuffer];
  int r = OpenALSound::Instance()->UnqueueBuffers(buffers, numOfProcessedBuffer, source);
  if (r == -1) { return -1; }

  // 取り除いたバッファは再利用する。
  int i;
  for (i=0; i<numOfProcessedBuffer; ++i) {
    m_BufferList.push_back(buffers[i]);  
  }
  
  //cout << numOfProcessedBuffer << " Buffer(s) Deleted." << endl;

  return 0;
}

void TextToSpeech::Synthesis(OpenJTalk* pOJT, int pos, int size, FILE *fp)
{
  //cout << "RapidTextToSpeech::Synthesis()" << endl;

  char** labels = JPCommon_get_label_feature(&pOJT->jpcommon);

  HTS_Engine_load_label_from_string_list(&pOJT->engine, labels+pos, size);

  HTS_Engine_create_sstream(&pOJT->engine);
  HTS_Engine_create_pstream(&pOJT->engine);
  HTS_Engine_create_gstream(&pOJT->engine);

  HTS_Engine_save_generated_speech(&pOJT->engine, fp);
  
  HTS_Engine_refresh(&pOJT->engine);  
}

void *TextToSpeech::Process(void *arg)
{
  int r;
  TextToSpeech *pTTS = static_cast<TextToSpeech*>(arg);

  // スピーチが始まったことをコールバックする。
  if (pTTS->m_pSpeechStartCallBack != NULL) {
    pTTS->m_pSpeechStartCallBack(pTTS->m_pSpeechStartCallBackUserData);
  }

  // 読み込み位置を初期化
  pTTS->m_Pos = 0;

  while (1) {
    
    // 未処理のラベルがあり、バッファが空いている場合は、処理する
    if (pTTS->m_Pos < pTTS->m_LabelSize && !pTTS->m_BufferList.empty()) {
	
      // 処理するラベル数を決める
      int size;
      if (pTTS->m_Pos + NUM_LABELS <= pTTS->m_LabelSize) {
	size = NUM_LABELS;
      } else {
	size = pTTS->m_LabelSize - pTTS->m_Pos;
      }

      // ラベルをPCMに変換して一時ファイルに書き込む
      FILE *pTmpFile = tmpfile();
      pTTS->Synthesis(&pTTS->m_OJT, pTTS->m_Pos, size, pTmpFile);

      // 一時ファイルに書き込んだPCMをバッファに書き込む
      ALuint buffer = pTTS->m_BufferList.front();
      // 16Bit MONO 16000Hz
      r = OpenALSound::Instance()->LoadPCMToBuffer(pTmpFile, buffer, AL_FORMAT_MONO16); 
      if (r != 0) { break; }

      // 再生キューにバッファを追加する。
      r = OpenALSound::Instance()->AddBufferToQueue(pTTS->m_Source, buffer);
      if (r != 0) { break; }

      // 再生
      if (!OpenALSound::Instance()->IsPlaying(pTTS->m_Source) &&
	  !OpenALSound::Instance()->IsPaused(pTTS->m_Source)) {
	r = OpenALSound::Instance()->Play(pTTS->m_Source, 1.0, AL_FALSE);
	if (r != 0) { break; }

#ifdef TIME_DEBUG
	clock_t current = clock();
	cout << "[START -> Play()] " << (double)(current - pTTS->m_StartTime) / CLOCKS_PER_SEC << "sec.\n";
	//pTTS->m_StartTime = current;
#endif

      }

      // 使用したバッファ数をリストから減らす
      pTTS->m_BufferList.pop_front();

      // 後処理
      fclose(pTmpFile);

      // 処理したラベルの数だけ、ポジションを進める。
      pTTS->m_Pos += size;
    }

    // 再生済みバッファを解放する。
    r = pTTS->FreeProcessedBuffer(pTTS->m_Source);
    if (r != 0) { break; }
    
    // キューされているバッファの数を取得
    int numOfQueuedBuffer = OpenALSound::Instance()->GetNumOfQueuedBuffer(pTTS->m_Source);
    //cout << numOfQueuedBuffer << " Buffer(s) Queued." << endl;
    //int numOfProcessedBuffer = OpenALSound::Instance()->GetNumOfProcessedBuffer(pTTS->m_Source);
    //cout << numOfProcessedBuffer << " Buffer(s) Processed." << endl;
    
    if (pTTS->m_Pos == pTTS->m_LabelSize && numOfQueuedBuffer == 0) {
      // 処理終了
      break;
    } 
    
    // インターバルなしでループすると、再生済みバッファの取得ができないので、
    // 適度にインターバルを入れる。
    struct timespec ts = {0, 300 * 1000000}; //300msec
    nanosleep(&ts, NULL);
    
  } // while

  // スピーチが終わったことをコールバックする。
  if (pTTS->m_pSpeechEndCallBack != NULL) {
    pTTS->m_pSpeechEndCallBack(pTTS->m_pSpeechEndCallBackUserData);
  }

  // 次回に備えてクリーンアップする
  JPCommon_refresh(&(pTTS->m_OJT.jpcommon));
  
  return 0;
}


int TextToSpeech::Speech(const string& text)
{

#ifdef TIME_DEBUG
  m_StartTime = clock();
#endif

  // 渡されたテキストから、ラベルを作成。
  m_LabelSize = Analyze(&m_OJT, text);
  if (m_LabelSize < 2) {
    cerr << "TextToSpeech::Speech() ERROR. Analyze()" << endl;
    return -1;
  }

#ifdef TIME_DEBUG
  clock_t current = clock();
  cout << "[START -> Analyze()] " << (double)(current - m_StartTime) / CLOCKS_PER_SEC << "sec.\n";
  //m_StartTime = current;
#endif

  // スレッドを作成して処理する。
  int s = pthread_create(&m_thread, NULL, Process, this);
  if (s != 0) { cerr << "TextToSpeech::Speech() ERROR. pthread_create" << endl; return -1;}

  s = pthread_detach(m_thread);
  if (s != 0) { cerr << "TextToSpeech::Speech() ERROR. pthread_detach" << endl; return -1;}

  return 0;
}


void TextToSpeech::SetSpeechEndCallBack(void (*pFunc)(void*), void *pUserData)
{
  m_pSpeechEndCallBack = pFunc;
  m_pSpeechEndCallBackUserData = pUserData;
}

void TextToSpeech::SetSpeechPauseCallBack(void (*pFunc)(void*), void *pUserData)
{
  m_pSpeechPauseCallBack = pFunc;
  m_pSpeechPauseCallBackUserData = pUserData;
}


void TextToSpeech::SetSpeechResumeCallBack(void (*pFunc)(void*), void *pUserData)
{
  m_pSpeechResumeCallBack = pFunc;
  m_pSpeechResumeCallBackUserData = pUserData;
}


void TextToSpeech::SetSpeechStartCallBack(void (*pFunc)(void*), void *pUserData)
{
  m_pSpeechStartCallBack = pFunc;
  m_pSpeechStartCallBackUserData = pUserData;
}


int TextToSpeech::PauseSpeech()
{
  // すでに一時停止中の場合は、何もしない。
  if (OpenALSound::Instance()->IsPaused(m_Source) || 
      OpenALSound::Instance()->IsStopped(m_Source)) { return 0; }

  int r = OpenALSound::Instance()->Pause(m_Source);
  if (r != 0) return r;

  // スピーチが一時停止したことをコールバックする。
  if (m_pSpeechPauseCallBack != NULL) {
    m_pSpeechPauseCallBack(m_pSpeechPauseCallBackUserData);
  }

  return 0;
}


int TextToSpeech::ResumeSpeech()
{
  // 一時停止していない場合は、何もしない。
  if (!OpenALSound::Instance()->IsPaused(m_Source)) { return 0; }

  int r = OpenALSound::Instance()->Play(m_Source, 1.0, AL_FALSE);
  if (r != 0) return r;

  // スピーチが再開したことをコールバックする。
  if (m_pSpeechResumeCallBack != NULL) {
    m_pSpeechResumeCallBack(m_pSpeechResumeCallBackUserData);
  }

  return 0;
}


int TextToSpeech::StopSpeech()
{
  
  if (OpenALSound::Instance()->IsStopped(m_Source)) { return 0; }

  // 再生停止
  OpenALSound::Instance()->Stop(m_Source);
    
  // 再生が停止するまで少し待つ
  struct timespec ts = {0, 50 * 1000000}; // 50msec
  nanosleep(&ts, NULL);

  // テキストを最後まで読み込んだことにする。
  m_Pos = m_LabelSize;

  return 0;
}
