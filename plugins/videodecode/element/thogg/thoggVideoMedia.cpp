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

const char* TheoraVideoMedia::GetType () const
{
  return "TheoraVideo";
}

const csVPLvideoFormat *TheoraVideoMedia::GetFormat() const
{
  // TO DO
  return 0;
}

unsigned long TheoraVideoMedia::GetFrameCount() const
{
  return frameCount;
}

float TheoraVideoMedia::GetLength () const
{
  return length;
}

void TheoraVideoMedia::SetVideoTarget (csRef<iTextureHandle> &texture)
{
  texture = _texture;   // a Set method should do '_texture = texture' otherwise it's a Get method ?????
}

double TheoraVideoMedia::GetPosition () const
{
  return videobuf_time;
}

bool TheoraVideoMedia::Update ()
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
          return false;
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

  if (!videobuf_ready)
    return true;

  csRef<iTextureHandle> texToWrite = activeBuffer==1 ? _buffer2 : _buffer1;
  size_t dstSize;
  uint8* pixels = texToWrite->QueryBlitBuffer (ti.pic_x,ti.pic_y,ti.pic_width,ti.pic_height,dstSize);

  th_ycbcr_buffer yuv;
  th_decode_ycbcr_out(td,yuv);
  int y_offset=(ti.pic_x&~1)+yuv[0].stride*(ti.pic_y&~1);

  // 4:2:0 pixel format
  if (ti.pixel_fmt==TH_PF_420)
  {
    int uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y/2);

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

        // Clamping the values here is faster than calling a function
        if(R<0) R=0;
        else if(R>255) R=255;
        if(G<0) G=0;
        else if(G>255) G=255;
        if(B<0) B=0;
        else if(B>255) B=255;

        *pixels++ = (uint8)R;
        *pixels++ = (uint8)G;
        *pixels++ = (uint8)B;
        *pixels++ = 0xff;
      }

  }
  // 4:2:2 pixel format
  else if (ti.pixel_fmt==TH_PF_422)
  {
    int uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y);

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

        // Clamping the values here is faster than calling a function
        if(R<0) R=0;
        else if(R>255) R=255;
        if(G<0) G=0;
        else if(G>255) G=255;
        if(B<0) B=0;
        else if(B>255) B=255;

        *pixels++ = (uint8)R;
        *pixels++ = (uint8)G;
        *pixels++ = (uint8)B;
        *pixels++ = 0xff;
      }

  }
  // 4:4:4 pixel format
  else if (ti.pixel_fmt==TH_PF_444)
  {
    int uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y);

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

        // Clamping the values here is faster than calling a function
        if(R<0) R=0;
        else if(R>255) R=255;
        if(G<0) G=0;
        else if(G>255) G=255;
        if(B<0) B=0;
        else if(B>255) B=255;

        *pixels++ = R;
        *pixels++ = G;
        *pixels++ = B;
        *pixels++ = 0xff;
      }

  }
  else
  {
    csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
             "The Theora video stream has an unsupported pixel format.\n");
    return false;
  }

  texToWrite->ApplyBlitBuffer (pixels);
  //SwapBuffers ();

  return false;
}

long TheoraVideoMedia::SeekPage (long targetFrame,bool return_keyframe, ogg_sync_state *oy,unsigned long fileSize)
{
  ogg_stream_reset (&to);
  th_decode_free (td);
  td=th_decode_alloc(&ti,ts);

  int seek_min=0, seek_max=fileSize;
  long frame;
  ogg_int64_t granule=0;
  bool fineseek=false;
  ogg_page og;

  for (int i=0;i<100;i++)
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
              fineseek=true;
              if (!return_keyframe) break;
            }

            if (fineseek && frame >= targetFrame)
              break;

            if (fineseek) 
              continue;

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
    return (long) (granule >> ti.keyframe_granule_shift);
  }

  return -1;
}

void TheoraVideoMedia::SwapBuffers()
{
  if (activeBuffer==1)
  {
    _texture = _buffer2;
    activeBuffer = 2;
  }
  else
  {
    _texture = _buffer1;
    activeBuffer = 1;
  }
}
