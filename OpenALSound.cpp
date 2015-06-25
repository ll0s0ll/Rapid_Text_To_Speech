/*
OpenALSound.cpp

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

#include "OpenALSound.h"

#include <cstdio>
#include <iostream>

using namespace std;

OpenALSound::OpenALSound()
{
  InitDeviceAndContext();
}


OpenALSound::~OpenALSound()
{
  DeleteDeviceAndContext();
}


OpenALSound* OpenALSound::Instance()
{
  static OpenALSound instance;
  return &instance;
}


//- Device and Context ---------------------------------//

void OpenALSound::InitDeviceAndContext()
{
  //デバイスを開く
  m_pDevice = alcOpenDevice(NULL);
  //コンテキストを生成
  m_pContext = alcCreateContext(m_pDevice, NULL);
  //使用するコンテキストの指定
  alcMakeContextCurrent(m_pContext);
}

void OpenALSound::DeleteDeviceAndContext()
{
  alcMakeContextCurrent(NULL);
  alcDestroyContext(m_pContext);
  alcCloseDevice(m_pDevice);
}


//- Source ---------------------------------------------// 

int OpenALSound::DeleteSource(ALuint source)
{
  alSourceStop(source);

  alDeleteSources(1, &source);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }
  
  DisplayALError(error, "OpenALSound::DeleteSource()");
  
  return -1;
}


ALuint OpenALSound::GenerateSource()
{
  ALuint source;

  alGenSources(1, &source);

  ALenum error;
  if ((error = alGetError()) != AL_NO_ERROR) {
    DisplayALError(error, "OpenALSound::GenerateSource()");
    return 0;
  }

  return source;
}


//- Buffer ---------------------------------------------//

int OpenALSound::AddBufferToQueue(ALuint source, ALuint buffer)
{
  alSourceQueueBuffers(source, 1, &buffer);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(error, "OpenALSound::AddBufferToQueue()");

  return -1;
}


int OpenALSound::DeleteBuffer(ALuint buffer)
{
  alDeleteBuffers(1, &buffer);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(error, "OpenALSound::DeleteBuffer()");

  return -1;
}


int OpenALSound::DeleteBuffers(ALuint* buffers, ALsizei num)
{
  alDeleteBuffers(num, buffers);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(error, "OpenALSound::DeleteBuffers()");

  return -1;
}


int OpenALSound::DetachAllBuffers(ALuint source)
{
  alSourcei(source, AL_BUFFER, 0);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(error, "OpenALSound::DetachAllBuffers()");

  return -1;
}


ALuint OpenALSound::GenerateBuffer()
{
  ALuint buffer;
  alGenBuffers(1, &buffer);

  ALenum error;
  if ((error = alGetError()) != AL_NO_ERROR) { 
    DisplayALError(error, "OpenALSound::GenerateBuffer()");
    return 0;
  }

  return buffer;
}


int OpenALSound::GenerateBuffers(ALuint* buffers, ALsizei num)
{
  alGenBuffers(num, buffers);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(error, "OpenALSound::GenerateBuffers()");

  return -1;
}


int OpenALSound::GetNumOfProcessedBuffer(ALuint source)
{
  ALint val;
  alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
  return val;
}


int OpenALSound::GetNumOfQueuedBuffer(ALuint source)
{
  ALint val;
  alGetSourcei(source, AL_BUFFERS_QUEUED, &val);
  return val;
}


int OpenALSound::LoadPCMToBuffer(FILE* fp, ALuint buffer, ALenum format)
{
  int wavFreq;

  switch(format) {
  case AL_FORMAT_MONO8:
    wavFreq = 8000;
    break;
  case AL_FORMAT_MONO16:
    wavFreq = 16000;
    break;
  case AL_FORMAT_STEREO8:
    wavFreq = 8000;
    break;
  case AL_FORMAT_STEREO16:
    wavFreq = 16000;
    break;
  default:
    cerr << "OpenALSound::LoadPCMToBuffer() Unknown format." << endl;
    return -1;
  }

  // ファイルサイズ取得
  fseek( fp, 0, SEEK_END );
  long size = ftell( fp );

  // 先頭に戻す
  rewind( fp );

  // データを取り出し、バッファに書き込み
  unsigned char* data = new unsigned char[size];
  fread(data, size, 1 ,fp);
  alBufferData(buffer, format, data, size, wavFreq);
  delete data;

  // エラーチェック
  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }
  
  DisplayALError(error, "OpenALSound::LoadPCMToBuffer()");

  return -1;
}


int OpenALSound::UnqueueBuffers(ALuint* buffers, ALsizei num, ALuint source)
{
  alSourceUnqueueBuffers(source, num, buffers);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(source, "OpenALSound::UnqueueBuffers()");

  return -1;
}


//- OPERATION ----------------------------------------------//

int OpenALSound::Pause(ALuint source)
{
  //cout << "OpenALSound::Pause()" << endl;

  alSourcePause(source);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(source, "OpenALSound::Pause()");

  return -1;
}

int OpenALSound::Play(ALuint source, float gain, ALboolean loop)
{
  //cout << "OpenALSound::Play()" << endl;

  alSourcef(source, AL_GAIN, gain);
  alSourcei(source, AL_LOOPING, loop);
  alSourcePlay (source);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(source, "OpenALSound::Play()");

  return -1;
}

int OpenALSound::Stop(ALuint source)
{
  //cout << "OpenALSound::Stop()" << endl;

  alSourceStop(source);

  ALenum error;
  if ((error = alGetError()) == AL_NO_ERROR) { return 0; }

  DisplayALError(source, "OpenALSound::Stop()");

  return -1;
}


//- State ----------------------------------------------//

bool OpenALSound::IsInitialState(ALuint source)
{
  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);

  if (state == AL_INITIAL) return true;

  return false;
}

bool OpenALSound::IsPaused(ALuint source)
{
  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);

  if (state == AL_PAUSED) return true;

  return false;
}

bool OpenALSound::IsPlaying(ALuint source)
{
  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);

  if (state == AL_PLAYING) return true;

  return false;
}

bool OpenALSound::IsStopped(ALuint source)
{
  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);

  if (state == AL_STOPPED) return true;

  return false;
}


//- Util --------------------------------------------------//

void OpenALSound::DisplayALError(ALenum error, const string& comment)
{
  switch (error) {
  case AL_INVALID_NAME:
    cerr << comment << " A bad name (ID) was passed." << endl;
    break;

  case AL_INVALID_ENUM:
    cerr << comment << " An invalid enum value was passed." << endl;
    break;

  case AL_INVALID_VALUE:
    cerr << comment << " An invalid value was passed." << endl;
    break;

  case AL_INVALID_OPERATION:
    cerr << comment << " The requested operation is not valid." << endl;
    break;

  case AL_OUT_OF_MEMORY:
    cerr << comment << " The requested operation resulted in OpenAL running out of memory." << endl;
    break;

  default:
    cerr << comment << " Unknown error state." << endl;
  }
}
