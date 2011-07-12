#include <cssysdef.h>
#include "thoggVideoMedia.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>


SCF_IMPLEMENT_FACTORY (TheoraVideoMedia)


TheoraVideoMedia::TheoraVideoMedia (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
  //Clear the theora state.
  /*td.granulepos=0;
  td.internal_decode=td.internal_encode=td.i=0;*/
}

TheoraVideoMedia::~TheoraVideoMedia ()
{
}

void TheoraVideoMedia::CleanMedia ()
{
  ogg_stream_clear(&to);

  if (decodersStarted)
  {
    th_decode_free(td);
  }

  th_comment_clear(&tc);
  th_info_clear(&ti);
  th_setup_free(ts);
  printf("video stream is clean\n");
}

bool TheoraVideoMedia::Initialize (iObjectRegistry* r)
{
  object_reg = r;

  // initialize the decoders
  if (theora_p)
  {
    //Clear the theora state in case it contains previous data
    td=NULL;

    td=th_decode_alloc(&ti,ts);

    //Initialize the decoders and print the info on the stream
    printf("Ogg logical stream %ld is Theora %dx%d %.02f fps video\n"
      "  Frame content is %dx%d with offset (%d,%d).\n",
      to.serialno,ti.pic_width,ti.pic_height, (double)ti.fps_numerator/ti.fps_denominator,
      ti.frame_width, ti.frame_height, ti.pic_x, ti.pic_y);

    decodersStarted=true;
    videobuf_granulepos=-1;
    videobuf_time=0;
    frameToSkip=-1;
  }
  else
  {
    /* tear down the partial theora setup */
    th_info_clear(&ti);
    th_comment_clear(&tc);

    decodersStarted=false;
  }

  return 0;
}

const char* TheoraVideoMedia::GetType ()
{
  return "TheoraVideo";
}

const csVPLvideoFormat *TheoraVideoMedia::GetFormat()
{
  return 0;
}

unsigned long TheoraVideoMedia::GetFrameCount()
{
  return frameCount;
}

float TheoraVideoMedia::GetLength ()
{
  return length;
}

void TheoraVideoMedia::SetVideoTarget (csRef<iTextureHandle> &texture)
{
  texture = _texture;
}

double TheoraVideoMedia::GetPosition ()
{
  return videobuf_time;
}

int clamp(int number)
{
  if (number<0)
    number=0;
  if (number>255)
    number=255;
  return number;
}

int TheoraVideoMedia::Update ()
{
  videobuf_ready=false;

  while (theora_p && !videobuf_ready)
  {
    if (ogg_stream_packetout(&to,&op)>0)
    {
      if (th_decode_packetin(td,&op,&videobuf_granulepos)>=0)
      {
        videobuf_time=th_granule_time(td,videobuf_granulepos);

        if (th_granule_frame (td,videobuf_granulepos)<frameToSkip)
        {
          videobuf_ready=false;
          return 0;
        }
        else
        {
          cout<<videobuf_time<<'-'<<th_granule_frame (td,videobuf_granulepos)<<endl;
          videobuf_ready=true;
          frameToSkip = -1;
        }
      }
    }
    else
      break;
  }

  if (videobuf_ready)
  {
    th_ycbcr_buffer yuv;
    th_decode_ycbcr_out(td,yuv);

    int y_offset=(ti.pic_x&~1)+yuv[0].stride*(ti.pic_y&~1);

    //if the video is in 4:2:0 pixel format
    if (ti.pixel_fmt==TH_PF_420)
    {
      int uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y/2);

      int size = ti.frame_width*ti.frame_height;

      size_t dstSize;
      uint8 * pixels = _texture->QueryBlitBuffer (ti.pic_x,ti.pic_y,ti.pic_width,ti.pic_height,dstSize);

      for (ogg_uint32_t y = 0 ; y < ti.frame_height ; y++)
        for (ogg_uint32_t x = 0 ; x < ti.frame_width ; x++)
        {
          //          int uvOff = uv_offset+x/2;
          int Y = (int)(yuv[0].data+y_offset+yuv[0].stride*y)[x];
          int U = (int)(yuv[1].data+uv_offset+yuv[1].stride*(y/2))[x/2];
          int V = (int)(yuv[2].data+uv_offset+yuv[2].stride*(y/2))[x/2];

          int R = (Y + 1.402f*(V-128));
          int G = (Y - 0.334f*(U-128) - 0.714f*(V-128));
          int B = (Y + 1.772f*(U-128));
          *pixels++ = (uint8)R;
          *pixels++ = (uint8)G;
          *pixels++ = (uint8)B;
          *pixels++ = 0xff;
        }

        _texture->ApplyBlitBuffer (pixels);
    }
    //if the video is in 4:2:2 pixel format
    else if (ti.pixel_fmt==TH_PF_422)
    {
      int uv_offset;

      uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y);

      int size = ti.frame_width*ti.frame_height;

      size_t dstSize;
      uint8 * pixels = _texture->QueryBlitBuffer (ti.pic_x,ti.pic_y,ti.pic_width,ti.pic_height,dstSize);

      for (ogg_uint32_t y = 0 ; y < ti.frame_height ; y++)
        for (ogg_uint32_t x = 0 ; x < ti.frame_width ; x++)
        {
          //            int uvOff = uv_offset+x/2;
          int Y = (int)(yuv[0].data+y_offset+yuv[0].stride*y)[x];
          int U = (int)(yuv[1].data+uv_offset+yuv[1].stride*(y))[x/2];
          int V = (int)(yuv[2].data+uv_offset+yuv[2].stride*(y))[x/2];

          int R = (Y + 1.402f*(V-128));
          int G = (Y - 0.334f*(U-128) - 0.714f*(V-128));
          int B = (Y + 1.772f*(U-128));
          *pixels++ = (uint8)R;
          *pixels++ = (uint8)G;
          *pixels++ = (uint8)B;
          *pixels++ = 0xff;
        }

        _texture->ApplyBlitBuffer (pixels);
    }
    //if the video is in 4:4:4 pixel format
    else if (ti.pixel_fmt==TH_PF_444)
    {
      int uv_offset;

      uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y);

      int size = ti.frame_width*ti.frame_height;

      size_t dstSize;
      uint8 * pixels = _texture->QueryBlitBuffer (ti.pic_x,ti.pic_y,ti.pic_width,ti.pic_height,dstSize);

      for (ogg_uint32_t y = 0 ; y < ti.frame_height ; y++)
        for (ogg_uint32_t x = 0 ; x < ti.frame_width ; x++)
        {
          //              int uvOff = uv_offset+x/2;
          int Y = (int)(yuv[0].data+y_offset+yuv[0].stride*y)[x];
          int U = (int)(yuv[1].data+uv_offset+yuv[1].stride*(y))[x];
          int V = (int)(yuv[2].data+uv_offset+yuv[2].stride*(y))[x];

          int R = (Y + 1.402f*(V-128));
          int G = (Y - 0.334f*(U-128) - 0.714f*(V-128));
          int B = (Y + 1.772f*(U-128));
          *pixels++ = R;
          *pixels++ = G;
          *pixels++ = B;
          *pixels++ = 0xff;
        }

        _texture->ApplyBlitBuffer (pixels);
    }
    else
      csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
      "The Theora video stream has an unsupported pixel format.\n");
  }

  if (videobuf_ready)
    return 0;
  return 1;
}

long TheoraVideoMedia::SeekPage(long targetFrame,bool return_keyframe, ogg_sync_state *oy,unsigned long fileSize)
{
  ogg_stream_reset (&to);
  th_decode_free (td);
  td=th_decode_alloc(&ti,ts);

  int i,seek_min=0, seek_max=fileSize;
  long frame;
  ogg_int64_t granule=0;
  bool fineseek=0;
  ogg_page og;

  for (i=0;i<100;i++)
  {
    ogg_sync_reset( oy );

    fseek (infile,(seek_min+seek_max)/2,SEEK_SET);
    memset(&og, 0, sizeof(ogg_page));
    ogg_sync_pageseek(oy,&og);

    while (true)
    {
      int ret=ogg_sync_pageout( oy, &og );
      if (ret == 1)
      {
        int serno=ogg_page_serialno(&og);
        if (serno == to.serialno)
        {
          granule=ogg_page_granulepos(&og);
          if (granule >= 0)
          {
            frame=(long) th_granule_frame(td,granule);
            if (frame < targetFrame-1 && targetFrame-frame < 10)
            {
              fineseek=1;
              if (!return_keyframe) break;
            }
            if (fineseek && frame >= targetFrame)
              break;

            if (fineseek) 
            {
              continue;
            }
            if (targetFrame-1 > frame) 
              seek_min=(seek_min+seek_max)/2;
            else
              seek_max=(seek_min+seek_max)/2;
            break;
          }
        }
      }
      else
      {
        char *buffer = ogg_sync_buffer( oy, 4096);
        int bytesRead = fread (buffer,1,4096,infile);
        if (bytesRead == 0) break;
        ogg_sync_wrote( oy, bytesRead );
      }
    }
    if (fineseek) break;
  }

  ogg_stream_pagein(&to,&og);
  //granule=frame << mInfo->TheoraInfo.keyframe_granule_shift;
  th_decode_ctl(td,TH_DECCTL_SET_GRANPOS,&granule,sizeof(granule));

  //make sure we skip to the keyframe we need
  if (return_keyframe)
  {
    frameToSkip = targetFrame;
    cout<<"want to skip to :"<<frameToSkip<<endl;
  }

  if (return_keyframe) 
    return (long) (granule >> ti.keyframe_granule_shift);

  return -1;
}
