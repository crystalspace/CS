#include <cssysdef.h>
#include "thoggLoader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>
using namespace std;

#include "theora/theoradec.h"
#include <vorbis/codec.h>


SCF_IMPLEMENT_FACTORY (thoggLoader)

thoggLoader::thoggLoader (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

thoggLoader::~thoggLoader ()
{
  /// Clear theora/vorbis components we no longer need
  vorbis_comment_clear(&vc);
  vorbis_info_clear(&vi);
  th_comment_clear(&tc);
  th_info_clear(&ti);
}

int thoggLoader::BufferData (ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer (oy,4096);
  int bytes=fread (buffer,1,4096,infile);
  ogg_sync_wrote (oy,bytes);
  return (bytes);
}

bool thoggLoader::StartParsing (csRef<TheoraMediaContainer> container)
{
  /* start up Ogg stream synchronization layer */
  ogg_sync_init(&oy);

  /* init supporting Vorbis structures needed in header parsing */
  vorbis_info_init(&vi);
  vorbis_comment_init(&vc);

  /* init supporting Theora structures needed in header parsing */
  th_comment_init (&tc);
  th_info_init (&ti);

  /* Copy the file pointer in the container */
  container->infile=infile;

  return ParseHeaders (container);
}

bool thoggLoader::ParseHeaders (csRef<TheoraMediaContainer> container)
{
  fseek(infile,0,SEEK_END);
  unsigned long mSize = ftell(infile);
  fseek(infile,0,SEEK_SET);
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
    int ret=BufferData (&oy);
    if (ret==0)
      break;

    while (ogg_sync_pageout(&oy,&og)>0)
    {
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if(!ogg_page_bos(&og))
      {
        /* don't leak the page; get it into the appropriate stream */
        container->QueuePage (&og);

        stateflag=1;
        break;
      }

      ogg_stream_init(&test,ogg_page_serialno(&og));
      ogg_stream_pagein (&test,&og);
      ogg_stream_packetout (&test,&op);

      /* identify the codec: try theora */
      if (th_decode_headerin(&ti,&tc,&ts,&op)>=0)
      {
        // it is theora 
        csRef<TheoraVideoMedia> videoStream;
        videoStream.AttachNew ( new TheoraVideoMedia ( (iBase*)this));

        videoStream->InitializeStream (test, ti, tc, ts, infile, texManager);

        container->AddMedia (videoStream);

        foundVideo=true;

        //reinitialize Theora structures for next stream, if there is one
        th_comment_init (&tc);
        th_info_init (&ti);
      }
      else
        if (/*(got_packet==1) &&  (vorbis_processing_headers=*/ 
           (vorbis_synthesis_headerin(&vi,&vc,&op))>=0)
        {
          // it is vorbis 
          csRef<TheoraAudioMedia> audioStream;
          audioStream.AttachNew ( new TheoraAudioMedia ( (iBase*)this));
          
          audioStream->InitializeStream (test, vi, vc, infile);

          container->AddMedia (audioStream);

          //reinitialize Vorbis structures for next stream, if there is one
          vorbis_info_init(&vi);
          vorbis_comment_init(&vc);
        }
        else
        {
          //cout<<"other stream detected!\n";
          /* whatever it is, we don't care about it */
          ogg_stream_clear(&test);
        }
    }

  }

  /// if there isn't a Theora video stream in the file, we don't care anymore
  if (!foundVideo)
  {
    //clear the sync state
    ogg_sync_clear (&oy);

    //post a warning
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "Unable to find a Theora video stream.\n");

    return false;
  }


  /// Next, parse secondary headers.
  size_t times=0;
  while (times!=container->GetMediaCount())
  {
    int ret;
    for (uint i=0;i<container->GetMediaCount();i++)
    {
      // Read extra headers for video
      if (strcmp (container->GetMedia (i)->GetType (), "TheoraVideo")==0)
      {
        csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (container->GetMedia (i)); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);

          while (buff->Theora_p() && (buff->Theora_p()<3) && (ret=ogg_stream_packetpeek (buff->StreamState(),&op)))
          {
            if (ret<0)
            {
              printf("Error parsing Theora stream headers; corrupt stream?\n");
              return false;
            }
            int res=th_decode_headerin (buff->StreamInfo(), buff->StreamComments(),buff->SetupInfo(),&op);
            if (res>0)
            {
              ogg_stream_packetout (buff->StreamState(),&op);
            }
            else
            {
              printf ("Error parsing Theora stream headers; corrupt stream?\n");
              return false;
            }
            buff->Theora_p()++;
            if (buff->Theora_p()==3)
            {
              times++;
              break;
            }
          }

        }
      }
      // Read extra headers for audio
      if (strcmp (container->GetMedia(i)->GetType(), "TheoraAudio") == 0)
      {
        csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (container->GetMedia (i)); 
        if (media.IsValid ()) 
        { 
          csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*> ( (iAudioMedia*)media);

          while (buff->Vorbis_p () && (buff->Vorbis_p ()<3) && (ret=ogg_stream_packetout (buff->StreamState (),&op)))
          {
            if (ret<0)
            {
              fprintf (stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
              return false;
            }
            if (vorbis_synthesis_headerin (buff->StreamInfo (),buff->StreamComments (),&op))
            {
              fprintf (stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
              return false;
            }
            buff->Vorbis_p ()++;
            if (buff->Vorbis_p ()==3)
            {
              times++;
              break;
            }
          }
        }
      }
    }

    if (ogg_sync_pageout (&oy,&og)>0)
    {
      container->QueuePage (&og); /* demux into the appropriate stream */
    }
    else
    {
      int ret=BufferData (&oy);
      if (ret==0)
      {
        fprintf(stderr,"End of file while searching for codec headers.\n");
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
    int ret=BufferData (&oy);
    if (ret==0)
      break;

    while (ogg_sync_pageout(&oy,&og)>0)
    {
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if (!ogg_page_bos (&og))
      {
        /* don't leak the page; get it into the appropriate stream */
        container->QueuePage (&og);

        stateflag=1;
        break;
      }

      ogg_stream_init (&test,ogg_page_serialno (&og));
      ogg_stream_pagein (&test,&og);
      ogg_stream_packetout (&test,&op);

      ogg_stream_clear (&test);
    }
  }

  /// Next, parse secondary headers.
  times=0;
  while (times!=container->GetMediaCount ())
  {
    for (uint i=0;i<container->GetMediaCount ();i++)
    {

      times++;
      if (ogg_sync_pageout (&oy,&og)>0)
      {
        container->QueuePage (&og); /* demux into the appropriate stream */
      }
      else
      {
        int ret=BufferData (&oy);
        if (ret==0)
        {
          fprintf (stderr,"End of file while searching for codec headers.\n");
          return false;
        }
      }
    }

  }

  //copy the ogg sync state to the container
  memcpy (&container->_syncState,&oy,sizeof (oy));

  return true;
}

void thoggLoader::ComputeStreamLength (csRef<TheoraMediaContainer> container)
{
  for (size_t i=0;i<container->GetMediaCount ();i++)
  {
    if (strcmp (container->GetMedia (i)->GetType (), "TheoraVideo")==0)
    {
      csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (container->GetMedia (i)); 
      if (media.IsValid ()) 
      { 
        csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);
        buff->Initialize (this->object_reg);

        float mDuration=0;
        unsigned long mNumFrames=0;
        ogg_sync_reset (&oy);
        //fseek(infile,currentPos,SEEK_SET);

        for (int i=1;i<=10;i++)
        {
          ogg_sync_reset (&oy);
          fseek (infile,container->GetFileSize () - 4096*i,SEEK_SET);

          char *buffer = ogg_sync_buffer (&oy, 4096*i);
          int bytesRead = fread (buffer,1,4096*i,infile);
          ogg_sync_wrote (&oy, bytesRead );
          ogg_sync_pageseek (&oy,&og);

          while (true)
          {
            int ret=ogg_sync_pageout (&oy, &og);
            if (ret == 0) break;
            // if page is not a theora page, skip it
            if (ogg_page_serialno (&og) != buff->StreamState()->serialno) continue;

            unsigned long granule= (unsigned long) ogg_page_granulepos (&og);
            if (granule >= 0)
            {
              mDuration= (float) th_granule_time (buff->DecodeControl(),granule);
              mNumFrames= (unsigned long) th_granule_frame (buff->DecodeControl(),granule)+1;
            }
          }

          if (mDuration > 0) break;
        }

        cout<<"Media file duration: "<<mDuration<<" Frame count: "<<mNumFrames<<endl;

        buff->SetLength (mDuration);
        buff->SetFrameCount (mNumFrames);

        ogg_sync_reset (&oy);
        fseek (infile,0,SEEK_SET);
      }
    }
    if (strcmp (container->GetMedia (i)->GetType (),"TheoraAudio")==0 )
    {
      csRef<iAudioMedia> media2 = scfQueryInterface<iAudioMedia> (container->GetMedia(i)); 
      if (media2.IsValid ()) 
      { 
        csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*> ( (iAudioMedia*)media2);
        buff->Initialize (this->object_reg);

        float mDuration=0;
//        unsigned long mNumFrames=0;

        for (int i=1;i<=10;i++)
        {
          ogg_sync_reset(&oy);
          fseek (infile,container->GetFileSize () - 4096*i,SEEK_SET);

          char *buffer = ogg_sync_buffer(&oy, 4096*i);
          int bytesRead = fread (buffer,1,4096*i,infile);
          ogg_sync_wrote (&oy, bytesRead );
          ogg_sync_pageseek (&oy,&og);

          while (true)
          {
            int ret=ogg_sync_pageout ( &oy, &og );
            if (ret == 0) break;
            // if page is not a theora page, skip it
            if (ogg_page_serialno (&og) != buff->StreamState ()->serialno) continue;

            unsigned long granule= (unsigned long) ogg_page_granulepos (&og);
            if (granule >= 0)
            {
              mDuration= (float) vorbis_granule_time (buff->DspState (),granule);
              // mNumFrames=(unsigned long) th_granule_frame(&buff->vd,granule)+1;
            }
          }

          if (mDuration > 0) break;
        }

        cout<<"Audio duration: "<<mDuration<<endl;

        buff->SetLength (mDuration);

        ogg_sync_reset (&oy);
        fseek (infile,0,SEEK_SET);
      }
    }
  }

}

bool thoggLoader::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  infile = NULL;

  _g3d=csQueryRegistry<iGraphics3D> (object_reg);

  texManager = _g3d->GetTextureManager ();

  return true;
}

csRef<iMediaContainer> thoggLoader::LoadMedia (const char * pFileName, const char *pDescription, const char* pMediaType)
{
  csReport(object_reg, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
    "Loading Theora video '%s'.\n", pFileName);
  
  csRef<iDocumentAttribute> videoName;
  /// Parse XML

  /// Read the xml file and create the document
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDocumentSystem> docSys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> xmlDoc = docSys->CreateDocument ();
  csRef<iDataBuffer> xmlData = vfs->ReadFile (pFileName);

  /// Start parsing
  if (xmlDoc->Parse (xmlData) == 0)
  {
    /// Get the root
    csRef<iDocumentNode> node = xmlDoc->GetRoot ();

    csRef<iDocumentNodeIterator> it = node->GetNodes ();

    /// Iterate through the nodes
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();

      /// The <media> tag
      if ( strcmp (child->GetValue (),"media")==0)
      {
        if ( strcmp (child->GetAttributeValue ("type"),"video") ==0)
        {
          cout<<"video file detected!\n";
        }
        else
        {
          cout<<"not video file!\n";
          break;
        }
      }

      csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
      while (it2->HasNext ())
      {
        csRef<iDocumentNode> child2 = it2->Next ();
        cout<<"node: "<<child2->GetValue ()<<endl;

        /// Get the video stream path
        if (strcmp(child2->GetValue (),"videoStream")==0)
        {
          cout<<"video file path: "<<child2->GetAttributeValue ("path")<<endl;
          /// Save the video path
          videoName = child2->GetAttribute ("path");
        }
        /// Get all the languages
        if (strcmp(child2->GetValue (),"audioStream")==0)
        {
          csRef<iDocumentNodeIterator> it3 = child2->GetNodes ();

          /// Read the name and path for each language
          while (it3->HasNext ())
          {
            csRef<iDocumentNode> child3 = it3->Next ();
            if (strcmp(child3->GetValue (),"language")==0)
            {
              cout<<"lang name: "<<child3->GetAttributeValue ("name")<<
                " lang path: "<<child3->GetAttributeValue ("path")<<endl;
            }
          }
        }
      }
    }

    cout<<"done parsing xml\n";
  }
  else
    cout<<"fail"<<endl;
  //return NULL;

  csRef<iDataBuffer> vidPath = vfs->GetRealPath (videoName->GetValue ());
  infile = fopen (vidPath->GetData (),"rb");

  /// checking if the file exists
  if (infile==NULL)
  {
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "Unable to open '%s' for playback.\n", pFileName);

    return NULL;  
  }
  else	
  {
    csRef<TheoraMediaContainer> container;
    container.AttachNew (new TheoraMediaContainer ( (iBase*)this));
    container->Initialize (object_reg);

    bool res=false;
    res = StartParsing(container);

    if (!res)
      return NULL;

    container->SetDescription (pDescription);
    return container;
  }
}

