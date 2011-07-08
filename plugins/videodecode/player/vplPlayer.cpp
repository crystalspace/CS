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
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
    "Media container is not valid!");
  else
    _mediaFile = media;
}

void vplPlayer::SetActiveStream (int index) 
{
  if (_mediaFile.IsValid ())
  {
    if(index==-1)
      _mediaFile->AutoActivateStreams ();
    else
      _mediaFile->SetActiveStream (index);
  }
}

void vplPlayer::RemoveActiveAudioStream (int index) 
{
  if (_mediaFile.IsValid ())
    _mediaFile->RemoveActiveStream (index);
}

void vplPlayer::SetTargetTexture (csRef<iTextureHandle> &target) 
{
  _mediaFile->SetTargetTexture (target);
}

void vplPlayer::Update ()
{
  if (_playing)
  {
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
}

void vplPlayer::Loop (bool shouldLoop)
{
  _shouldLoop = shouldLoop;
}

void vplPlayer::Play () 
{
  _playing=true;
}

void vplPlayer::Pause() 
{
  _playing=false;
}

void vplPlayer::Stop () 
{
  // Stop playing
  _playing=false;
  // Seek back to the beginning of the stream
  _mediaFile->Seek (0.0f);
}

void vplPlayer::Seek (float time)
{
  if (_mediaFile.IsValid ())
  {
    _mediaFile->Seek (time);
    _playing = true;
  }
}

csTicks vplPlayer::GetPosition () 
{
  return 0;
}

bool vplPlayer::IsPlaying () 
{
  return _playing;
}