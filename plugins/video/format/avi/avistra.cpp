/*
    Copyright (C) 2001 by Norman Kraemer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "avistra.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <ctype.h>

SCF_IMPLEMENT_IBASE (csAVIStreamAudio)
  SCF_IMPLEMENTS_INTERFACE (iAudioStream)
  SCF_IMPLEMENTS_INTERFACE (iStream)
SCF_IMPLEMENT_IBASE_END

csAVIStreamAudio::csAVIStreamAudio (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  pChunk = 0;
  pAVI = (csAVIFormat*)pBase;
  object_reg = 0;
  pCodec = 0;
}

bool csAVIStreamAudio::Initialize (const csAVIFormat::AVIHeader *ph,
				   const csAVIFormat::StreamHeader *psh,
				   const csAVIFormat::AudioStreamFormat *pf,
				   uint16 nStreamNumber,
				   uint8 *pInitData, uint32 nInitDataLen,
				   char *pName,
				   uint8 *pFormatEx, uint32 nFormatEx,
				   iObjectRegistry *object_reg)
{

  (void)ph;
  strdesc.type = CS_STREAMTYPE_AUDIO;
  memcpy (strdesc.codec, psh->handler, 4);
  strdesc.formattag = pf->formattag;
  strdesc.channels = pf->channels;
  strdesc.samplespersecond = pf->samplespersecond;
  strdesc.bitspersample = pf->bitspersample;
  strdesc.duration = psh->length / psh->scale;
  strdesc.name = pName;

  int i;
  for (i=3; i>=0 && strdesc.codec[i] == ' '; i--);
  strdesc.codec[i+1] = '\0';
  for (i=0; strdesc.codec[i]; i++)
    strdesc.codec[i] = tolower (strdesc.codec[i]);

  delete pChunk;
  pChunk = new csAVIFormat::AVIDataChunk;
  pChunk->currentframe = 0;
  pChunk->currentframepos = 0;
  sprintf (pChunk->id, "%02" PRIu16 "wb", nStreamNumber);
  pChunk->id[4] = '\0';

  nStream = nStreamNumber;
  csAVIStreamAudio::object_reg = object_reg;

  bTimeSynced = false;
  // load the CODEC
  return LoadCodec (pInitData, nInitDataLen, pFormatEx, nFormatEx);
}

csAVIStreamAudio::~csAVIStreamAudio ()
{
  delete pChunk;
  SCF_DESTRUCT_IBASE();
}

void csAVIStreamAudio::GetStreamDescription (csStreamDescription &desc)
{
  memcpy (&desc, (csStreamDescription*)&strdesc, sizeof (csStreamDescription));
}

bool csAVIStreamAudio::GotoFrame (uint32 frameindex)
{
  return pAVI->GetChunk (frameindex, pChunk);
}

bool csAVIStreamAudio::GotoTime (uint32 timeindex)
{
  (void)timeindex;
  // not yet implemented
  return false;
}

bool csAVIStreamAudio::SetPlayMethod (bool bTimeSynced)
{
  // timesynced isnt yet implemented, so return false if its requested.
  return bTimeSynced == false;
}

void csAVIStreamAudio::GetStreamDescription (csAudioStreamDescription &desc)
{
  memcpy (&desc, &strdesc, sizeof (csAudioStreamDescription));
}

void csAVIStreamAudio::NextFrame ()
{
  /*
  if (pAVI->GetChunk (pChunk->currentframe++, pChunk))
  {
    pCodec->Decode (pChunk->data, pChunk->length, outdata);
    if (pCodecDesc->decodeoutput != CS_CODECFORMAT_PCM)
      return;

    // @@@TODO: do the actual playing of sound here
  }
  */
}

bool csAVIStreamAudio::LoadCodec (uint8 *pInitData, uint32 nInitDataLen,
				  uint8 *pFormatEx, uint32 nFormatEx)
{
  // based on the codec id we try to load the apropriate codec

  // create a classname from the coec id
  char cn[128];
  sprintf (cn, "crystalspace.audio.codec.avi.%s", strdesc.codec);
  // try open this class
  pCodec = SCF_CREATE_INSTANCE (cn, iAVICodec);
  if (pCodec)
  {
    // codec exists, now try to initialize it
    if (pCodec->Initialize (&strdesc, pInitData, nInitDataLen, pFormatEx,
        nFormatEx))
      return true;
    else
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		"crystalspace.video.avi",
		"CODEC class \"%s\" could not be initialized !", cn);
      pCodec = 0;
    }
  }
  else
  {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		"crystalspace.video.avi",
		"CODEC class \"%s\" could not be loaded !", cn);
  }

  return false;
}
