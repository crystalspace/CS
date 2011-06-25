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
		fclose(out);
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
	if(theora_p)
	{
		//Clear the theora state in case it contains previous data
		td=NULL;

		td=th_decode_alloc(&ti,ts);

		//Initialize the decoders and print the info on the stream

		printf("Ogg logical stream %x is Theora %dx%d %.02f fps video\n"
			"  Frame content is %dx%d with offset (%d,%d).\n",
			to.serialno,ti.pic_width,ti.pic_height, (double)ti.fps_numerator/ti.fps_denominator,
			ti.frame_width, ti.frame_height, ti.pic_x, ti.pic_y);

		decodersStarted=true;
		videobuf_granulepos=-1;
		videobuf_time=0;
	}else
	{
		/* tear down the partial theora setup */
		th_info_clear(&ti);
		th_comment_clear(&tc);

		decodersStarted=false;
	}

	out = fopen("log.txt","wb");
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

void TheoraVideoMedia::SetVideoTarget (csRef<iTextureHandle> texture)
{
	_texture = texture;
}

double TheoraVideoMedia::GetPosition ()
{
	return videobuf_time;
}

int TheoraVideoMedia::Update ()
{
	//static int frames=0;
	videobuf_ready=false;


	while(theora_p && !videobuf_ready)
	{
		/* theora is one in, one out... */
		if(ogg_stream_packetout(&to,&op)>0)
		{
			if (skipToKeyframe)
			{
				int nSeekSkippedFrames = 0;
				int keyframe=th_packet_iskeyframe(&op);
				if (!keyframe) 
				{ 
					nSeekSkippedFrames++; 
					continue; 
				}
				skipToKeyframe=false;
				if (nSeekSkippedFrames > 0)
					cout<<+"[seek]: skipped "<<nSeekSkippedFrames<<" frames while searching for keyframe";
			}
			
			if(th_decode_packetin(td,&op,&videobuf_granulepos)>=0)
			{
				videobuf_time=th_granule_time(td,videobuf_granulepos);
				cout<<videobuf_time<<'-'<<th_granule_frame (td,videobuf_granulepos)<<endl;
				videobuf_ready=1;
			}

		}else
			break;

		//videobuf_ready=0;
	}

	if(videobuf_ready)
	{
		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(td,yuv);


		for(int pli=0;pli<3;pli++)
		{
			for(int i=0;i<yuv[pli].height;i++)
			{
				fwrite(yuv[pli].data+yuv[pli].stride*i, 1,
					yuv[pli].width, out);
			}
		}
	}

	if(videobuf_ready)
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

		for (;;)
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

						if (fineseek) continue;
						if (targetFrame-1 > frame) seek_min=(seek_min+seek_max)/2;
						else				       seek_max=(seek_min+seek_max)/2;
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
	if (return_keyframe) return (long) (granule >> ti.keyframe_granule_shift);

	ogg_stream_pagein(&to,&og);
	//granule=frame << mInfo->TheoraInfo.keyframe_granule_shift;
	th_decode_ctl(td,TH_DECCTL_SET_GRANPOS,&granule,sizeof(granule));

	skipToKeyframe=true;

	return -1;
}
