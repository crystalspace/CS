#include <cssysdef.h>
#include "vplPlayer.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>

using namespace std;


SCF_IMPLEMENT_FACTORY (vplPlayer)

vplPlayer::vplPlayer (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

vplPlayer::~vplPlayer ()
{
  // Unregister the Frame event listener
  if (frameEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->RemoveListener (frameEventHandler);
  }
}

bool vplPlayer::Initialize (iObjectRegistry* r)
{
  object_reg = r;

  _mediaFile=NULL;
  _playing = false;
  return true;
}

void vplPlayer::InitializePlayer (csRef<iMediaContainer> media)
{
  if (!media.IsValid())
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
              "Media container is not valid!");
  else
    _mediaFile = media;

  CS_INITIALIZE_FRAME_EVENT_SHORTCUTS (object_reg);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));

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
}

void vplPlayer::SetActiveStream (int index) 
{
  if (_mediaFile.IsValid ())
  {
    if (index==-1)
      _mediaFile->AutoActivateStreams ();
    else
      _mediaFile->SetActiveStream (index);
  }
}

void vplPlayer::RemoveActiveStream (int index) 
{
  if (_mediaFile.IsValid ())
    _mediaFile->RemoveActiveStream (index);
}

void vplPlayer::GetTargetTexture (csRef<iTextureHandle> &target) 
{
  _mediaFile->GetTargetTexture (target);
}

//void vplPlayer::Update ()
THREADED_CALLABLE_IMPL(vplPlayer, Update)
{
  while (true)
  {
    if (_playing)
    {
      if(_shouldStop)
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
    // If the media isn't playing, we don't want to slow down the thread a bit
    else
      Sleep(100);
  }

  return true;
}

void vplPlayer::Loop (bool shouldLoop)
{
  _shouldLoop = shouldLoop;
}

void vplPlayer::Play () 
{
  _playing=true;
  if(_shouldStop)
  {
    _shouldStop=false;
    if (_mediaFile.IsValid ())
      _mediaFile->Seek (0.0f);
  }
}

void vplPlayer::Pause() 
{
  _playing=false;
}

void vplPlayer::Stop () 
{
  _shouldStop = true;
}

void vplPlayer::Seek (float time)
{
  if (_mediaFile.IsValid ())
  {
    _mediaFile->Seek (time);
    _playing = true;
  }
}

float vplPlayer::GetPosition () const
{
  return _mediaFile->GetPosition ();
}

bool vplPlayer::IsPlaying () 
{
  return _playing;
}

float vplPlayer::GetLength () const
{
  return _mediaFile->GetLength ();
}

void vplPlayer::SwapBuffers ()
{
  _mediaFile->SwapBuffers ();
}

void vplPlayer::WriteData ()
{
  _mediaFile->WriteData ();
}
