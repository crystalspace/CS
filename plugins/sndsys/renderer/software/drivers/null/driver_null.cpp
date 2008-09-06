/*
    Copyright (C) 2005 by Andrew Mann

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csutil/sysfunc.h"
#include "csutil/scf_implementation.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"

#include "../../renderer.h"
#include "driver_null.h"




CS_PLUGIN_NAMESPACE_BEGIN(SndSysNull)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverNull)


SndSysDriverNull::SndSysDriverNull(iBase* pParent) :
  scfImplementationType(this, pParent),
  m_pObjectReg(0), m_bRunning(false)
{
}


SndSysDriverNull::~SndSysDriverNull()
{
}


bool SndSysDriverNull::Initialize (iObjectRegistry *pObjectReg)
{
  /// Interface to the Configuration file
  csConfigAccess Config;

  // Store the Object registry interface pointer, we'll need it later
  m_pObjectReg=pObjectReg;

  // Get an interface for event recorder (if present)
  m_EventRecorder = csQueryRegistry<iSndSysEventRecorder> (m_pObjectReg);

  // Critical because you really want to log this.  Trust me.  Really.
  RecordEvent(SSEL_CRITICAL, "NULL (no output) driver for software sound renderer initialized.");

  // Make sure sound.cfg is available
  Config.AddConfig(m_pObjectReg, "/config/sound.cfg");

  csRef<iCommandLineParser> CMDLine (
    csQueryRegistry<iCommandLineParser> (m_pObjectReg));

  m_BufferLengthms=0;
  if (CMDLine)
  {
    const char *BufferLengthStr = CMDLine->GetOption("soundbufferms");
    if (BufferLengthStr) m_BufferLengthms=atoi(BufferLengthStr);
  }

  // Check for sound config file option. Default to 20 ms if no option is found.
  if (m_BufferLengthms<=0)
    m_BufferLengthms = Config->GetInt("SndSys.Driver.NULL.SoundBufferms", 20);

  return true;
}

void SndSysDriverNull::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_EventRecorder->RecordEventV(SSEC_DRIVER, Severity, msg, arg);
  va_end (arg);
}


bool SndSysDriverNull::Open (csSndSysRendererSoftware* pRenderer,
			     csSndSysSoundFormat *pRequestedFormat)
{
  RecordEvent(SSEL_DEBUG, "NULL Driver: Open()");
//  CS_ASSERT (Config != 0);

  // Copy the format into local storage
  memcpy(&m_PlaybackFormat, pRequestedFormat, sizeof(csSndSysSoundFormat));

  m_pAttachedRenderer=pRenderer;

  return true;
}

void SndSysDriverNull::Close ()
{
}

bool SndSysDriverNull::StartThread()
{
  if (m_bRunning) return false;

  m_bRunning=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  m_pBGThread.AttachNew (new CS::Threading::Thread (runnable, false));
  runnable->DecRef ();

  m_pBGThread->Start();
  
  return true;
}


void SndSysDriverNull::StopThread()
{
  m_bRunning=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  m_pParent->Run ();
}

void SndSysDriverNull::Run()
{
  unsigned char *pSoundBuffer;
  size_t SoundBufferFrames;

  // Allocate a buffer that's as big as a real output buffer would be
  SoundBufferFrames=m_BufferLengthms * m_PlaybackFormat.Freq / 1000;
  pSoundBuffer=new unsigned char [SoundBufferFrames * m_PlaybackFormat.Bits/8 * m_PlaybackFormat.Channels];

  csTicks CurrentTicks, LastTicks;

  LastTicks=csGetTicks();

  // The main loop of the background thread
  while (m_bRunning)
  {
    CurrentTicks=csGetTicks();
    size_t NeededFrames=(CurrentTicks - LastTicks) * m_PlaybackFormat.Freq / 1000;
    if (NeededFrames >= SoundBufferFrames/4)
    {
      if (NeededFrames > SoundBufferFrames)
      {
        RecordEvent(SSEL_ERROR, "Would have underbuffered. Need %d frames. Buffer size %d frames.",
                    NeededFrames, SoundBufferFrames);
        NeededFrames=SoundBufferFrames;
      }
      m_pAttachedRenderer->FillDriverBuffer(pSoundBuffer, NeededFrames, 0, 0);
      LastTicks=CurrentTicks;
    } 
    csSleep(m_BufferLengthms/4);
  }
  RecordEvent(SSEL_DEBUG, "Main run loop complete.  Shutting down.");

  delete[] pSoundBuffer;
}


}
CS_PLUGIN_NAMESPACE_END(SndSysNull)

