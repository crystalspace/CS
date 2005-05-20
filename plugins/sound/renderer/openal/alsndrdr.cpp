/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#include "csutil/scf.h"
#include "csutil/event.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"

#include "alsndrdr.h"
#include "alsndlst.h"
#include "alsndsrc.h"
#include "alsndhdl.h"


#if defined(CS_OPENAL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,al.h)
#else
#include <AL/al.h>
#endif

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundRenderOpenAL);


SCF_IMPLEMENT_IBASE(csSoundRenderOpenAL)
  SCF_IMPLEMENTS_INTERFACE(iSoundRender)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderOpenAL::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderOpenAL::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csSoundRenderOpenAL::csSoundRenderOpenAL(iBase *parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  Listener = 0;
  object_reg = 0;

  roll = 1.0;
  dist = 1.0;

  mutex_Listener = csMutex::Create (true);
  mutex_ActiveSources = csMutex::Create (true);
  mutex_SoundHandles = csMutex::Create (true);
  mutex_OpenAL = csMutex::Create ();

  bRunning=false;
  al_open = false;
}

bool csSoundRenderOpenAL::Initialize(iObjectRegistry *r)
{
  object_reg = r;

  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->RegisterListener (&scfiEventHandler,
    CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);

  config.AddConfig(object_reg, "/config/sound.cfg");

  // Play the data in whatever format it's already in
  format.Freq=-1;
  format.Bits=-1;
  format.Channels=-1;

  return true;
}

csSoundRenderOpenAL::~csSoundRenderOpenAL()
{
  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoundRenderOpenAL::Open()
{
  csRef<iReporter> reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.openal",
    "SoundRender OpenAL selected");

  // OpenAL Initialization
  alutInit (0, 0);
  alGetError ();

  // We have to pass new through a csPtr<> to avoid reference issues
  Listener = csPtr<iSoundListener> (new csSoundListenerOpenAL (this));

  // Let the config value override the default sound volume
  SetVolume (config->GetFloat ("Sound.Volume", 1.0));


  // OpenAL Initialized
  al_open = true;

  /* This is the amount of audio (in seconds) that is sent ahead to the underlying layer for streaming audio
  *  Don't set this extremely tiny or you will cause increased CPU usage.
  */
  BufferLengthSeconds=config->GetFloat("Sound.OpenAL.StreamingBufferLength",1.0);

  /* If true, a background thread is created for streaming updates.  If false updates are triggered by
  *  events through the main cs loop.  
  *
  * There's really no reason to turn this off, and a lot of reasons to leave it on.  You'll probably need a LARGE
  *  StreamingBufferLength value to get uninterrupted streams with this set to false.
  *
  * Static (non-streaming) audio is not affected in any end-user noticable manner when this is false.
  */
  BackgroundProcessing=config->GetBool("Sound.OpenAL.BackgroundProcessing",true);

  /* This can be turned on if you're ok with a delay on adding streaming audio sources to a running stream exactly
  *  equal to Sound.OpenAL.StreamingBufferLength .  It will mean that the handle does not need to keep its own
  *  internal buffer, which can save up to 176Kbtyes per second of Sound.OpenAL.StreamingBufferLength per streaming
  *  handle registered (a good bit).
  *
  *  If you need to add sources to a started stream and have the Play() command take effect without delay
  *   then you will need to have this set to false.
  *
  *  The default is on since this can be a fairly memory intensive option, and adding sources to a running stream is
  *   uncommon.
  */
  LazySourceSync=config->GetBool("Sound.OpenAL.LazySourceSync",true);


  // Set the first time
  LastTime = csGetTicks ();



  // Start the background thread - Make sure this is done AFTER all initialization!
  if (BackgroundProcessing)
  {
    bRunning=true;
    bgThread=csThread::Create (new OpenALRunnable (this));
    bgThread->Start();
  }


  return true;
}

void csSoundRenderOpenAL::Close()
{
  if (!al_open) return;

  // Signal the background thread to halt
  if (bRunning)
  {
    bRunning=false;
    bgThread->Wait();
  }

  // Release the listener reference
  Listener = 0;

  // Stop all sources
  csSoundSourceOpenAL *src;
  mutex_ActiveSources->LockWait();
  while (ActiveSources.Length())
  {
    src = (csSoundSourceOpenAL*)ActiveSources.Get(0);
    src->Stop();
    ActiveSources.DeleteIndex (0);
  }
  mutex_ActiveSources->Release();

  // Delete all handles
  csSoundHandleOpenAL *hdl;
  mutex_SoundHandles->LockWait();
  while (SoundHandles.Length())
  {
    hdl = (csSoundHandleOpenAL *)SoundHandles.Get(0);
    hdl->StopStream();
    SoundHandles.DeleteIndex (0);
  }
  mutex_SoundHandles->Release();

  // Terminate the OpenAL library
  mutex_OpenAL->LockWait();
  alutExit ();
  mutex_OpenAL->Release();

  al_open = false;
}

void csSoundRenderOpenAL::SetVolume(float vol)
{
  mutex_OpenAL->LockWait();
  alListenerf (AL_GAIN, vol);
  mutex_OpenAL->Release();
}

float csSoundRenderOpenAL::GetVolume()
{
  float vol;
  mutex_OpenAL->LockWait();
  alGetListenerf (AL_GAIN, &vol);
  mutex_OpenAL->Release();
  return vol;
}

csPtr<iSoundHandle> csSoundRenderOpenAL::RegisterSound(iSoundData *snd)
{
  CS_ASSERT (snd);
  if (!snd->Initialize (&format)) return 0;

#ifdef OPENAL_DEBUG_CALLS
  csRef<iReporter> reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.openal",
    "RegisterSound (%s)",snd->IsStatic() ? "static" : "streamed");
#endif


  csSoundHandleOpenAL *hdl = new csSoundHandleOpenAL (this, snd,BufferLengthSeconds,!LazySourceSync);

  // Push onto the handles list
  mutex_SoundHandles->LockWait();
  SoundHandles.Push (hdl);
  mutex_SoundHandles->Release();

  return hdl;
}

void csSoundRenderOpenAL::UnregisterSound (iSoundHandle *snd)
{
  mutex_SoundHandles->LockWait();
  // Remove the handle from the list
  csRef<csSoundHandleOpenAL> hdl = (csSoundHandleOpenAL *)snd;
  size_t n = SoundHandles.Find(hdl);
  if (n != csArrayItemNotFound)
  {
    SoundHandles.DeleteIndex (n);
  }
  mutex_SoundHandles->Release();


#ifdef OPENAL_DEBUG_CALLS
  csRef<iReporter> reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.openal",
    "UnRegisterSound (%s)",hdl->IsStatic() ? "static" : "streamed");
#endif

}

void csSoundRenderOpenAL::MixingFunction()
{
}

void csSoundRenderOpenAL::Update()
{
  size_t i;
  csTicks ct, et;
  ct = csGetTicks ();

  et=ct-LastTime;
  LastTime = ct;

#ifdef OPENAL_DEBUG_CALLS
  csRef<iReporter> reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.openal",
    "Update()");
#endif


  //  mutex_Listener->LockWait();
  //  Listener->Prepare();
  //  mutex_Listener->Release();

  mutex_SoundHandles->LockWait();
  mutex_ActiveSources->LockWait();
  for (i=0;i<SoundHandles.Length();i++)
  {
    csSoundHandleOpenAL *hdl = (csSoundHandleOpenAL*)SoundHandles.Get(i);
    // Elapsed Time is ignored unless the handle itself is keeping track of playback time
    hdl->Update_Time(et);
  }

  for (i=0;i<ActiveSources.Length();i++)
  {
    csSoundSourceOpenAL *src = (csSoundSourceOpenAL*)ActiveSources.Get(i);
    if (!src->IsPlaying())
    {
      //  Once a source has stopped playing it can be removed.
      ActiveSources.DeleteIndex (i);
      i--;
    }
  }
  mutex_ActiveSources->Release();
  mutex_SoundHandles->Release();
}

void csSoundRenderOpenAL::AddSource(csSoundSourceOpenAL *src)
{
  mutex_OpenAL->LockWait();
  alSourcef(src->GetID(), AL_REFERENCE_DISTANCE, dist);
  alSourcef(src->GetID(), AL_ROLLOFF_FACTOR, roll);
  mutex_OpenAL->Release();
  mutex_ActiveSources->LockWait();
  ActiveSources.Push(src);
  mutex_ActiveSources->Release();
}

void csSoundRenderOpenAL::RemoveSource(csSoundSourceOpenAL *src)
{
  mutex_ActiveSources->LockWait();
  ActiveSources.Delete(src);
  mutex_ActiveSources->Release();
}

void csSoundRenderOpenAL::ThreadProc ()
{
  /* This should let us sleep a good amount, but not so much that the buffer can empty out
  *  even if the system is under high load and things are running slow.
  */
  int sleeptime=(int)(BufferLengthSeconds*300);

  while (bRunning)
  {
    Update();
    csSleep(sleeptime);
  }
}

bool csSoundRenderOpenAL::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast)
  {
    switch (csCommandEventHelper::GetCode(&e))
    {
    case cscmdPreProcess:
      if (!BackgroundProcessing)
        Update(); // Only perform updates here if there is no background thread
      break;
    case cscmdSystemOpen:
      Open();
      break;
    case cscmdSystemClose:
      Close();
      break;
    }
  }
  return false;
}
