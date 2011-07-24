#include <cssysdef.h>
#include "thoggMediaContainer.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>


SCF_IMPLEMENT_FACTORY (TheoraMediaContainer)

TheoraMediaContainer::TheoraMediaContainer (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
  hasDataToBuffer=1;
}

TheoraMediaContainer::~TheoraMediaContainer ()
{
  // Close the media file
  fclose (infile);

  // Tear everything down
  // Clean all the streams inside the container
  for (uint i=0;i<media.GetSize ();i++)
  {
    media[i]->CleanMedia ();
  }

  ogg_sync_clear (&_syncState);
}

bool TheoraMediaContainer::Initialize (iObjectRegistry* r)
{
  object_reg=r;
  endOfFile = false;
  timeToSeek = -1;
  _target = NULL;
  return 0;
}
size_t TheoraMediaContainer::GetMediaCount () const
{
  return media.GetSize ();
}

csRef<iMedia> TheoraMediaContainer::GetMedia (size_t index)
{
  if (index < media.GetSize ())
    return media[index];
  return NULL;
}

const char* TheoraMediaContainer::GetDescription () const
{
  return pDescription;
}

void TheoraMediaContainer::AddMedia (csRef<iMedia> media)
{
  this->media.Push (media);
}

void TheoraMediaContainer::SetActiveStream (size_t index)
{
  if (index >= media.GetSize ())
    return;

  bool found = false;
  for (uint i=0;i<activeStreams.GetSize ();i++)
  {
    if (media [activeStreams [i]]->GetType () == media [index]->GetType ())
    {
      found = true;
      activeStreams[i] = index;
    }
  }

  if (!found)
  {
    activeStreams.Push (index);
  }

  // Store the activated stream in our references, for fast, full access
  if ( strcmp(media [index]->GetType (),"TheoraVideo") == 0)
  {
    csRef<iVideoMedia> stream = scfQueryInterface<iVideoMedia> ( media [index] ); 
    if (stream.IsValid()) 
    { 
      _activeTheoraStream = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)stream);
    }
  }

  if ( strcmp(media [index]->GetType (),"TheoraAudio") == 0)
  {
    csRef<iAudioMedia> stream = scfQueryInterface<iAudioMedia> ( media [index] ); 
    if (stream.IsValid ()) 
    { 
      _activeVorbisStream = static_cast<TheoraAudioMedia*>((iAudioMedia*)stream);
    }
  }
}

bool TheoraMediaContainer::RemoveActiveStream (size_t index)
{
  if ( strcmp(media [activeStreams [index]]->GetType (),"TheoraVideo") == 0)
  {
    _activeTheoraStream = NULL;
  }
  if ( strcmp(media [activeStreams [index]]->GetType (),"TheoraAudio") == 0)
  {
    _activeVorbisStream = NULL;
  }
  return activeStreams.Delete (index);
}

void TheoraMediaContainer::Update ()
{
  //if a seek is scheduled, do it
  if (timeToSeek!=-1)
  {
    MutexScopedLock lock (swapMutex);
    DoSeek ();
    timeToSeek=-1;
    endOfFile=false;

    isSeeking.NotifyOne ();
  }
  if (!endOfFile && activeStreams.GetSize () > 0)
  {
    ok=0;
    for (size_t i=0;i<activeStreams.GetSize ();i++)
    {
      if( media [activeStreams [i]]->Update ())
        ok++;
    }

    if (ok==0)
    {
      canSwap=true;
    }

    /* buffer compressed data every loop */
    if (ok>0)
    {
      hasDataToBuffer=BufferData (&_syncState);
      if (hasDataToBuffer==0)
      {
        printf("Ogg buffering stopped, end of file reached.\n");
        endOfFile = true;
      }
      while (ogg_sync_pageout(&_syncState,&_oggPage)>0)
      {
        QueuePage (&_oggPage);
      }

    }
  }
}

void TheoraMediaContainer::QueuePage (ogg_page *page)
{
  // If there are no active stream (i.e. the video file is currently being loaded), 
  // queue the page to every stream
  if (activeStreams.GetSize () == 0)
  {
    for (uint i=0;i<media.GetSize();i++)
    {
      if (strcmp (media[i]->GetType(),"TheoraVideo")==0)
      {
        csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (this->GetMedia (i) ); 
        if (media.IsValid()) 
        { 
          csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);

          ogg_stream_pagein ( buff->StreamState() ,page);
        }
      }
      else if (strcmp (media[i]->GetType (),"TheoraAudio")==0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (this->GetMedia (i) ); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*>((iAudioMedia*)media);

          ogg_stream_pagein ( buff->StreamState () ,page);
        }
      }
    }

  }
  // Otherwise, queue the page only to the active streams
  else
  {
    if(_activeTheoraStream.IsValid ())
    {
      ogg_stream_pagein ( _activeTheoraStream->StreamState () ,page);
    }

    if(_activeVorbisStream.IsValid ())
    {
      ogg_stream_pagein ( _activeVorbisStream->StreamState () ,page);
    }
  }
}

int TheoraMediaContainer::BufferData (ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer (oy,4096);
  int bytes=fread (buffer,1,4096,infile);
  ogg_sync_wrote (oy,bytes);
  return (bytes);
}

bool TheoraMediaContainer::Eof () const
{
  return endOfFile;
}

void TheoraMediaContainer::Seek (float time)
{
  // Seeking is triggered and will be executed at the beginning of
  // the next update
  if (time<0)
    timeToSeek=0;
  else
    timeToSeek=time;
  endOfFile=false;
}

void TheoraMediaContainer::DoSeek ()
{
  // In order to seek, there needs to be an active video stream
  // This is because we first have to seek the video stream and
  // sync the rest of the streams to that frame
  // This is important, because of the nature of seeking in theora

  if (!_activeTheoraStream.IsValid ())
  {
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "There isn't an active video stream in the media container. Seeking not available.\n");
    return;
  }

  // If a video stream is present, seek
  long frame;
  unsigned long targetFrame=(unsigned long) (_activeTheoraStream->GetFrameCount () * 
    timeToSeek / _activeTheoraStream->GetLength ());

  //check if we're seeking outside the video
  if (targetFrame>_activeTheoraStream->GetFrameCount ())
  {
    targetFrame = _activeTheoraStream->GetFrameCount ();
  }

  frame = _activeTheoraStream->SeekPage (targetFrame,true,&_syncState,mSize);
  if (frame != -1)
    _activeTheoraStream->SeekPage (std::max ( (long)0,frame),false,&_syncState,mSize);

  float time= ((float) targetFrame/_activeTheoraStream->GetFrameCount ()) *_activeTheoraStream->GetLength ();
  
  if(_activeVorbisStream.IsValid ())
    _activeVorbisStream->Seek (time,&_syncState,&_oggPage, _activeTheoraStream->StreamState());
}

void TheoraMediaContainer::AutoActivateStreams ()
{
  if (activeStreams.GetSize () == 0)
  {
    for (size_t i=0;i<media.GetSize ();i++)
    {
      bool found = false;

      for (size_t j=0;j<activeStreams.GetSize ();j++)
      {
        if (strcmp(media[i]->GetType (), media[activeStreams[j]]->GetType ())==0)
        {
          found = true;
          break;
        }
      }

      if (!found)
        SetActiveStream (i);
    }
  }
}

void TheoraMediaContainer::GetTargetTexture (csRef<iTextureHandle> &target) 
{
  if(_activeTheoraStream.IsValid ())
    _activeTheoraStream->GetVideoTarget (target);
}

float TheoraMediaContainer::GetPosition () const
{
  if (_activeTheoraStream.IsValid ())
    return _activeTheoraStream->GetPosition ();

  return 0;
}

float TheoraMediaContainer::GetLength () const
{
  if (_activeTheoraStream.IsValid ())
    return _activeTheoraStream->GetLength ();

  return 0;
}

void TheoraMediaContainer::SwapBuffers ()
{
  MutexScopedLock lock (swapMutex);

  // Wait until we have an item
  while (timeToSeek != -1)
    isSeeking.Wait (swapMutex);

  if(canSwap)
  {
    canSwap=false;
    for (size_t i =0;i<activeStreams.GetSize ();i++)
    {
      media[activeStreams[i]]->SwapBuffers ();
    }
  }
}
void TheoraMediaContainer::WriteData ()
{
  if(ok==0)
  {
    if (_activeTheoraStream.IsValid ())
      _activeTheoraStream->WriteData ();
    if (_activeVorbisStream.IsValid ())
      _activeVorbisStream->WriteData ();
  }
}
