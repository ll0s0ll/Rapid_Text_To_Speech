/*
OpenALSound.h

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

#ifndef _OPENALSOUND_H_
#define _OPENALSOUND_H_

#ifdef MACOSX
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <string>

class OpenALSound
{
 private:

  ALCdevice*  m_pDevice;
  ALCcontext* m_pContext;

 private:

  // Singlton
  OpenALSound();
  OpenALSound(const OpenALSound&);
  OpenALSound& operator=(const OpenALSound&);

  // Device & Context
  void   InitDeviceAndContext();
  void   DeleteDeviceAndContext();

  // Util
  void   DisplayALError(ALenum error, const std::string& comment);

 public:

  virtual ~OpenALSound();

  // Singlton
  static OpenALSound* Instance();

  //- Source -------------------------------------------------------------------//

  int    DeleteSource(ALuint source);
  ALuint GenerateSource();

  //- Buffer -------------------------------------------------------------------//

  int    AddBufferToQueue(ALuint source, ALuint buffer);
  int    DeleteBuffer(ALuint buffer);
  int    DeleteBuffers(ALuint* buffers, ALsizei num);
  int    DetachAllBuffers(ALuint source);
  ALuint GenerateBuffer();
  int    GenerateBuffers(ALuint* buffers, ALsizei num);
  int    GetNumOfProcessedBuffer(ALuint source);
  int    GetNumOfQueuedBuffer(ALuint source);

  // fpに記録されたfomat形式のPCMデータをbufferに書き込む。
  // formatは、alBufferDataのformatと同じで、下記4つのどれかを指定する。
  // AL_FORMAT_MONO8、AL_FORMAT_MONO16、AL_FORMAT_STEREO8、AL_FORMAT_STEREO16
  // 成功した場合は0、それ以外は-1を返す。
  int    LoadPCMToBuffer(FILE* fp, ALuint buffer, ALenum format);

  int    UnqueueBuffers(ALuint* buffers, ALsizei num, ALuint source);

  // Operation
  int    Pause(ALuint source);
  int    Play(ALuint source, float gain, ALboolean loop);
  int    Stop(ALuint source);
  
  // State
  bool   IsInitialState(ALuint source);
  bool   IsPaused(ALuint source);
  bool   IsPlaying(ALuint source);
  bool   IsStopped(ALuint source);

};

#endif
