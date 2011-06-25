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
	ogg_stream_clear(&vo);

	if (decodersStarted)
	{
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		fclose(out);
	}
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
	printf("audio stream is clean\n");
}

bool TheoraAudioMedia::Initialize (iObjectRegistry* r)
{
	object_reg = r;

	// initialize the decoders
	if(vorbis_p)
	{
		vorbis_synthesis_init(&vd,&vi);
		vorbis_block_init(&vd,&vb);
		printf("Ogg logical stream %x is Vorbis %d channel %d Hz audio.\n",
			vo.serialno,vi.channels,vi.rate);
		decodersStarted = true;
	}else
	{
		/* tear down the partial vorbis setup */
		vorbis_info_clear(&vi);
		vorbis_comment_clear(&vc);
		decodersStarted = false;
	}
	out = fopen("snd.txt","wb");
	return 0;
}

const char* TheoraAudioMedia::GetType ()
{
	return "TheoraAudio";
}

unsigned long TheoraAudioMedia::GetFrameCount()
{
	return 0;
}

float TheoraAudioMedia::GetLength ()
{
	return length;
}

void TheoraAudioMedia::SetAudioTarget (csRef<iSndSysStream> stream)
{
	_stream = stream;
}

double TheoraAudioMedia::GetPosition ()
{
	return vo.granulepos;
}

int TheoraAudioMedia::Update ()
{
	audiobuf_ready=false;
	while(vorbis_p && !audiobuf_ready)
	{
		float **pcm;
		int ret=vorbis_synthesis_pcmout(&vd,&pcm);

		/* if there's pending, decoded audio, grab it */
		if(ret>0)
		{
			int i,j;
			int count=0;
			for(i=0;i<ret && i<(256/vi.channels);i++)
				for(j=0;j<vi.channels;j++)
				{
					int val=(int)(pcm[j][i]*32767.f);
					if(val>32767)
						val=32767;
					if(val<-32768)
						val=-32768;
					//fwrite(&val,sizeof(val),1,out);
				}
				audiobuf_ready=1;
				vorbis_synthesis_read(&vd,i);
		}else
		{
			/* no pending audio; is there a pending packet to decode? */
			if(ogg_stream_packetout(&vo,&op)>0)
			{
				if(vorbis_synthesis(&vb,&op)==0) /* test for success! */
					vorbis_synthesis_blockin(&vd,&vb);
			}else   /* we need more data; break out to suck in another page */
				break;
		}

	}
	if(audiobuf_ready)
	{
		//cout<<vorbis_granule_time (&vd,ogg_page_granulepos (og))<<endl;
		return 0;
	}
	return 1;
}

void TheoraAudioMedia::Seek(float time, ogg_sync_state *oy,ogg_page *op,ogg_stream_state *thState)
{
	ogg_stream_reset(&vo);
	vorbis_synthesis_restart(&vd);

	//memset(op, 0, sizeof(ogg_page));

	// let's decode some pages and seek to the appropriate PCM sample
	ogg_int64_t granule=0;
	float last_page_time=time;
	for (;;)
	{
		int ret=ogg_sync_pageout( oy, op );
		if (ret == 1)
		{
			int serno=ogg_page_serialno(op);
			if (serno == vo.serialno)
			{
				granule=ogg_page_granulepos(op);
				float g_time=(float) vorbis_granule_time(&vd,granule);
				if (g_time > time)
				{
					float **pcm;
					int len = vorbis_synthesis_pcmout(&vd,&pcm);
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
			int bytesRead = fread(buffer,4096,1,infile);//->read( buffer, 4096);
			if (bytesRead == 0) break;
			ogg_sync_wrote( oy, bytesRead );
		}
	}
}
