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
#include "csutil/event.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"

#include "csplugincommon/directx/error.h"

#include "../../renderer.h"
#include "isndsys/ss_renderer.h"
#include "driver_ds.h"


CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(SndSysDIRECTSOUND)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverDirectSound)


SndSysDriverDirectSound::SndSysDriverDirectSound(iBase* pParent) :
  scfImplementationType(this, pParent),
  m_pDirectSoundDevice(0), m_pDirectSoundBuffer(0), m_bRunning(false)
{
  m_pObjectRegistry = 0;

  m_pDirectSoundDevice=0;
  m_pDirectSoundBuffer=0;
}


SndSysDriverDirectSound::~SndSysDriverDirectSound()
{
}


bool SndSysDriverDirectSound::Initialize (iObjectRegistry *obj_reg)
{
  /// Interface to the Configuration file
  csConfigAccess Config;

  // copy the system pointer
  m_pObjectRegistry=obj_reg;

  // Get an interface for event recorder (if present)
  m_EventRecorder = csQueryRegistry<iSndSysEventRecorder> (m_pObjectRegistry);

  // Critical because you really want to log this.  Trust me.  Really.
  RecordEvent(SSEL_CRITICAL, "Direct sound driver for software sound renderer initialized.");

  // read the config file
  Config.AddConfig(m_pObjectRegistry, "/config/sound.cfg");

  // check for optional output buffer length from the command line and config
  csRef<iCommandLineParser> CMDLine (CS_QUERY_REGISTRY (m_pObjectRegistry,
    iCommandLineParser));

  m_BufferLengthms=0;
  if (CMDLine)
  {
    const char *BufferLengthStr = CMDLine->GetOption("soundbufferms");
    if (BufferLengthStr) m_BufferLengthms=atoi(BufferLengthStr);
  }

  // Check for sound config file option. Default to 20 ms if no option is found.
  if (m_BufferLengthms<=0)
    m_BufferLengthms = Config->GetInt("SndSys.Driver.Win.SoundBufferms", 20);

  // The number of underbuffer events before the buffer size is automatically increased
  m_UnderBuffersAllowed=5;
 
  return true;
}

void SndSysDriverDirectSound::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_EventRecorder->RecordEventV(SSEC_DRIVER, Severity, msg, arg);
  va_end (arg);
}


bool SndSysDriverDirectSound::Open (csSndSysRendererSoftware *renderer,
  csSndSysSoundFormat *requested_format)
{
  HRESULT DirectSoundResult;

  RecordEvent(SSEL_DEBUG, 
    "Sound System: Direct Sound Driver: Open()");
//  CS_ASSERT (Config != 0);

  m_pAttachedRenderer=renderer;
  memcpy (&m_PlaybackFormat, requested_format, sizeof(csSndSysSoundFormat));


  DirectSoundResult = DirectSoundCreate8(0, &m_pDirectSoundDevice, 0);
  if (FAILED(DirectSoundResult))
  {
    RecordEvent(SSEL_ERROR, "DirectSoundCreate8 failed.");
    return false;
  }

  csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (m_pObjectRegistry);
  if (!g2d.IsValid())
  {
    RecordEvent(SSEL_ERROR, 
      "Could not obtain an iGraphics2D canvas (required for Driver)");
    return false;
  }
  csRef<iWin32Canvas> canvas = scfQueryInterface<iWin32Canvas> (g2d);
  CS_ASSERT (canvas.IsValid());

  DirectSoundResult = m_pDirectSoundDevice->SetCooperativeLevel (canvas->GetWindowHandle(),
    DSSCL_PRIORITY);
  if (FAILED(DirectSoundResult))
  {
    RecordEvent(SSEL_ERROR, "Failed to set cooperative level to "
      "DSSCL_PRIORITY: %s (%s)", 
      csDirectXError::GetErrorDescription (DirectSoundResult),
      csDirectXError::GetErrorSymbol (DirectSoundResult));
    return false;
  }

  // Store the number of bytes per audio frame
  m_BytesPerFrame=m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8;

  // Create the DirectSound buffer
  if (!CreateBuffer())
  {
    m_pDirectSoundDevice->Release();
    m_pDirectSoundDevice=0;
    return false;
  }

  return true;
}

void SndSysDriverDirectSound::Close ()
{
  if (m_pDirectSoundBuffer) m_pDirectSoundBuffer->Release();
  if (m_pDirectSoundDevice) m_pDirectSoundDevice->Release();
  m_pDirectSoundBuffer=0;
  m_pDirectSoundDevice=0;
}

bool SndSysDriverDirectSound::CreateBuffer()
{
  HRESULT DirectSoundResult;

  // Update the Frames, Bytes and Minimum Fill Frames values from the current millisecond length
  m_DirectSoundBufferFrames=m_PlaybackFormat.Freq * m_BufferLengthms / 1000;
  m_DirectSoundBufferBytes=(DWORD)(m_BytesPerFrame * m_DirectSoundBufferFrames);
  // If there's more than 1/4 of the total buffer space free, try to fill it
  m_DirectSoundBufferMinimumFillFrames=m_DirectSoundBufferFrames/4;

  RecordEvent(SSEL_DEBUG, "Creating new sound buffer.  Freq [%d] Chan [%d] Bits [%d] Length [%d ms]",
    m_PlaybackFormat.Freq, m_PlaybackFormat.Channels, m_PlaybackFormat.Bits, m_BufferLengthms);


  DSBUFFERDESC m_pDirectSoundBufferdesc;
  WAVEFORMATEX ds_wavformat;

  memset(&ds_wavformat, 0 ,sizeof(WAVEFORMATEX));
  ds_wavformat.wFormatTag=WAVE_FORMAT_PCM;
  ds_wavformat.nChannels=m_PlaybackFormat.Channels;
  ds_wavformat.nSamplesPerSec=m_PlaybackFormat.Freq;
  ds_wavformat.wBitsPerSample=m_PlaybackFormat.Bits;
  ds_wavformat.nBlockAlign = (DWORD)m_BytesPerFrame;
  ds_wavformat.nAvgBytesPerSec = (DWORD)(m_PlaybackFormat.Freq * m_BytesPerFrame);
  ds_wavformat.cbSize=0;


  memset(&m_pDirectSoundBufferdesc, 0, sizeof(DSBUFFERDESC));
  m_pDirectSoundBufferdesc.dwSize=sizeof(DSBUFFERDESC);
  m_pDirectSoundBufferdesc.dwFlags=DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2 
    | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER  ;
  m_pDirectSoundBufferdesc.dwBufferBytes=m_DirectSoundBufferBytes;
  m_pDirectSoundBufferdesc.lpwfxFormat=&ds_wavformat;
  m_pDirectSoundBufferdesc.guid3DAlgorithm=GUID_NULL;

  DirectSoundResult = m_pDirectSoundDevice->CreateSoundBuffer(&m_pDirectSoundBufferdesc,&m_pDirectSoundBuffer,0);
  if (FAILED(DirectSoundResult))
  {
    RecordEvent(SSEL_ERROR, "Failed to create sound buffer.  Freq [%d] Chan [%d] Bits [%d] Length [%d ms]",
      m_PlaybackFormat.Freq, m_PlaybackFormat.Channels, m_PlaybackFormat.Bits, m_BufferLengthms);
    return false;
  }
  return true;
}

bool SndSysDriverDirectSound::DestroyBuffer()
{
  RecordEvent(SSEL_DEBUG, "Destroying current sound buffer.");

  if (m_pDirectSoundBuffer)
  {
    m_pDirectSoundBuffer->Stop();
    m_pDirectSoundBuffer->Release();
  }
  m_pDirectSoundBuffer=0;

  return true;
}

bool SndSysDriverDirectSound::StartThread()
{
  if (m_bRunning) return false;

  m_bRunning=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  m_pBackgroundThread = csThread::Create(runnable);
  runnable->DecRef ();

  m_pBackgroundThread->Start();
  
  return true;
}


void SndSysDriverDirectSound::StopThread()
{
  m_bRunning=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  parent->Run ();
}

void SndSysDriverDirectSound::Run()
{
  HRESULT DirectSoundResult;
  int UnderBufferCount=0;

  // To detect underbuffer conditions, we will use both the cursors provided by
  //  DirectSound and the system clock
  csTicks LastBufferFillTime, CurrentTime;

  //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Clearing buffer in preparation for playback.");
  ClearBuffer();

  //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Beginning playback of empty buffer.");
  DirectSoundResult=m_pDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
  if (FAILED(DirectSoundResult))
  {
    RecordEvent(SSEL_ERROR, "Failed to begin playback! Error [%s]",
      csDirectXError::GetErrorDescription(DirectSoundResult));
    return;
  }

  // Start the clock
  LastBufferFillTime=csGetTicks();

  // We need to track our own write cursor, because DirectSound's write cursor doesn't
  //  indicate the position at which data should be written next, but instead indicates
  //  the earliest position which has not yet been committed to the systems below DirectSound
  // This difference means that using DirectSound's writecursor will cause us to constantly
  //  overwrite a little bit of our last buffer commit.
  int RealWriteCursor=-1;
  while (m_bRunning)
  {

    // Retrieve the current play and write cursor for the buffer.
    //  These values are used to determine how much space is available in the buffer
    //  and whether a write is worthwhile
    DWORD playcursor=0, writecursor=0;
    DirectSoundResult = m_pDirectSoundBuffer->GetCurrentPosition(&playcursor, &writecursor);
    if (!SUCCEEDED(DirectSoundResult))
    {
      RecordEvent(SSEL_ERROR, "Failed to retrieve current position from DirectSound buffer. Error [%s]", 
        csDirectXError::GetErrorDescription(DirectSoundResult));

      // TODO: Can we recover from this?
    }
    else
    {
      if (RealWriteCursor<0)
        RealWriteCursor=writecursor;

      RecordEvent(SSEL_DEBUG, "Write cursor is [%u]  Our write cursor is [%d]", writecursor, RealWriteCursor);  


      // Determine if it's worth filling the buffer.  Calculate the number of frames available in the buffer
      int bytesfree=(int)(playcursor-RealWriteCursor);
      if (bytesfree<0)
        bytesfree += m_DirectSoundBufferBytes;
      int framesfree=(int)(bytesfree / m_BytesPerFrame);
      if (framesfree >= (int)m_DirectSoundBufferMinimumFillFrames)
      {
        LPVOID buf1,buf2;
        DWORD buf1_len,buf2_len;
        size_t frames_used;

        RecordEvent(SSEL_DEBUG, "Locking %d bytes %d frames in DirectSound buffer.", bytesfree, framesfree);  

        DirectSoundResult=m_pDirectSoundBuffer->Lock (RealWriteCursor, bytesfree,
          &buf1, &buf1_len, &buf2, &buf2_len, 0);
        if (FAILED(DirectSoundResult))
        {
          RecordEvent(SSEL_ERROR, "Failed to lock DirectSound buffer. Error [%s]",  
            csDirectXError::GetErrorDescription(DirectSoundResult));

          // TODO: Can we recover from this?

          continue;
        }

        RecordEvent(SSEL_DEBUG, "Locked buffer %d bytes in first, %d bytes in second.", buf1_len, buf2_len);  

        // Tell the renderer 
        frames_used = m_pAttachedRenderer->FillDriverBuffer (buf1, buf1_len/m_BytesPerFrame, buf2, buf2_len/m_BytesPerFrame);

        // Unlock the buffer
        if (frames_used * m_BytesPerFrame <= buf1_len)
        {
          buf1_len=(DWORD)(frames_used * m_BytesPerFrame);
          buf2_len=0;
        }
        else
          buf2_len=(DWORD)((frames_used * m_BytesPerFrame) - buf1_len);

        RecordEvent(SSEL_DEBUG, "Unlocking buffer %d bytes in first, %d bytes in second.", buf1_len, buf2_len);  

        m_pDirectSoundBuffer->Unlock(buf1,buf1_len,buf2,buf2_len);
        if (FAILED(DirectSoundResult))
        {
          RecordEvent(SSEL_ERROR, "Failed to unlock DirectSound buffer. Error [%s]",  
            csDirectXError::GetErrorDescription(DirectSoundResult));

          // TODO: Can we recover from this?

          continue;
        }

        // Advance and wrap our real write cursor
        RealWriteCursor+=buf1_len+buf2_len;
        RealWriteCursor%=m_DirectSoundBufferBytes;


        CurrentTime=csGetTicks();
        // Detect an underbuffer.  This occurs when the time between the last fill
        //  and now exceeds the entire buffer length.
        if (CurrentTime - LastBufferFillTime >= m_BufferLengthms)
        {
          UnderBufferCount++;

          RecordEvent(SSEL_WARNING, "Underbuffer condition detected.  Buffer length [%u] Time since last fill [%u] Count [%d]", 
            m_BufferLengthms, CurrentTime - LastBufferFillTime, UnderBufferCount);

          if (UnderBufferCount >  m_UnderBuffersAllowed)
          {
            // Reset the counter
            UnderBufferCount=0;

            // Take corrective action - but do not expand the buffer beyond 1 second
            if (m_BufferLengthms < 1000)
            {
              RecordEvent(SSEL_WARNING, "Corrective underbuffering protection doubling buffer size.");
              m_BufferLengthms*=2;
              if (m_BufferLengthms > 1000)
                m_BufferLengthms=1000;

              DestroyBuffer();

              if (CreateBuffer())
              {
                // Clear the buffer to silence
                ClearBuffer();

                // Start the buffer playing
                DirectSoundResult=m_pDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
                if (FAILED(DirectSoundResult))
                {
                  RecordEvent(SSEL_ERROR, "Failed to begin playback! Error [%s]",
                    csDirectXError::GetErrorDescription(DirectSoundResult));
                  break;
                }

                // Reset the clock
                LastBufferFillTime=csGetTicks();

                // Restart our write cursor
                RealWriteCursor=-1;
              }
              else
                break;
            } // if (m_BufferLengthms < 1000) {...}
          } // if (UnderBufferCount >  m_UnderBuffersAllowed) {...}
        } // if (CurrentTime - LastBufferFillTime >= m_BufferLengthms) {...}  

        // Update the last fill time
        LastBufferFillTime=CurrentTime;
      } // if (framesfree >= (int)m_DirectSoundBufferMinimumFillFrames) {...}
    } // if (!SUCCEEDED(DirectSoundResult)) {...} else {...}

    csSleep(m_BufferLengthms/4);
  }
}

void SndSysDriverDirectSound::ClearBuffer()
{
  LPVOID buf1,buf2;
  uint32 buf1_len,buf2_len;
  HRESULT DirectSoundResult;

  // Here we fill the entire buffer with silence values.  This is intended as an initialization procedure
  //  so that random noise isn't in the buffer when playback is first started.
  DirectSoundResult=m_pDirectSoundBuffer->Lock(0,0,&buf1,(LPDWORD)&buf1_len,&buf2,(LPDWORD)&buf2_len,DSBLOCK_FROMWRITECURSOR  | DSBLOCK_ENTIREBUFFER);
  if (FAILED(DirectSoundResult))
  {
    RecordEvent(SSEL_ERROR, "Failed to lock buffer for clear. Error: %s", csDirectXError::GetErrorDescription(DirectSoundResult));
    return;
  }

  // 16 bit samples use 0 as the value for silence
  int clearval=0;
  // 8 bit samples use 128 as the value for silence
  if (m_PlaybackFormat.Bits == 8)
    clearval=128;

  if (buf1)
    memset(buf1,clearval,buf1_len);
  if (buf2)
    memset(buf2,clearval,buf2_len);

  m_pDirectSoundBuffer->Unlock(buf1,buf1_len,buf2,buf2_len);
}



}
CS_PLUGIN_NAMESPACE_END(SndSysDIRECTSOUND)

