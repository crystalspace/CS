#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include "thoggFile.h"

#include <iostream>
using namespace std;


SCF_IMPLEMENT_FACTORY (thoggFile)

thoggFile::thoggFile (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

thoggFile::~thoggFile ()
{
}
bool thoggFile::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  theora_p=vorbis_p=stateflag=0;
  return true;
}

/* Helper; just grab some more compressed bitstream and sync it for
   page extraction */
int thoggFile::buffer_data(ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer(oy,4096);
  int bytes=fread(buffer,1,4096,infile);
  ogg_sync_wrote(oy,bytes);
  return(bytes);
}

/* helper: push a page into the appropriate stream */
/* this can be done blindly; a stream won't accept a page
                that doesn't belong to it */
int thoggFile::queue_page(ogg_page *page)
{
  if(theora_p)ogg_stream_pagein(&to,page);
  if(vorbis_p)ogg_stream_pagein(&vo,page);
  return 0;
}

bool thoggFile::checkHeaders()
{
  format = new csVPLvideoFormat ();
  format->foundVid=false;

  int pp_level_max;
  int pp_level;
  int pp_inc;
  int i,j;
  

  /* start up Ogg stream synchronization layer */
  ogg_sync_init (&oy);

  /* init supporting Vorbis structures needed in header parsing */
  vorbis_info_init (&vi);
  vorbis_comment_init (&vc);

  /* init supporting Theora structures needed in header parsing */
  th_comment_init (&tc);
  th_info_init (&ti);

  /* Ogg file open; parse the headers */
  /* Only interested in Vorbis/Theora streams */
  while (!stateflag)
  {
    int ret=buffer_data (&oy);
    if (ret==0) break;
    while (ogg_sync_pageout (&oy,&og)>0)
	{
      ogg_stream_state test;

      /* is this a mandated initial header? If not, stop parsing */
      if (!ogg_page_bos (&og))
	  {
        /* don't leak the page; get it into the appropriate stream */
        queue_page (&og);
        stateflag=1;
        break;
      }

      ogg_stream_init (&test,ogg_page_serialno (&og));
      ogg_stream_pagein (&test,&og);
      ogg_stream_packetout (&test,&op);

      /* identify the codec: try theora */
      if (!theora_p && th_decode_headerin (&ti,&tc,&ts,&op)>=0)
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
        memcpy (&to,&test,sizeof (test));
        theora_p=1;
      }
	  else if (!vorbis_p && vorbis_synthesis_headerin (&vi,&vc,&op)>=0)
	  {
		/* it is vorbis */
	    ///ignoring this for now...

		cout<<"\naudio stream detected!\n";
        memcpy (&vo,&test,sizeof (test));
        vorbis_p=1;
      }
	  else
	  {
        /* whatever it is, we don't care about it */
		cout<<"\nother stream detected!\n";
        ogg_stream_clear (&test);
      }
    }
    /* fall through to non-bos page parsing */
  }

  
  if(!format->foundVid)
	  return false;

  return true;
}

bool thoggFile::parseSecondaryHeaders()
{
  /* we're expecting more header packets. */
  while((theora_p && theora_p<3) || (vorbis_p && vorbis_p<3))
  {
    int ret;

    /* look for further theora headers */
    while(theora_p && (theora_p<3) && (ret=ogg_stream_packetout(&to,&op)))
	{
      if(ret<0)
	  {
        fprintf(stderr,"Error parsing Theora stream headers; "
         "corrupt stream?\n");

		return false;
      }
      if(!th_decode_headerin(&ti,&tc,&ts,&op))
	  {
        fprintf(stderr,"Error parsing Theora stream headers; "
         "corrupt stream?\n");

		return false;
      }
      theora_p++;
    }

    /* look for more vorbis header packets */
    while(vorbis_p && (vorbis_p<3) && (ret=ogg_stream_packetout(&vo,&op)))
	{
      if(ret<0)
	  {
        fprintf(stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");

		return false;
      }
      if(vorbis_synthesis_headerin(&vi,&vc,&op))
	  {
        fprintf(stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");

		return false;
      }
      vorbis_p++;
      if(vorbis_p==3)break;
    }

    /* The header pages/packets will arrive before anything else we
       care about, or the stream is not obeying spec */

    if(ogg_sync_pageout(&oy,&og)>0)
	{
      queue_page(&og); /* demux into the appropriate stream */
    }else
	{
      int ret=buffer_data(&oy); /* someone needs more data */
      if(ret==0)
	  {
        fprintf(stderr,"End of file while searching for codec headers.\n");

		return false;
      }
    }
  }

  //done
  return true;
}



bool thoggFile::InitFile (FILE *infile)
{
  this->infile=infile;
  
  theora_p=vorbis_p=stateflag=0;

  bool res = checkHeaders();

  if(res)
  {
	bool res2 = this->parseSecondaryHeaders ();

	if (res2==false)
	{
	  printf("error in theora video!\n");
	  return false;
	}
  }
  if(!res)
  {
	  printf("theora video not found!\n");
  }
  return res;
}

void thoggFile::SetName (const char *name)
{
  fileName=name;
}

const char *thoggFile::GetName ()
{
  return fileName;
}

size_t thoggFile::GetSize ()
{
  return 0;
}

int thoggFile::GetStatus ()
{
  return 0;
}

size_t thoggFile::Read (char *Data, size_t DataSize)
{
  return 0;
}

size_t thoggFile::Write (const char *Data, size_t DataSize)
{
  return 0;
}

void thoggFile::Flush ()
{
}

bool thoggFile::AtEOF ()
{
  return false;
  return true;
}

size_t thoggFile::GetPos ()
{
  return 0;
}

bool thoggFile::SetPos (size_t newpos)
{
  return true;
}

csPtr<iDataBuffer> thoggFile::GetAllData (bool nullterm)
{
  return NULL;
}