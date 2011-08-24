#include <cssysdef.h>
#include "theoramediacontainer.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>


SCF_IMPLEMENT_FACTORY (TheoraMediaContainer)

TheoraMediaContainer::TheoraMediaContainer (iBase* parent) :
scfImplementationType (this, parent),
_object_reg (0)
{
  _hasDataToBuffer=1;
}

TheoraMediaContainer::~TheoraMediaContainer ()
{
  // Close the media file
  fclose (_infile);

  // Tear everything down
  // Clean all the streams inside the container
  for (uint i=0;i<_media.GetSize ();i++)
  {
    _media[i]->CleanMedia ();
  }

  ogg_sync_clear (&_syncState);
}

bool TheoraMediaContainer::Initialize (iObjectRegistry* r)
{
  _object_reg=r;
  _endOfFile = false;
  _timeToSeekTo = -1;
  _waitToFillCache=true;
  _target = NULL;
  _canWrite=false;

  return 0;
}
size_t TheoraMediaContainer::GetMediaCount () const
{
  return _media.GetSize ();
}
void TheoraMediaContainer::SetLanguages (csArray<Language> languages)
{
  this->_languages = languages;
}

csRef<iMedia> TheoraMediaContainer::GetMedia (size_t index)
{
  if (index < _media.GetSize ())
    return _media[index];
  return NULL;
}

const char* TheoraMediaContainer::GetDescription () const
{
  return _pDescription;
}

void TheoraMediaContainer::AddMedia (csRef<iMedia> media)
{
  this->_media.Push (media);
}

void TheoraMediaContainer::SetActiveStream (size_t index)
{
  if (index >= _media.GetSize ())
    return;

  bool found = false;
  for (uint i=0;i<_activeStreams.GetSize ();i++)
  {
    if (_media [_activeStreams [i]]->GetType () == _media [index]->GetType ())
    {
      found = true;
      _activeStreams[i] = index;
      _media[index]->SetCacheSize (_cacheSize);
    }
  }

  if (!found)
  {
    _activeStreams.Push (index);

    _media[index]->SetCacheSize (_cacheSize);
  }

  // Store the activated stream in our references, for fast, full access
  if ( strcmp(_media [index]->GetType (),"TheoraVideo") == 0)
  {
    csRef<iVideoMedia> stream = scfQueryInterface<iVideoMedia> ( _media [index] ); 
    if (stream.IsValid ()) 
    { 
      _activeTheoraStream = static_cast<csTheoraVideoMedia*> ( (iVideoMedia*)stream);
    }
  }

  if ( strcmp(_media [index]->GetType (),"TheoraAudio") == 0)
  {
    csRef<iAudioMedia> stream = scfQueryInterface<iAudioMedia> ( _media [index] ); 
    if (stream.IsValid ()) 
    { 
      _activeVorbisStream = static_cast<csTheoraAudioMedia*> ((iAudioMedia*)stream);
    }
  }
}

bool TheoraMediaContainer::RemoveActiveStream (size_t index)
{
  if ( strcmp (_media [_activeStreams [index]]->GetType (),"TheoraVideo") == 0)
  {
    _activeTheoraStream = NULL;
  }
  if ( strcmp (_media [_activeStreams [index]]->GetType (),"TheoraAudio") == 0)
  {
    _activeVorbisStream = NULL;
  }
  return _activeStreams.Delete (index);
}

void TheoraMediaContainer::DropFrame ()
{
  for (uint i=0;i<_activeStreams.GetSize ();i++)
  {
    _media [_activeStreams [i]]->DropFrame ();
  }
}
void TheoraMediaContainer::Update ()
{
  // if processingCache is true, that means we've reached the end
  // of the file, but there still is data in the caches of the 
  // active streams which needs processing
  static bool processingCache=false;

  static csTicks frameTime = 0;
  static csTicks lastTime=csGetTicks ();

  if (frameTime==0)
  if (_activeTheoraStream.IsValid ())
  {
    //HACK!: we subtract 3 from the target frame time, because otherwise,
    //it runs too slow
    frameTime = 1000/_activeTheoraStream->GetTargetFPS ();//-3;
  }

  //if a seek is scheduled, do it
  if (_timeToSeekTo!=-1)
  {
    MutexScopedLock lock (_swapMutex);
    DoSeek ();
    _timeToSeekTo=-1;
    _endOfFile=false;

    _isSeeking.NotifyOne ();
  }
  if (!_endOfFile && _activeStreams.GetSize () > 0)
  {
    _updateState=0;
    size_t cacheFull = 0;
    size_t dataAvailable = 0;
    for (size_t i=0;i<_activeStreams.GetSize ();i++)
    {
      // First, we want to know if any stream needs more data.
      // in one stream needs data, ok will be different from 0
      // If we're at the end of the file, but there's still data 
      // in the caches, we don't care if a streams needs more data
      if ( _media [_activeStreams [i]]->Update () && !processingCache)
        _updateState++;
      // Next, we want to know if all the active streams have
      // a full cache. if they do, we won't read more data until 
      // there's space left in the cache
      if ( _media [_activeStreams [i]]->IsCacheFull ())
        cacheFull++;
      // Next, we want to know if there still is data available
      // we don't want to stop updating 'til we used every frame
      if ( _media [_activeStreams [i]]->HasDataReady ())
        dataAvailable++;
    }
    if (_waitToFillCache )
      if (dataAvailable == _activeStreams.GetSize ())
      {
        _waitToFillCache=false;
      }

    /*if (ok==0 && !_waitToFillCache && !canWrite)
    {
      //canSwap=true;
    }*/
    if (!_waitToFillCache && !_canWrite && dataAvailable)
    {
      if ( (csGetTicks () - lastTime) >= (frameTime) && ( (csGetTicks () - lastTime) < (frameTime+100)))
      {
        _canSwap=true;
        lastTime=csGetTicks ();
      }
      else
      if ( ( (csGetTicks () - lastTime) >= (frameTime+100)))
      {
        DropFrame ();
        lastTime=csGetTicks ();
      }
    }

    if(processingCache && dataAvailable==0)
      _updateState++;

    /* buffer compressed data every loop */
    if (_updateState>0 && cacheFull!=_activeStreams.GetSize ())
    {
      _hasDataToBuffer=BufferData (&_syncState);
      if (_hasDataToBuffer==0)
      {
        if (dataAvailable==0)
        {
          if (_sndstream.IsValid ())
          {
            _sndstream->Pause ();
          }
          _waitToFillCache=true;
          _endOfFile = true;
          processingCache=false;
        }
        else
        {
          processingCache=true;
        }
      }
      while (ogg_sync_pageout (&_syncState,&_oggPage)>0)
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
  if (_activeStreams.GetSize () == 0)
  {
    for (uint i=0;i<_media.GetSize ();i++)
    {
      if (strcmp (_media[i]->GetType (),"TheoraVideo")==0)
      {
        csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (this->GetMedia (i) ); 
        if (media.IsValid ()) 
        { 
          csRef<csTheoraVideoMedia> buff = static_cast<csTheoraVideoMedia*> ( (iVideoMedia*)media);

          ogg_stream_pagein (buff->StreamState () ,page);
        }
      }
      else if (strcmp (_media[i]->GetType (),"TheoraAudio")==0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (this->GetMedia (i) ); 
        if (media.IsValid ()) 
        { 
          csRef<csTheoraAudioMedia> buff = static_cast<csTheoraAudioMedia*> ( (iAudioMedia*)media);

          ogg_stream_pagein ( buff->StreamState () ,page);
        }
      }
    }

  }
  // Otherwise, queue the page only to the active streams
  else
  {
    if (_activeTheoraStream.IsValid ())
    {
      ogg_stream_pagein (_activeTheoraStream->StreamState () ,page);
    }

    if (_activeVorbisStream.IsValid ())
    {
      ogg_stream_pagein (_activeVorbisStream->StreamState () ,page);
    }
  }
}

int TheoraMediaContainer::BufferData (ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer (oy,1024);
  int bytes=fread (buffer,1,1024,_infile);
  ogg_sync_wrote (oy,bytes);
  return (bytes);
}

bool TheoraMediaContainer::Eof () const
{
  return _endOfFile;
}

void TheoraMediaContainer::Seek (float time)
{
  // Seeking is triggered and will be executed at the beginning of
  // the next update
  if (time<0)
    _timeToSeekTo=0;
  else
    _timeToSeekTo=time;
  _endOfFile=false;
}

void TheoraMediaContainer::DoSeek ()
{
  // In order to seek, there needs to be an active video stream
  // This is because we first have to seek the video stream and
  // sync the rest of the streams to that frame
  // This is important, because of the nature of seeking in theora

  if (!_activeTheoraStream.IsValid ())
  {
    csReport (_object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "There isn't an active video stream in the media container. Seeking not available.\n");
    return;
  }

  // If a video stream is present, seek
  long frame;
  unsigned long targetFrame= (unsigned long) (_activeTheoraStream->GetFrameCount () * 
    _timeToSeekTo / _activeTheoraStream->GetLength ());

  // check if we're seeking outside the video
  if (targetFrame>_activeTheoraStream->GetFrameCount ())
  {
    targetFrame = _activeTheoraStream->GetFrameCount ();
  }

  frame = _activeTheoraStream->SeekPage (targetFrame,true,&_syncState,_fileSize);
  if (frame != -1)
    _activeTheoraStream->SeekPage (std::max ( (long)0,frame),false,&_syncState,_fileSize);

//  float time= ( (float) targetFrame/_activeTheoraStream->GetFrameCount ()) *_activeTheoraStream->GetLength ();


  // skip to the frame we need
  while (_activeTheoraStream->Update ()!=0)
  {
    _hasDataToBuffer=BufferData (&_syncState);
    if (_hasDataToBuffer==0)
    {
      _waitToFillCache=true;
      _endOfFile = true;
    }
    while (ogg_sync_pageout (&_syncState,&_oggPage)>0)
    {
      QueuePage (&_oggPage);
    }
  }

  if (_sndstream.IsValid ())
  {
    // We want to know if we seek past the end of the audio stream.
    // If we do seek past the end, seek to the end of the stream.
    if (_timeToSeekTo > _audioStreamLength)
      _sndstream->SetPosition (_sndstream->GetFrameCount ());
    // Ortherwise, seek to the required position
    else
      _sndstream->SetPosition (_timeToSeekTo*_sndstream->GetRenderedFormat ()->Freq);
  }
}

void TheoraMediaContainer::AutoActivateStreams ()
{
  if (_activeStreams.GetSize () == 0)
  {
    for (size_t i=0;i<_media.GetSize ();i++)
    {
      bool found = false;

      for (size_t j=0;j<_activeStreams.GetSize ();j++)
      {
        if (strcmp(_media[i]->GetType (), _media[_activeStreams[j]]->GetType ())==0)
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        SetActiveStream (i);
        _media[i]->SetCacheSize (_cacheSize);
      }
    }
  }
}

void TheoraMediaContainer::GetTargetTexture (csRef<iTextureHandle> &target) 
{
  if (_activeTheoraStream.IsValid ())
    _activeTheoraStream->GetVideoTarget (target);
  else target=NULL;
}
void TheoraMediaContainer::GetTargetAudio (csRef<iSndSysStream> &target)
{
  if (_sndstream.IsValid ())
    target = _sndstream;
  else target=NULL;
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

void TheoraMediaContainer::OnPause ()
{
  if (_sndstream.IsValid ())
  {
    _sndstream->Pause ();
  }
}

void TheoraMediaContainer::OnPlay ()
{
  if (_sndstream.IsValid ())
  {
    _sndstream->Unpause ();
  }
}

void TheoraMediaContainer::OnStop ()
{
  if (_sndstream.IsValid ())
  {
    _sndstream->Pause ();
    _sndstream->SetPosition (0);
  }
}

void TheoraMediaContainer::SwapBuffers ()
{
  static csTicks time =0;
  static csTicks total=0;

  csTicks timeTaken = csGetTicks ()-time;
  total+=timeTaken;
  MutexScopedLock lock (_swapMutex);

  // Wait until we have an item
  while (_timeToSeekTo != -1)
    _isSeeking.Wait (_swapMutex);

  time = csGetTicks ();

  if (_canSwap)
  {
    total = 0;
    _canSwap=false;
    _canWrite=true;
    for (size_t i =0;i<_activeStreams.GetSize ();i++)
    {
      _media[_activeStreams[i]]->SwapBuffers ();
    }
  }
}
void TheoraMediaContainer::WriteData ()
{
  if (_waitToFillCache)
    return;
  if (!_canSwap && _canWrite)
  {
    if (_updateState==0)
    {
      _canWrite=false;
      if (_activeTheoraStream.IsValid ())
        _activeTheoraStream->WriteData ();
      if (_activeVorbisStream.IsValid ())
        _activeVorbisStream->WriteData ();
    }
  }
}

void TheoraMediaContainer::SetCacheSize (size_t size) 
{
  _cacheSize = size;
}


float TheoraMediaContainer::GetAspectRatio () 
{
  if (_activeTheoraStream.IsValid ())
    return _activeTheoraStream->GetAspectRatio ();
  return 1;
}

void TheoraMediaContainer::SelectLanguage (const char* identifier)
{
  for (size_t i =0;i<_languages.GetSize ();i++)
  {
    if (strcmp (_languages[i].name,identifier) ==0)
    {
      csRef<iVFS> vfs = csQueryRegistry<iVFS> (_object_reg);
      csRef<iSndSysLoader> sndloader = csQueryRegistry<iSndSysLoader> (_object_reg);
      csRef<iSndSysRenderer> sndrenderer = csQueryRegistry<iSndSysRenderer> (_object_reg);

      csRef<iDataBuffer> soundbuf = vfs->ReadFile (_languages[i].path);
      if (!soundbuf)
      {
        csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
          "Can't load file %s!\n", _languages[i].path);
        return;
      }

      csRef<iSndSysData> snddata = sndloader->LoadSound (soundbuf);
      if (!snddata)
      {
        csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
          "Can't load sound %s!\n", _languages[i].path);
        return;
      }

      _sndstream = sndrenderer->CreateStream (snddata,CS_SND3D_DISABLE );
      if (!_sndstream)
      {
        csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
          "Can't create stream for %s!\n", _languages[i].path);
        return;
      }
      _sndstream->SetLoopState (CS_SNDSYS_STREAM_DONTLOOP);

      // store the audio stream length
      _audioStreamLength = _sndstream->GetFrameCount ()/_sndstream->GetRenderedFormat ()->Freq;
    }
  }
}
