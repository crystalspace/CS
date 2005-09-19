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

#include "../../renderer.h"
#include "ss_driver.h"
#include "ss_renderer.h"
#include "driver_ds.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (SndSysDriverDirectSound)


SCF_IMPLEMENT_IBASE(SndSysDriverDirectSound)
SCF_IMPLEMENTS_INTERFACE(iSndSysSoftwareDriver)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysDriverDirectSound::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// The system driver.
iObjectRegistry *SndSysDriverDirectSound::object_reg=NULL;

// The loaded CS reporter
csRef<iReporter> SndSysDriverDirectSound::reporter;


SndSysDriverDirectSound::SndSysDriverDirectSound(iBase* piBase) :
 ds_buffer_writecursor(0), running(false), ds_buffer(NULL), ds_device(NULL)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

//  scfiEventHandler = 0;
  object_reg = 0;

  ds_device=NULL;
  ds_buffer=NULL;
}


SndSysDriverDirectSound::~SndSysDriverDirectSound()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void SndSysDriverDirectSound::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  if (!reporter)
    reporter = CS_QUERY_REGISTRY(object_reg, iReporter);

  if (reporter)
    reporter->ReportV (severity, "crystalspace.SndSys.driver.software.directsound", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}



bool SndSysDriverDirectSound::Initialize (iObjectRegistry *obj_reg)
{
  // copy the system pointer
  object_reg=obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct sound driver for software sound renderer initialized.");

  // read the config file
//  Config.AddConfig(object_reg, "/config/sound.cfg");


  win32Assistant = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  if (!win32Assistant)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Direct sound driver: Could not locate iWin32Assistant object in CS registry.");
    return false;
  }



 
  return true;
}

//////////////////////////////////////////////////////////////////////////
// 
//  
//
//
//
//
//////////////////////////////////////////////////////////////////////////
bool SndSysDriverDirectSound::Open (SndSysRendererSoftware *renderer,SndSysSoundFormat *requested_format)
{
  HRESULT hr;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Open()");
//  CS_ASSERT (Config != 0);

  attached_renderer=renderer;
  memcpy(&playback_format, requested_format, sizeof(SndSysSoundFormat));


  hr = DirectSoundCreate8(NULL, &ds_device, NULL);
  if (FAILED(hr))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Direct Sound Driver: DirectSoundCreate8 failed.");
    return false;
  }

  hr = ds_device->SetCooperativeLevel(win32Assistant->GetApplicationWindow(),DSSCL_PRIORITY);
  if (FAILED(hr))
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Sound System: Direct Sound Driver: Failed to set cooperative level to DSSCL_PRIORITY! Error %08x :'%s'", hr, GetDSError(hr));
    return false;
  }

  ds_buffer_bytes=40000;
  ds_buffer_minimum_fill_bytes=ds_buffer_bytes/10;

  DSBUFFERDESC ds_bufferdesc;
  WAVEFORMATEX ds_wavformat;
  
  memset(&ds_wavformat, 0 ,sizeof(WAVEFORMATEX));
  ds_wavformat.wFormatTag=WAVE_FORMAT_PCM;
  ds_wavformat.nChannels=requested_format->Channels;
  ds_wavformat.nSamplesPerSec=requested_format->Freq;
  ds_wavformat.wBitsPerSample=requested_format->Bits;
  ds_wavformat.nBlockAlign = (requested_format->Channels * requested_format->Bits) / 8;
  ds_wavformat.nAvgBytesPerSec = requested_format->Freq * ds_wavformat.nBlockAlign;
  ds_wavformat.cbSize=0;




  memset(&ds_bufferdesc, 0, sizeof(DSBUFFERDESC));
  ds_bufferdesc.dwSize=sizeof(DSBUFFERDESC);
  ds_bufferdesc.dwFlags=DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER  ;
  ds_bufferdesc.dwBufferBytes=ds_buffer_bytes;
  ds_bufferdesc.lpwfxFormat=&ds_wavformat;
  ds_bufferdesc.guid3DAlgorithm=GUID_NULL;
  
  hr = ds_device->CreateSoundBuffer(&ds_bufferdesc,&ds_buffer,NULL);
  if (FAILED(hr))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Direct Sound Driver: Failed to create sound buffer.");
    return false;
  }







  return true;
}

void SndSysDriverDirectSound::Close ()
{
  if (ds_buffer) ds_buffer->Release();
  if (ds_device) ds_device->Release();
}

bool SndSysDriverDirectSound::StartThread()
{
  if (running) return false;

  running=true;
  bgthread = csThread::Create(this);

  bgthread->Start();
  
  return true;
}


void SndSysDriverDirectSound::StopThread()
{
  running=false;
  csSleep(100);
}

void SndSysDriverDirectSound::Run()
{
  HRESULT hr;
  uint32 playcursor, writecursor;

  //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Clearing buffer in preparation for playback.");
  ClearBuffer();

  //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Beginning playback of empty buffer.");
  hr=ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
  if (FAILED(hr))
  {
    //Report(CS_REPORTER_SEVERITY_ERROR, "Sound System: Direct Sound Driver: Failed to begin playback! Error %08x :'%s'", hr, GetDSError(hr));
    return;
  }

  // Start 1/10th of a second from the end of the buffer
  uint32 bytes_per_sec=(playback_format.Freq * playback_format.Channels * playback_format.Bits/8);
  if (ds_buffer_bytes < (bytes_per_sec/10))
    ds_buffer_writecursor=0;
  else
    ds_buffer_writecursor=ds_buffer_bytes - (bytes_per_sec/10);


  while (running)
  {

    // Retrive the current play and write cursor for the buffer.
    // The write cursor is mostly ignored, unless it's beyond where we think the cursor is,
    //  which means we've skipped
    hr = ds_buffer->GetCurrentPosition((LPDWORD)&playcursor, (LPDWORD)&writecursor);
    if (SUCCEEDED(hr))
    {
      uint32 writablebytes;
      int writegap;

      // Retrieve the gap between the end of the DS locked buffer and our tracked write cursor
      writegap=GetWriteGap(playcursor,writecursor);

      if (writegap < (long)(bytes_per_sec/20))
      {
        //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Skip of %d bytes detected.", -writegap);
        ds_buffer_writecursor=playcursor;
        if (ds_buffer_writecursor < (bytes_per_sec/10))
          ds_buffer_writecursor+=ds_buffer_bytes;
        ds_buffer_writecursor-=bytes_per_sec/10;
      }

      // Retrieve the gap between our tracked write cursor and the DS play cursor
      writablebytes=GetWritableBytes(playcursor);

      /*
      if (writegap<0)
      {
        LPVOID buf1,buf2;
        uint32 buf1_len,buf2_len;

        // Since we underbuffered, we have to use a different method of locking the buffer here
        Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System 2: Direct Sound Driver: Skip of %d bytes detected.", -writegap);

        // Lock the buffer from the writecursor - offset is ignored
        hr=ds_buffer->Lock(0, writablebytes,&buf1,&buf1_len,&buf2,&buf2_len,DSBLOCK_FROMWRITECURSOR);
        if (FAILED(hr))
        {
          Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System 2: Direct Sound Driver: Failed to lock %d bytes for write.", writablebytes);
          continue;
        }

        // Re-read the write cursor after the lock, we'll sync the write cursor up to this
        hr = ds_buffer->GetCurrentPosition(&playcursor, &writecursor);
        ds_buffer_writecursor=writecursor;


        attached_renderer->FillDriverBuffer(buf1, buf1_len, buf2, buf2_len);

        // Advance the write buffer
        AdvanceWriteBuffer(writablebytes);

        writablebytes=0;
      }
      */


      // Don't write data to the buffer if there's not enough to be worth writing
      if (writablebytes >= ds_buffer_minimum_fill_bytes)
      {
        LPVOID buf1,buf2;
        uint32 buf1_len,buf2_len;
        uint32 bytes_used;

        hr=ds_buffer->Lock(ds_buffer_writecursor, writablebytes,&buf1,(LPDWORD)&buf1_len,&buf2,(LPDWORD)&buf2_len,0);
        if (FAILED(hr))
        {
          //Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Direct Sound Driver: Failed to lock %d bytes for write.", writablebytes);
          continue;
        }

        bytes_used=attached_renderer->FillDriverBuffer(buf1, buf1_len, buf2, buf2_len);

        // Unlock the buffer
        ds_buffer->Unlock(buf1,buf1_len,buf2,buf2_len);

        // Advance the write buffer
        AdvanceWriteBuffer(bytes_used);
      }
    }
    csSleep(0);
  }
}

void SndSysDriverDirectSound::ClearBuffer()
{
  LPVOID buf1,buf2;
  uint32 buf1_len,buf2_len;
  HRESULT hr;

  hr=ds_buffer->Lock(0,0,&buf1,(LPDWORD)&buf1_len,&buf2,(LPDWORD)&buf2_len,DSBLOCK_FROMWRITECURSOR  | DSBLOCK_ENTIREBUFFER);
  if (FAILED(hr))
  {
    //Report(CS_REPORTER_SEVERITY_NOTIFY, "Sound System: Direct Sound Driver: Failed to lock buffer for clear. Error: %s", GetDSError(hr));
    return;
  }

  if (buf1)
    memset(buf1,0,buf1_len);
  if (buf2)
    memset(buf2,0,buf2_len);

  ds_buffer->Unlock(buf1,buf1_len,buf2,buf2_len);
}


int SndSysDriverDirectSound::GetWriteGap(uint32 real_play_cursor, uint32 real_write_cursor)
{
  uint32 write_cursor = ds_buffer_writecursor;
  int gap;

  // Write cursor is always ahead of the play cursor
  if (real_write_cursor<=real_play_cursor)
    real_write_cursor+=ds_buffer_bytes;
  if (write_cursor<=real_play_cursor)
    write_cursor+=ds_buffer_bytes;

  gap=write_cursor-real_write_cursor;

  return gap;
}

uint32 SndSysDriverDirectSound::GetWritableBytes(uint32 real_play_cursor)
{
  if (real_play_cursor<ds_buffer_writecursor)
    real_play_cursor+=ds_buffer_bytes;

  return (real_play_cursor-ds_buffer_writecursor);
}

void SndSysDriverDirectSound::AdvanceWriteBuffer(uint32 bytes)
{
  ds_buffer_writecursor+=bytes;
  if (ds_buffer_writecursor >= ds_buffer_bytes)
    ds_buffer_writecursor-=ds_buffer_bytes;
}


const char *SndSysDriverDirectSound::GetDSError(HRESULT hr)
{
  switch (hr)
  {
    case DSERR_BUFFERLOST:
      return "Buffer Lost";
    case DSERR_INVALIDCALL:
      return "Invalid Call";
    case DSERR_INVALIDPARAM:
      return "Invalid Parameter";
    case DSERR_PRIOLEVELNEEDED:
      return "Priority Level Needed";
    case DSERR_OUTOFMEMORY:
      return "Out of Memory";
    default:
      return "Unknown error";
    break;
  }


}

