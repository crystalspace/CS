#include <cssysdef.h>
#include "theoraloader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>
using namespace std;

#include "theora/theoradec.h"
#include <vorbis/codec.h>


SCF_IMPLEMENT_FACTORY (csThOggLoader)

csThOggLoader::csThOggLoader (iBase* parent) :
scfImplementationType (this, parent),
_object_reg(0)
{
}

csThOggLoader::~csThOggLoader ()
{
}

int csThOggLoader::BufferData (ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer (oy,4096);
  int bytes=fread (buffer,1,4096,_infile);
  ogg_sync_wrote (oy,bytes);
  return (bytes);
}

bool csThOggLoader::StartParsing (csRef<TheoraMediaContainer> container)
{
  /* start up Ogg stream synchronization layer */
  ogg_sync_init (&_oy);

  /* init supporting Vorbis structures needed in header parsing */
  vorbis_info_init (&_vi);
  vorbis_comment_init (&_vc);

  /* init supporting Theora structures needed in header parsing */
  th_comment_init (&tc);
  th_info_init (&ti);

  /* Copy the file pointer in the container */
  container->_infile=_infile;

  return ParseHeaders (container);
}

bool csThOggLoader::ParseHeaders (csRef<TheoraMediaContainer> container)
{
  fseek (_infile,0,SEEK_END);
  unsigned long mSize = ftell (_infile);
  fseek (_infile,0,SEEK_SET);
  container->SetFileSize (mSize);

  th_setup_info *ts=0;
  ogg_packet op;
  int stateflag=0;

  //since we're loading a Theora video, we expect video to be present
  //otherwise, everything will be dropped.
  bool foundVideo=false;

  /* Parse the headers */
  /* Only interested in Vorbis/Theora streams */

  while (!stateflag)
  {
    int ret=BufferData (&_oy);
    if (ret==0)
      break;

    while (ogg_sync_pageout (&_oy,&_og)>0)
    {
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if (!ogg_page_bos (&_og))
      {
        /* don't leak the page; get it into the appropriate stream */
        container->QueuePage (&_og);

        stateflag=1;
        break;
      }

      ogg_stream_init (&test,ogg_page_serialno (&_og));
      ogg_stream_pagein (&test,&_og);
      ogg_stream_packetout (&test,&op);

      /* identify the codec: try theora */
      if (th_decode_headerin (&ti,&tc,&ts,&op)>=0)
      {
        // it is theora 
        csRef<csTheoraVideoMedia> videoStream;
        videoStream.AttachNew ( new csTheoraVideoMedia ( (iBase*)this));

        videoStream->InitializeStream (test, ti, tc, ts, _infile, _texManager);

        container->AddMedia (videoStream);

        foundVideo=true;

        //reinitialize Theora structures for next stream, if there is one
        th_comment_init (&tc);
        th_info_init (&ti);
      }
      else
        if ( (vorbis_synthesis_headerin (&_vi,&_vc,&op))>=0)
        {
          // it is vorbis 
          csRef<csTheoraAudioMedia> audioStream;
          audioStream.AttachNew ( new csTheoraAudioMedia ( (iBase*)this));
          
          audioStream->InitializeStream (test, _vi, _vc, _infile);

          container->AddMedia (audioStream);

          //reinitialize Vorbis structures for next stream, if there is one
          vorbis_info_init (&_vi);
          vorbis_comment_init (&_vc);
        }
        else
        {
          /* whatever it is, we don't care about it */
          ogg_stream_clear (&test);
        }
    }

  }

  // if there isn't a Theora video stream in the file, we don't care anymore
  if (!foundVideo)
  {
    //clear the sync state
    ogg_sync_clear (&_oy);

    //post a warning
    csReport (_object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "Unable to find a Theora video stream.\n");

    return false;
  }


  // Next, parse secondary headers.
  size_t times=0;
  while (times!=container->GetMediaCount ())
  {
    int ret;
    for (uint i=0;i<container->GetMediaCount ();i++)
    {
      // Read extra headers for video
      if (strcmp (container->GetMedia (i)->GetType (), "TheoraVideo")==0)
      {
        csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (container->GetMedia (i)); 
        if (media.IsValid ()) 
        { 
          csRef<csTheoraVideoMedia> buff = static_cast<csTheoraVideoMedia*> ( (iVideoMedia*)media);

          while (buff->Theora_p () && (buff->Theora_p ()<3) && (ret=ogg_stream_packetpeek (buff->StreamState (),&op)))
          {
            if (ret<0)
            {
              csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
                "Error parsing Theora stream headers; corrupt stream?\n");
              return false;
            }
            int res=th_decode_headerin (buff->StreamInfo (), buff->StreamComments (),buff->SetupInfo (),&op);
            if (res>0)
            {
              ogg_stream_packetout (buff->StreamState (),&op);
            }
            else
            {
              csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
                "Error parsing Theora stream headers; corrupt stream?\n");
              return false;
            }
            buff->Theora_p ()++;
            if (buff->Theora_p ()==3)
            {

              th_comment_clear (&tc);
              th_info_clear (&ti);
              times++;
              break;
            }
          }

        }
      }
      // Read extra headers for audio
      if (strcmp (container->GetMedia (i)->GetType (), "TheoraAudio") == 0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (container->GetMedia (i)); 
        if (media.IsValid ()) 
        { 
          csRef<csTheoraAudioMedia> buff = static_cast<csTheoraAudioMedia*> ( (iAudioMedia*)media);

          while (buff->Vorbis_p () && (buff->Vorbis_p ()<3) && (ret=ogg_stream_packetout (buff->StreamState (),&op)))
          {
            if (ret<0)
            {
              csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
                "Error parsing Vorbis stream headers; corrupt stream?\n");
              return false;
            }
            if (vorbis_synthesis_headerin (buff->StreamInfo (),buff->StreamComments (),&op))
            {
              csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
                "Error parsing Vorbis stream headers; corrupt stream?\n");
              return false;
            }
            buff->Vorbis_p ()++;
            if (buff->Vorbis_p ()==3)
            {

              vorbis_comment_clear (&_vc);
              vorbis_info_clear(&_vi);
              times++;
              break;
            }
          }
        }
      }
    }

    if (ogg_sync_pageout (&_oy,&_og)>0)
    {
      container->QueuePage (&_og); /* demux into the appropriate stream */
    }
    else
    {
      int ret=BufferData (&_oy);
      if (ret==0)
      {
        csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
          "End of file while searching for codec headers.\n");
        return false;
      }
    }
  }

  //Compute the length in seconds and frame count (where applicable) of the streams
  ComputeStreamLength (container);

  //In order to get everything back in order, we need to reparse the headers
  //Streams aren't recreated, but we need to get the sync state in the proper position
  stateflag=0;
  ts=0;

  /* Parse the headers */
  while (!stateflag)
  {
    int ret=BufferData (&_oy);
    if (ret==0)
      break;

    while (ogg_sync_pageout (&_oy,&_og)>0)
    {
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if (!ogg_page_bos (&_og))
      {
        /* don't leak the page; get it into the appropriate stream */
        container->QueuePage (&_og);

        stateflag=1;
        break;
      }

      ogg_stream_init (&test,ogg_page_serialno (&_og));
      ogg_stream_pagein (&test,&_og);
      ogg_stream_packetout (&test,&op);

      ogg_stream_clear (&test);
    }
  }

  // Next, parse secondary headers.
  times=0;
  while (times!=container->GetMediaCount ())
  {
    for (uint i=0;i<container->GetMediaCount ();i++)
    {

      times++;
      if (ogg_sync_pageout (&_oy,&_og)>0)
      {
        container->QueuePage (&_og); /* demux into the appropriate stream */
      }
      else
      {
        int ret=BufferData (&_oy);
        if (ret==0)
        {
          csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
            "End of file while searching for codec headers.\n");
          return false;
        }
      }
    }

  }

  //copy the ogg sync state to the container
  memcpy (&container->_syncState,&_oy,sizeof (_oy));

  return true;
}

void csThOggLoader::ComputeStreamLength (csRef<TheoraMediaContainer> container)
{
  for (size_t i=0;i<container->GetMediaCount ();i++)
  {
    if (strcmp (container->GetMedia (i)->GetType (), "TheoraVideo")==0)
    {
      csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (container->GetMedia (i)); 
      if (media.IsValid ()) 
      { 
        csRef<csTheoraVideoMedia> buff = static_cast<csTheoraVideoMedia*> ( (iVideoMedia*)media);
        buff->Initialize (this->_object_reg);

        float mDuration=0;
        unsigned long mNumFrames=0;
        ogg_sync_reset (&_oy);
        //fseek(infile,currentPos,SEEK_SET);

        for (int i=1;i<=10;i++)
        {
          ogg_sync_reset (&_oy);
          fseek (_infile,container->GetFileSize () - 4096*i,SEEK_SET);

          char *buffer = ogg_sync_buffer (&_oy, 4096*i);
          int bytesRead = fread (buffer,1,4096*i,_infile);
          ogg_sync_wrote (&_oy, bytesRead );
          ogg_sync_pageseek (&_oy,&_og);

          while (true)
          {
            int ret=ogg_sync_pageout (&_oy, &_og);
            if (ret == 0) break;
            // if page is not a theora page, skip it
            if (ogg_page_serialno (&_og) != buff->StreamState ()->serialno) continue;

            unsigned long granule= (unsigned long) ogg_page_granulepos (&_og);
            if (granule >= 0)
            {
              mDuration= (float) th_granule_time (buff->DecodeControl (),granule);
              mNumFrames= (unsigned long) th_granule_frame (buff->DecodeControl (),granule)+1;
            }
          }

          if (mDuration > 0) break;
        }

        buff->SetLength (mDuration);
        buff->SetFrameCount (mNumFrames);

        ogg_sync_reset (&_oy);
        fseek (_infile,0,SEEK_SET);
      }
    }
    if (strcmp (container->GetMedia (i)->GetType (),"TheoraAudio")==0 )
    {
      csRef<iAudioMedia> media2 = scfQueryInterface<iAudioMedia> (container->GetMedia (i)); 
      if (media2.IsValid ()) 
      { 
        csRef<csTheoraAudioMedia> buff = static_cast<csTheoraAudioMedia*> ( (iAudioMedia*)media2);
        buff->Initialize (this->_object_reg);

        float mDuration=0;
//        unsigned long mNumFrames=0;

        for (int i=1;i<=10;i++)
        {
          ogg_sync_reset (&_oy);
          fseek (_infile,container->GetFileSize () - 4096*i,SEEK_SET);

          char *buffer = ogg_sync_buffer (&_oy, 4096*i);
          int bytesRead = fread (buffer,1,4096*i,_infile);
          ogg_sync_wrote (&_oy, bytesRead );
          ogg_sync_pageseek (&_oy,&_og);

          while (true)
          {
            int ret=ogg_sync_pageout ( &_oy, &_og );
            if (ret == 0) break;
            // if page is not a theora page, skip it
            if (ogg_page_serialno (&_og) != buff->StreamState ()->serialno) continue;

            unsigned long granule= (unsigned long) ogg_page_granulepos (&_og);
            if (granule >= 0)
            {
              mDuration= (float) vorbis_granule_time (buff->DspState (),granule);
              // mNumFrames=(unsigned long) th_granule_frame(&buff->vd,granule)+1;
            }
          }

          if (mDuration > 0) break;
        }

        buff->SetLength (mDuration);

        ogg_sync_reset (&_oy);
        fseek (_infile,0,SEEK_SET);
      }
    }
  }

}

bool csThOggLoader::Initialize (iObjectRegistry* r)
{
  _object_reg = r;
  _infile = NULL;


  return true;
}

csRef<iMediaContainer> csThOggLoader::LoadMedia (const char * pFileName, const char *pDescription)
{
  _g3d=csQueryRegistry<iGraphics3D> (_object_reg);

  _texManager = _g3d->GetTextureManager ();

  csReport (_object_reg, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
    "Loading Theora video from '%s'.\n", pFileName);

  // Get an iMediaParser from the object registry

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (_object_reg);
  // Get the path for the video
  csRef<iDataBuffer> vidPath = vfs->GetRealPath (_path.GetData ());
  _infile = fopen (vidPath->GetData (),"rb");

  // checking if the file exists
  if (_infile==NULL)
  {
    csReport (_object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "Unable to open '%s' for playback.\n", pFileName);

    return NULL;  
  }
  else	
  {
    csRef<TheoraMediaContainer> container;
    container.AttachNew (new TheoraMediaContainer ( (iBase*)this));
    container->Initialize (_object_reg);
    container->SetLanguages (_languages);

    bool res=false;
    res = StartParsing (container);

    if (!res)
      return NULL;

    container->SetDescription (pDescription);
    return container;
  }
}

void csThOggLoader::Create (csString path,csArray<Language> languages)
{
  this->_path = path;
  this->_languages = languages;
}