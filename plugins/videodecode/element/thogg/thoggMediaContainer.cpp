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

  //Tear everything down
  /// Clean all the streams inside the container
  for (uint i=0;i<media.GetSize ();i++)
  {
    media[i]->CleanMedia ();
  }

  ogg_sync_clear (&oy);
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
  // TO DO
  return 0;
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
    activeStreams.Push (index);
}

bool TheoraMediaContainer::RemoveActiveStream (size_t index)
{
  return activeStreams.Delete (index);
}

void TheoraMediaContainer::Update ()
{
  //if a seek is scheduled, do it
  if (timeToSeek!=-1)
  {
    DoSeek ();
    timeToSeek=-1;
    endOfFile=false;
  }
  if (!endOfFile && activeStreams.GetSize () > 0)
  {
    int ok=0;
    for (uint i=0;i<activeStreams.GetSize ();i++)
    {
      if( media [activeStreams [i]]->Update ())
        ok++;
    }

    /* buffer compressed data every loop */
    if (ok>0)
    {
      hasDataToBuffer=BufferData (&oy);
      if (hasDataToBuffer==0)
      {
        printf("Ogg buffering stopped, end of file reached.\n");
        endOfFile = true;
      }
      while (ogg_sync_pageout(&oy,&og)>0)
      {
        QueuePage (&og);
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

          ogg_stream_pagein (& buff->to ,page);
        }
      }
      else if (strcmp (media[i]->GetType (),"TheoraAudio")==0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (this->GetMedia (i) ); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*>((iAudioMedia*)media);

          ogg_stream_pagein (& buff->vo ,page);
        }
      }
    }

  }
  // Otherwise, queue the page only to the active streams
  else
  {
    for (uint i=0;i<activeStreams.GetSize();i++)
    {
      if (strcmp (media[activeStreams[i]]->GetType (),"TheoraVideo")==0)
      {
        csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (this->GetMedia (activeStreams[i]) ); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);

          ogg_stream_pagein (& buff->to ,page);
        }

      }
      else if (strcmp (media[activeStreams[i]]->GetType (),"TheoraAudio")==0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (this->GetMedia (activeStreams[i]) ); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*> ( (iAudioMedia*)media);

          ogg_stream_pagein (& buff->vo ,page);
          buff->og = page;
        }
      }
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
  bool hasVideo=false;
  int videoIndex=-1;
  for (size_t i=0;i<activeStreams.GetSize ();i++)
  {
    if (strcmp (media[activeStreams[i]]->GetType (), "TheoraVideo")==0)
    {
      hasVideo=true;
      videoIndex=activeStreams[i];
    }
  }

  if (!hasVideo)
  {
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "There isn't an active video stream in the media container. Seeking not available.\n");
    return;
  }

  // If a video stream is present, seek
  csRef<TheoraVideoMedia> vidStream;
  if (strcmp (media[videoIndex]->GetType (),"TheoraVideo")==0)
  {
    csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (this->GetMedia (videoIndex) ); 
    if (media.IsValid ()) 
    { 
      vidStream = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);
    }
  }
  long frame;
  unsigned long targetFrame=(unsigned long) (vidStream->GetFrameCount () * timeToSeek / vidStream->GetLength ());

  //check if we're seeking outside the video
  if (targetFrame>vidStream->GetFrameCount ())
  {
    /*csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "Cannot seek outside the stream.\n");*/
    targetFrame = vidStream->GetFrameCount ();
    //return;
  }
  //cout<<targetFrame<<endl;

  frame = vidStream->SeekPage (targetFrame,true,&oy,mSize);
  if (frame != -1)
    vidStream->SeekPage (std::max ( (long)0,frame),false,&oy,mSize);

  float time= ((float) targetFrame/vidStream->GetFrameCount ()) *vidStream->GetLength ();

  for (size_t i=0;i<activeStreams.GetSize ();i++)
  {
    if (strcmp (media[activeStreams[i]]->GetType (),"TheoraAudio") ==0)
    {
      csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (this->GetMedia (i) ); 
      if (media.IsValid ()) 
      { 
        csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*>((iAudioMedia*)media);

        buff->Seek (time,&oy,&og,&vidStream->to);
      }
    }
  }

  //cout<<"seeked to: "<<frame<<' '<<time<<endl;
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

void TheoraMediaContainer::SetTargetTexture (csRef<iTextureHandle> &target) 
{
  for (size_t i =0;i<activeStreams.GetSize ();i++)
  {
    if (strcmp("TheoraVideo",media[activeStreams[i]]->GetType ())==0)
    {
      csRef<iVideoMedia> video = scfQueryInterface<iVideoMedia>(media[activeStreams[i]]); 
      if (video.IsValid()) 
      {
        video->SetVideoTarget (target);
      }
    }
  }

}

float TheoraMediaContainer::GetPosition () const
{
  float position = 0;
  for (size_t i =0;i<activeStreams.GetSize ();i++)
  {
    if (strcmp("TheoraVideo",media[activeStreams[i]]->GetType ())==0)
    {
      position = media[activeStreams[i]]->GetPosition ();
    }
  }

  return position;
}

float TheoraMediaContainer::GetLength () const
{
  float length = 0;
  for (size_t i =0;i<activeStreams.GetSize ();i++)
  {
    if (strcmp("TheoraVideo",media[activeStreams[i]]->GetType ())==0)
    {
      length = media[activeStreams[i]]->GetLength ();
    }
  }

  return length;
}
