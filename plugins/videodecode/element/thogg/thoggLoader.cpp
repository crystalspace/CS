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

	th_setup_info    *ts;
	ogg_packet op;
	int stateflag=0;
	ts=0;

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


				memcpy(&videoStream->to,&test,sizeof(test));
				memcpy(&videoStream->ti,&ti,sizeof(ti));
				memcpy(&videoStream->tc,&tc,sizeof(tc));
				memcpy(&videoStream->ts,&ts,sizeof(ts));
				videoStream->theora_p=1;

				videoStream->decodersStarted = false;
				videoStream->infile = infile;

				//printf("found video stream!\n");

				container->AddMedia (videoStream);

				foundVideo=true;


				//reinitialize Theora structures for next stream, if there is one
				th_comment_init (&tc);
				th_info_init (&ti);

			}else
				if (/*(got_packet==1) &&  (vorbis_processing_headers=*/ 
					(vorbis_synthesis_headerin(&vi,&vc,&op))>=0)
				{
					// it is vorbis 

					csRef<TheoraAudioMedia> audioStream;
					audioStream.AttachNew ( new TheoraAudioMedia ( (iBase*)this));

					memcpy(&audioStream->vo,&test,sizeof(test));
					memcpy(&audioStream->vi,&vi,sizeof(vi));
					memcpy(&audioStream->vc,&vc,sizeof(vc));
					audioStream->vorbis_p=1;
					audioStream->infile = infile;

					audioStream->decodersStarted = false;


					//printf("found audio stream!\n");
					container->AddMedia (audioStream);


					//reinitialize Vorbis structures for next stream, if there is one
					vorbis_info_init(&vi);
					vorbis_comment_init(&vc);
				}else
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
	int times=0;
	while (times!=container->GetMediaCount())
	{
		int ret;
		for(uint i=0;i<container->GetMediaCount();i++)
		{
			// Read extra headers for video
			if (container->GetMedia (i)->GetType () == "TheoraVideo")
			{
				csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia> (container->GetMedia (i)); 
				if (media.IsValid ()) 
				{ 
					csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*> ( (iVideoMedia*)media);

					while(buff->theora_p && (buff->theora_p<3) && (ret=ogg_stream_packetpeek (&buff->to,&op)))
					{
						if (ret<0)
						{
							printf("Error parsing Theora stream headers; corrupt stream1?\n");
							return false;
						}
						int res=th_decode_headerin (&buff->ti,&buff->tc,&buff->ts,&op);
						if (res>0)
						{
							ogg_stream_packetout (&buff->to,&op);
						}
						else
						{
							printf ("Error parsing Theora stream headers; corrupt stream?\n");
							return false;
						}
						buff->theora_p++;
						if (buff->theora_p==3)
						{
							times++;
							break;
						}
					}
				}
			}
			// Read extra headers for audio
			if( container->GetMedia(i)->GetType() == "TheoraAudio")
			{
				csRef<iAudioMedia> media = scfQueryInterface<iAudioMedia> (container->GetMedia (i)); 
				if (media.IsValid ()) 
				{ 
					csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*> ( (iAudioMedia*)media);

					while (buff->vorbis_p && (buff->vorbis_p<3) && (ret=ogg_stream_packetout (&buff->vo,&op)))
					{
						if (ret<0)
						{
							fprintf (stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
							return false;
						}
						if (vorbis_synthesis_headerin (&buff->vi,&buff->vc,&op))
						{
							fprintf (stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
							return false;
						}
						buff->vorbis_p++;
						if (buff->vorbis_p==3)
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
		int ret;
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
	memcpy (&container->oy,&oy,sizeof (oy));

	return true;
}

void thoggLoader::ComputeStreamLength (csRef<TheoraMediaContainer> container)
{
	for (int i=0;i<container->GetMediaCount ();i++)
	{
		if (container->GetMedia (i)->GetType () == "TheoraVideo")
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

					while (1)
					{
						int ret=ogg_sync_pageout (&oy, &og);
						if (ret == 0) break;
						// if page is not a theora page, skip it
						if (ogg_page_serialno (&og) != buff->to.serialno) continue;

						unsigned long granule= (unsigned long) ogg_page_granulepos (&og);
						if (granule >= 0)
						{
							mDuration= (float) th_granule_time (buff->td,granule);
							mNumFrames= (unsigned long) th_granule_frame (buff->td,granule)+1;
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
		if (container->GetMedia (i)->GetType () == "TheoraAudio")
		{
			csRef<iAudioMedia> media2 = scfQueryInterface<iAudioMedia> (container->GetMedia(i)); 
			if (media2.IsValid ()) 
			{ 
				csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*> ( (iAudioMedia*)media2);
				buff->Initialize (this->object_reg);

				float mDuration=0;
				unsigned long mNumFrames=0;

				for (int i=1;i<=10;i++)
				{
					ogg_sync_reset(&oy);
					fseek (infile,container->GetFileSize () - 4096*i,SEEK_SET);

					char *buffer = ogg_sync_buffer(&oy, 4096*i);
					int bytesRead = fread (buffer,1,4096*i,infile);
					ogg_sync_wrote (&oy, bytesRead );
					ogg_sync_pageseek (&oy,&og);

					while (1)
					{
						int ret=ogg_sync_pageout ( &oy, &og );
						if (ret == 0) break;
						// if page is not a theora page, skip it
						if (ogg_page_serialno (&og) != buff->vo.serialno) continue;

						unsigned long granule= (unsigned long) ogg_page_granulepos (&og);
						if (granule >= 0)
						{
							mDuration= (float) vorbis_granule_time (&buff->vd,granule);
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
	return true;
}

csRef<iMediaContainer> thoggLoader::LoadMedia (const char * pFileName, const char *pDescription, const char* pMediaType)
{
	csReport(object_reg, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
		"Loading Theora video '%s'.\n", pFileName);

	infile = fopen(pFileName,"rb");



	/// checking if the file exists
	if(infile==NULL)
	{
		csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, QUALIFIED_PLUGIN_NAME,
			"Unable to open '%s' for playback.\n", pFileName);

		return NULL;  
	}
	else	
	{
		csRef<TheoraMediaContainer> container;
		container.AttachNew (new TheoraMediaContainer ( (iBase*)this));

		bool res=false;

		res = StartParsing(container);
		if(res)
		{

			container->SetDescription (pDescription);

			/*csRef<iVideoMedia> media = scfQueryInterface<iVideoMedia>(mediaContainer->GetMedia(0)); 
			if(media.IsValid()) 
			{ 
			csRef<TheoraVideoMedia> buff = static_cast<TheoraVideoMedia*>((iVideoMedia*)media);
			buff->Initialize(this->object_reg);
			}*/

			/*csRef<iAudioMedia> media2 = scfQueryInterface<iAudioMedia>(mediaContainer->GetMedia(1)); 
			if(media2.IsValid()) 
			{ 
			csRef<TheoraAudioMedia> buff = static_cast<TheoraAudioMedia*>((iAudioMedia*)media2);
			buff->Initialize(this->object_reg);
			}*/

			container->Initialize (object_reg);
		}

		if(!res)
		{
			return NULL;
		}
		return container;
	}
}

