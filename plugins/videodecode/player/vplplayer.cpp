#include <cssysdef.h>
#include "vplplayer.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>

using namespace std;


SCF_IMPLEMENT_FACTORY (csVplPlayer)

csVplPlayer::csVplPlayer (iBase* parent) :
scfImplementationType (this, parent),
_object_reg (0)
{
}

csVplPlayer::~csVplPlayer ()
{
  // Unregister the Frame event listener
  if (frameEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (_object_reg));
    if (q)
      q->RemoveListener (frameEventHandler);
  }
}

bool csVplPlayer::Initialize (iObjectRegistry* r)
{
  _object_reg = r;

  _mediaFile=NULL;
  _playing = false;
  return true;
}

void csVplPlayer::InitializePlayer (csRef<iMediaContainer> media, size_t cacheSize)
{
  if (!media.IsValid ())
  {
    csReport (_object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
              "Media container is not valid!");
    return;
  }
  else
    _mediaFile = media;

  if (cacheSize<1)
    _mediaFile->SetCacheSize (1);
  else;
  _mediaFile->SetCacheSize (cacheSize);

  CS_INITIALIZE_FRAME_EVENT_SHORTCUTS (_object_reg);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (_object_reg));

  // Register the Frame event listener
  if (!frameEventHandler)
  {
    frameEventHandler.AttachNew (new FrameEventHandler (this));
  }
  if (q != 0)
  {
    csEventID events[2] = { Frame, CS_EVENTLIST_END };
    q->RegisterListener (frameEventHandler, events);
  }
  _shouldStop = false;
  _shouldUpdate=false;
  _shouldPlay=false;
}

void csVplPlayer::StartPlayer ()
{
  if (!_threadInfo.IsValid ())
  {
    _shouldUpdate = true;
    _threadInfo = Update ();
  }
}

void csVplPlayer::StopPlayer ()
{
  _shouldUpdate = false;
}

void csVplPlayer::SetActiveStream (int index) 
{
  if (_mediaFile.IsValid ())
  {
    if (index==-1)
      _mediaFile->AutoActivateStreams ();
    else
      _mediaFile->SetActiveStream (index);
  }
}

void csVplPlayer::RemoveActiveStream (int index) 
{
  if (_mediaFile.IsValid ())
    _mediaFile->RemoveActiveStream (index);
}

void csVplPlayer::GetTargetTexture (csRef<iTextureHandle> &target) 
{
  _mediaFile->GetTargetTexture (target);
}
void csVplPlayer::GetTargetAudio (csRef<iSndSysStream> &target)
{
  _mediaFile->GetTargetAudio (target);
}

void csVplPlayer::SelectLanguage (const char* identifier)
{
  if (_mediaFile.IsValid ())
    _mediaFile->SelectLanguage (identifier);
}

THREADED_CALLABLE_IMPL(csVplPlayer, Update)
{
  while (_shouldUpdate)
  {
    if (_shouldPlay)
    {
      Play ();
      _shouldPlay=false;
    }
    if (_playing)
    {
      if (_shouldStop)
      {
        // Stop playing
        _playing=false;
        // Seek back to the beginning of the stream
      }
      if (_mediaFile.IsValid ())
      {
        if (_mediaFile->Eof ())
        {
          if (_shouldLoop) 
          {
            Seek (0.0f);
            _mediaFile->Update ();
            _playing=true;
          }
          else
            _playing=false;
        }

        _mediaFile->Update ();
      }

    }
    // If the media isn't playing, we don't want to use up the thread
    else
      csSleep (100);
  }

  return true;
}

void csVplPlayer::Loop (bool shouldLoop)
{
  _shouldLoop = shouldLoop;
}

void csVplPlayer::Play () 
{
  if (!_threadInfo.IsValid ())
  {
    _shouldPlay=true;
    return;
  }

  _playing=true;
  if (_mediaFile.IsValid ())
  {
    _mediaFile->OnPlay ();
  }
  if (_shouldStop)
  {
    _shouldStop=false;
    if (_mediaFile.IsValid ())
    {
      _mediaFile->Seek (0.0f);
      _mediaFile->OnPlay ();
    }
  }
}

void csVplPlayer::Pause () 
{
  _playing=false;
  if (_mediaFile.IsValid ())
  {
    _mediaFile->OnPause ();
  }
}

void csVplPlayer::Stop () 
{
  _shouldStop = true;
  if (_mediaFile.IsValid ())
  {
    _mediaFile->OnStop ();
  }
}

void csVplPlayer::Seek (float time)
{
  if (_mediaFile.IsValid ())
  {
    _mediaFile->Seek (time);
    _playing = true;
  }
}

float csVplPlayer::GetPosition () const
{
  return _mediaFile->GetPosition ();
}

bool csVplPlayer::IsPlaying () 
{
  return _playing;
}

float csVplPlayer::GetLength () const
{
  return _mediaFile->GetLength ();
}

void csVplPlayer::SwapBuffers ()
{
  _mediaFile->SwapBuffers ();
}

void csVplPlayer::WriteData ()
{
  _mediaFile->WriteData ();
}

float csVplPlayer::GetAspectRatio () 
{
  return _mediaFile->GetAspectRatio ();
}
