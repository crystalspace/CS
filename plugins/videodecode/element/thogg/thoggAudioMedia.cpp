#include <cssysdef.h>
#include "thoggAudioMedia.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>


SCF_IMPLEMENT_FACTORY (TheoraAudioMedia)

TheoraAudioMedia::TheoraAudioMedia (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

TheoraAudioMedia::~TheoraAudioMedia ()
{
}

void TheoraAudioMedia::CleanMedia ()
{
  ogg_stream_clear(&_streamState);

  if (_decodersStarted)
  {
    vorbis_block_clear(&_vorbisBlock);
    vorbis_dsp_clear(&_dspState);
  }
  vorbis_comment_clear(&_streamComments);
  vorbis_info_clear(&_streamInfo);
  printf("audio stream is clean\n");
  fclose(_log);
}

bool TheoraAudioMedia::Initialize (iObjectRegistry* r)
{
  object_reg = r;

  // initialize the decoders
  if (_vorbis_p)
  {
    vorbis_synthesis_init(&_dspState,&_streamInfo);
    vorbis_block_init(&_dspState,&_vorbisBlock);
    printf("Ogg logical stream %ld is Vorbis %d channel %ld Hz audio.\n",
      _streamState.serialno,_streamInfo.channels,_streamInfo.rate);
    _decodersStarted = true;

    csSndSysSoundFormat format;
    format.Bits = _streamInfo.bitrate_nominal;
    format.Channels = _streamInfo.channels;
    format.Freq = _streamInfo.rate;

    if (!csInitializer::RequestPlugins (object_reg,
      CS_REQUEST_VFS,
      CS_REQUEST_PLUGIN("crystalspace.sndsys.renderer.software", iSndSysRenderer),
      CS_REQUEST_END))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.vidplaydemo",
        "Can't initialize plugins!");
      return false;
    }

    csRef<iSndSysRenderer> sndrenderer = csQueryRegistry<iSndSysRenderer> (object_reg);
    if (!sndrenderer) cout<<"Failed to locate sound renderer!\n";

    csRef<SndSysTheoraSoundData> data;
    data.AttachNew(new SndSysTheoraSoundData (this,NULL));
    _outputStream =  sndrenderer->CreateStream (data,CS_SND3D_DISABLE);


    /*csRef<SndSysBasicStream> media = scfQueryInterface<SndSysBasicStream> (stream ); 
    if (media.IsValid()) 
    { 
      _outputStream = static_cast<SndSysTheoraStream*> ( (SndSysBasicStream*)media);
    }*/
    //_outputStream.AttachNew ( new SndSysTheoraStream (&format,CS_SND3D_DISABLE));

    _log = fopen("sndlog.txt","wb");
  }
  else
  {
    /* tear down the partial vorbis setup */
    vorbis_info_clear(&_streamInfo);
    vorbis_comment_clear(&_streamComments);
    _decodersStarted = false;
  }
  return 0;
}

const char* TheoraAudioMedia::GetType () const
{
  return "TheoraAudio";
}

unsigned long TheoraAudioMedia::GetFrameCount() const
{
  return 0;
}

float TheoraAudioMedia::GetLength () const
{
  return length;
}

void TheoraAudioMedia::GetAudioTarget (csRef<iSndSysStream> &stream)
{
  stream = _outputStream;
}

double TheoraAudioMedia::GetPosition () const
{
  return _streamState.granulepos;
}

bool TheoraAudioMedia::Update ()
{
  if (cache.GetSize ()>=cacheSize)
    return false;
    
  _audiobuf_ready=false;


  while (_vorbis_p && !_audiobuf_ready)
  {
    float **pcm;
    int ret=vorbis_synthesis_pcmout(&_dspState,&pcm);
    int count = 0;

    /// ToDo: change 714 to the frame count of the video
    int numSamples = 714 * _streamInfo.channels;
    int numBytes = numSamples * sizeof(short);

    short *samples = new short[numBytes];

    /* if there's pending, decoded audio, grab it */
    if (ret>0)
    {
      int i,j;
      // int count=0;
      for (i=0;i<ret && i<(2048/_streamInfo.channels);i++)
        for (j=0;j<_streamInfo.channels;j++)
        {
          int val=(int)(pcm[j][i]*32767.f);
          if(val>32767)
            val=32767;
          if(val<-32768)
            val=-32768;
          samples[count]=val;
          count++;
          //fwrite(&val,sizeof(val),1,_log);
        }
        _audiobuf_ready=1;
        vorbis_synthesis_read(&_dspState,i);

        cachedData dataOut;

        dataOut.count=count;
        dataOut.data=samples;

        cache.Push (dataOut);

        samples=NULL;
    }
    else
    {
      /* no pending audio; is there a pending packet to decode? */
      if (ogg_stream_packetout(&_streamState,&_oggPacket)>0)
      {
        if (vorbis_synthesis(&_vorbisBlock,&_oggPacket)==0) /* test for success! */
          vorbis_synthesis_blockin(&_dspState,&_vorbisBlock);
      }
      else   /* we need more data; break out to suck in another page */
        break;
    }

  }

  if (_audiobuf_ready)
  {
    //cout<<vorbis_granule_time (&vd,ogg_page_granulepos (og))<<endl;
    return 0;
  }

  return 1;
}
void TheoraAudioMedia::DropFrame ()
{
  if(cache.GetSize ()!=0)
  {
    cache.PopTop ();
  }
}

void TheoraAudioMedia::Seek(float time, ogg_sync_state *oy,ogg_page *op,ogg_stream_state *thState)
{
  ogg_stream_reset(&_streamState);
  vorbis_synthesis_restart(&_dspState);

  //memset(op, 0, sizeof(ogg_page));

  // let's decode some pages and seek to the appropriate PCM sample
  ogg_int64_t granule=0;
  float last_page_time=time;
  while (true)
  {
    int ret=ogg_sync_pageout( oy, op );
    if (ret == 1)
    {
      int serno=ogg_page_serialno(op);
      if (serno == _streamState.serialno)
      {
        granule=ogg_page_granulepos(op);
        float g_time=(float) vorbis_granule_time(&_dspState,granule);
        if (g_time > time)
        {
          float **pcm;
          int len = vorbis_synthesis_pcmout(&_dspState,&pcm);
          if (len > 0)
            break;
          //ogg_stream_pagein(&mInfo->VorbisStreamState,&mInfo->OggPage);
          time=g_time;
          break;
        }
        last_page_time=g_time;
      }
      else ogg_stream_pagein(thState,op);
    }
    else
    {
      char *buffer = ogg_sync_buffer( oy, 4096);
      int bytesRead = fread(buffer,sizeof(char),4096,_infile);
      if (bytesRead == 0) break;
      ogg_sync_wrote( oy, bytesRead );
    }
  }

}


void TheoraAudioMedia::SwapBuffers()
{

}
void TheoraAudioMedia::InitializeStream (ogg_stream_state &state, vorbis_info &info, vorbis_comment &comments, 
                       FILE *source)
{
  memcpy(&_streamState,&state,sizeof(state));
  memcpy(&_streamInfo,&info,sizeof(info));
  memcpy(&_streamComments,&comments,sizeof(comments));
  _vorbis_p=1;
  _infile = source;

  _decodersStarted = false;
}

void TheoraAudioMedia::WriteData ()
{
  if(cache.GetSize ()!=0)
  {
    cachedData data = cache.PopTop ();
    delete data.data;
  }
}


void TheoraAudioMedia::SetCacheSize(size_t size) 
{
  cacheSize = size;
}


bool TheoraAudioMedia::HasDataReady()
{
  if(cache.GetSize ()!=0)
    return true;
  return false;
}
bool TheoraAudioMedia::IsCacheFull()
{
  if(cache.GetSize ()>=cacheSize)
    return true;
  return false;
}