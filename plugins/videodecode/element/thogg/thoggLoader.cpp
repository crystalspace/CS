#include <cssysdef.h>
#include "thoggLoader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include <iostream>
using namespace std;

#include <theora/theoradec.h>
#include <vorbis/codec.h>

#pragma comment (lib,"../libs/libtheora_static.lib")
#pragma comment (lib,"ogg.lib")
#pragma comment (lib,"vorbis.lib")

SCF_IMPLEMENT_FACTORY (thoggLoader)

ogg_sync_state   oy;
ogg_page         og;
ogg_stream_state vo;
ogg_stream_state to;
th_info      ti;
th_comment   tc;
th_dec_ctx       *td;
th_setup_info    *ts;
vorbis_info      vi;
vorbis_dsp_state vd;
vorbis_block     vb;
vorbis_comment   vc;
th_pixel_fmt     px_fmt;

int              theora_p=0;
int              vorbis_p=0;
int              stateflag=0;

int buffer_data(FILE *in,ogg_sync_state *oy){
  char *buffer=ogg_sync_buffer(oy,4096);
  int bytes=fread(buffer,1,4096,in);
  ogg_sync_wrote(oy,bytes);
  return(bytes);
}
static int queue_page(ogg_page *page){
  if(theora_p)ogg_stream_pagein(&to,page);
  if(vorbis_p)ogg_stream_pagein(&vo,page);
  return 0;
}

csVPLvideoFormat *readTheoraHeaders(FILE *infile)
{
  csVPLvideoFormat *format = new csVPLvideoFormat();
  format->foundVid=false;

  int pp_level_max;
  int pp_level;
  int pp_inc;
  int i,j;
  ogg_packet op;

  int frames = 0;
  int dropped = 0;

  

  /* start up Ogg stream synchronization layer */
  ogg_sync_init(&oy);

  /* init supporting Vorbis structures needed in header parsing */
  vorbis_info_init(&vi);
  vorbis_comment_init(&vc);

  /* init supporting Theora structures needed in header parsing */
  th_comment_init(&tc);
  th_info_init(&ti);

  /* Ogg file open; parse the headers */
  /* Only interested in Vorbis/Theora streams */
  while(!stateflag){
    int ret=buffer_data(infile,&oy);
    if(ret==0)break;
    while(ogg_sync_pageout(&oy,&og)>0){
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if(!ogg_page_bos(&og)){
        /* don't leak the page; get it into the appropriate stream */
        queue_page(&og);
        stateflag=1;
        break;
      }

      ogg_stream_init(&test,ogg_page_serialno(&og));
      ogg_stream_pagein(&test,&og);
      ogg_stream_packetout(&test,&op);

      /* identify the codec: try theora */
      if(!theora_p && th_decode_headerin(&ti,&tc,&ts,&op)>=0)
	  {
		cout<<"video stream detected!\n";
		
		format->foundVid=true;
		format->fWidth=ti.frame_width;
		format->fHeight=ti.frame_height;
		format->pixelfmt=ti.pixel_fmt;
		format->target_bitrate=ti.target_bitrate;
		format->colorspace=ti.colorspace;

		format->FPS=(float)ti.fps_numerator/(float)ti.fps_denominator;
		format->picOffsetX=ti.pic_x;
		format->picOffsetY=ti.pic_y;
        /* it is theora */
        memcpy(&to,&test,sizeof(test));
        theora_p=1;
      }else if(!vorbis_p && vorbis_synthesis_headerin(&vi,&vc,&op)>=0)
	  {
	    ///ignoring this for now...

		cout<<"\naudio stream detected!\n";
        memcpy(&vo,&test,sizeof(test));
        vorbis_p=1;
      }else{
        /* whatever it is, we don't care about it */
		cout<<"\nother stream detected!\n";
        ogg_stream_clear(&test);
      }
    }
    /* fall through to non-bos page parsing */
  }


  
  if(!format->foundVid)
	  return NULL;

  return format;
}


thoggLoader::thoggLoader (iBase* parent) :
	scfImplementationType (this, parent),
	object_reg(0)
{
}

thoggLoader::~thoggLoader ()
{
}

bool thoggLoader::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}

csPtr<iVPLData> thoggLoader::LoadSound (const char * pFileName, const char *pDescription)
{
  printf("loading sound %s\n",pFileName);

  FILE *infile = fopen(pFileName,"rb");

  /// checking if the file exists
  if(infile==NULL)
  {
	csReport(object_reg, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
      "Unable to open '%s' for playback.\n", pFileName);

	return NULL;  
  }
  else	
  {
	thoggData *videoData = 0;
	videoData = new thoggData ((iBase*)this);

	csVPLvideoFormat *format = readTheoraHeaders (infile);

	/// check if the video is actually a theora
	if(format==NULL)
	{
	  csReport(object_reg, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
		"File '%s' is not a Theora video.\n", pFileName);
	}
	videoData->SetFormat (format);

    fclose(infile);
	return csPtr<iVPLData> (videoData);
  }
}

