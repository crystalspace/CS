/*
    Copyright (C) 2001 by Norman Krämer
  
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

#include <divx/decore.h>
#include "cssysdef.h"
#include "cssys/csendian.h"
#include "csavi.h"
#include "avistrv.h"
#include "avistra.h"

IMPLEMENT_IBASE (csAVIFormat)
  IMPLEMENTS_INTERFACE (iStreamFormat)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csAVIFormat)
EXPORT_CLASS_TABLE (avifmt)
  EXPORT_CLASS (csAVIFormat, "crystalspace.video.format.avi", 
		"CrystalSpace AVI format interface")
EXPORT_CLASS_TABLE_END

csAVIFormat::csAVIFormat (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csAVIFormat::~csAVIFormat ()
{
  if (pFile)
  {
    Unload ();
    pFile->DecRef ();
    delete [] pData;
  }

  if (pSystem) pSystem->DecRef ();
}

bool csAVIFormat::Initialize (iSystem *iSys)
{
  datalen = 0;

  pSystem = iSys;
  pSystem->IncRef ();
  pFile = NULL;
  pData = NULL;
  pChunkList = NULL;

  maxframe = 0; // max. frame visited to date
  maxframepos = NULL;
  pmovi = NULL; // right before movi LIST
  moviendpos = NULL; // right behind last LIST within movi LIST
  startframepos = NULL; // right before 1st LIST within movi LIST
  no_recl = false;

  return true;
}

void csAVIFormat::GetCaps (csStreamFormatCap &caps)
{
  caps = CS_POS_BY_FRAME; // @@@TODO: BY_TIME should be relatively easy to implement
}

bool csAVIFormat::Load (iFile *pVideoData)
{
  if (pFile)
  {
    pFile->DecRef ();
    if (pData) 
      delete [] pData;
    pData = NULL;
    pFile = NULL;
  }
  
  (pFile = pVideoData)->IncRef ();
  size_t r=0;
  datalen = pFile->GetSize ();
  pData = new char[datalen];
  if (pData)
    r = pFile->Read (pData, datalen);

  if (datalen==r)
    return InitVideoData ();

  return false;
}

bool csAVIFormat::InitVideoData ()
{
  bool bSucc;

  // first remove all previous streams
  Unload ();
  p = pData;
  memcpy (&fileheader, p, sizeof (RIFFheader));
  fileheader.filesize = little_endian_long (fileheader.filesize);
  bSucc = (!strcmp (fileheader.id, "RIFF") && !strcmp (fileheader.type, "AVI ") 
	   && fileheader.filesize <= datalen);
  if (bSucc)
  {
    bSucc = false;
    p += sizeof (RIFFheader);
    memcpy (&hdrl, p, sizeof (RIFFlist));
    hdrl.listsize = little_endian_long (hdrl.listsize);
    if (!strcmp (hdrl.id, "hdrl"))
    {
      pmovi = p + hdrl.listsize; // audio/video data begins here
      p += sizeof (RIFFlist);
      // read the AVI header chunk
      memcpy (&avichunk, p, sizeof (RIFFchunk));
      if (!strncmp (avichunk.id, "avih", 4))
      {
	// read the avi header data
	memcpy (&aviheader, p + sizeof (RIFFchunk), sizeof (AVIHeader));
	p += avichunk.chunksize;
	// now read all streamlists
	while (p < pmovi)
	{
	  memcpy (&strl, p, sizeof (RIFFlist));
	  strl.listsize = little_endian_long (strl.listsize);
	  if (!strncmp (strl.id, "strl", 4))
	  {
	    p += sizeof (RIFFlist);
	    ULong n = sizeof (RIFFlist);
	    // read all chunks in the list
	    while (n < strl.listsize)
	    {
	      memcpy (&strh, p, sizeof (RIFFchunk));
	      strh.chunksize = little_endian_long (strh.chunksize);
	      if (!strncmp (strh.id, "strh", 4))
	      {
		p += sizeof (RIFFchunk);
		memcpy (&streamheader, p, sizeof (StreamHeader));
		p += strh.chunksize - sizeof (RIFFchunk);
		n += strh.chunksize;
		ULong nread = CreateStream (&streamheader);
		if (nread == 0)
		{
		  // error ! we skip the remainder of the stream LIST
		  p += (strl.listsize - n);
		  n = strl.listsize;
		}
		else
		  n += nread; 
	      }
	      else
	      {
		p += strh.chunksize;
		n += strh.chunksize;
	      }
	    }
	  }
	  else
	  {
	    pSystem->Printf (MSG_DEBUG_0, "unrecognized LIST type \"%4c\" .. skipping it !", strl.id);
	    p += strl.listsize;
	  }
	}
	p = pmovi;
	// here is the <movi> LIST expected
	RIFFlist movi;
	memcpy (&movi, p, sizeof (RIFFlist));
	if (!strncmp (movi.id, "movi", 4))
	{
	  movi.listsize = little_endian_long (movi.listsize);
	  startframepos = p + sizeof (RIFFlist);
	  moviendpos = p + movi.listsize;
	  maxframe = 0; // max. frame visited to date
	  maxframepos = startframepos;
	  // some AVI with only one stream do not enclose the chunks in a <rec> LIST but
	  // sequentially write <##??> chunks
	  memcpy (&movi, p, sizeof (RIFFlist));
	  if (strncmp (movi.id, "rec ", 4))
	    // we have found such a beast (or the AVI is corrupt :)
	    no_recl = true;
	  else
	    no_recl = false;
	}
      }
      else
	pSystem->Printf (MSG_WARNING, "No <avih> chunk found !\n");
    }
    else
      pSystem->Printf (MSG_WARNING, "No <hdrl> LIST found !\n");
  }
  else
    pSystem->Printf (MSG_WARNING, "No RIFF header found !\n");

  // did we find a video stream ?
  // first check the validity of the streams found
  if (!ValidateStreams ())
    pSystem->Printf (MSG_WARNING, "No valid videostream found !\n");
  return vStream.Length () > 0;
}

void csAVIFormat::Unload ()
{
  // i don't DecRef the objects because the are not valid after this point. So deleting them
  // most likely will force a segmentation fault if the streams are still used behind this point.
  for (int i=0; i < vStream.Length (); i++)
    delete (iStream*)vStream.Get (i);
  vStream.DeleteAll ();

  pAudio = NULL;
  pVideo = NULL;
  nAudio = 0;
  nVideo = 0;
}

bool csAVIFormat::ValidateStreams ()
{
  // a valid avi file contains one video stream and zero or more audiostreams
  csStreamDescription desc;
  iStreamIterator *it = GetStreamIterator ();
  int nAudio = 0, nVideo = 0;
  while (it->HasNext())
  {
    it->GetNext ()->GetStreamDescription (desc);
    if (desc.type == CS_STREAMTYPE_AUDIO) 
      nAudio++;
    else
    if (desc.type == CS_STREAMTYPE_VIDEO) 
      nVideo++;
  }
  it->DecRef ();
  return nAudio >= 0 && nVideo == 1;
}

ULong csAVIFormat::CreateStream (StreamHeader *streamheader)
{
  ULong n=0;

  if (!strncmp (streamheader->type, "auds", 4))
  {
    // make an audio stream
    csAVIStreamAudio *pAudioStream = new csAVIStreamAudio (this);
    nAudio++;
    
    memcpy (&strh, p, sizeof (RIFFchunk));
    strh.chunksize = little_endian_long (strh.chunksize);
    if (!strncmp (strh.id, "strf",4))
    {
      p += sizeof (RIFFchunk);
      memcpy (&audsf, p, sizeof (AudioStreamFormat));
      p += strh.chunksize - sizeof (RIFFchunk);
      n = strh.chunksize;
      if (pAudioStream->Initialize (&aviheader, streamheader, &audsf, nAudio, pSystem))
	vStream.Push (pAudioStream);
      else
	pAudioStream->DecRef ();
    }
  }
  else
  if (!strncmp (streamheader->type, "vids", 4))
  {
    // make an video stream
    csAVIStreamVideo *pVideoStream = new csAVIStreamVideo (this);
    nVideo++;
    
    memcpy (&strh, p, sizeof (RIFFchunk));
    strh.chunksize = little_endian_long (strh.chunksize);
    if (!strncmp (strh.id, "strf",4))
    {
      p += sizeof (RIFFchunk);
      memcpy (&vidsf, p, sizeof (VideoStreamFormat));
      p += strh.chunksize - sizeof (RIFFchunk);
      n = strh.chunksize;
      if (pVideoStream->Initialize (&aviheader, streamheader, &vidsf, nVideo, pSystem))
	vStream.Push (pVideoStream);
      else
	pVideoStream->DecRef ();
    }
  }
  else
  {
    // assume this is just a format we dont know, so simply skip it
    memcpy (&strh, p, sizeof (RIFFchunk));
    strh.chunksize = little_endian_long (strh.chunksize);
    if (!strncmp (strh.id, "strf",4))
    {
      pSystem->Printf (MSG_WARNING, "Unsupported streamtype \"%4c\" found ... ignoring it !", 
		       strh.id);
      n = strh.chunksize;
      p += strh.chunksize;
    }
    else
    {
      // hmm, not only we dont know this stream type, it does not have an format header either.
      // this rather indicates an error. We should skip the remaining of the stream LIST.
      // we force this by returning 0.
    }
  }
  return n;
}

bool csAVIFormat::HasChunk (ULong frameindex)
{
  bool bSucc = false;
  if (pChunkList)
    bSucc = pChunkList->HasChunk (frameindex);
  else
  {
    // bad, no chunklist. skip through the data and count it down
    if (frameindex > 0 && frameindex <= maxframe)
      bSucc = true;
    else
      if (frameindex > 0)
      {
	char *pp = maxframepos;
	if (!no_recl)
	{
	  RIFFlist ch;
	  while (pp < moviendpos && maxframe < frameindex)
	  {
	    memcpy (&ch, pp, sizeof (RIFFlist));
	    ch.listsize = little_endian_long (ch.listsize);
	    maxframepos = pp;
	    pp += ch.listsize;
	    maxframe++;
	  }
	}
	else
	{
	  RIFFchunk ch;
	  while (pp < moviendpos && maxframe < frameindex)
	  {
	    memcpy (&ch, pp, sizeof (RIFFchunk));
	    ch.chunksize = little_endian_long (ch.chunksize);
	    maxframepos = pp;
	    pp += ch.chunksize;
	    maxframe++;
	  }
	}
	  bSucc = (maxframe == frameindex);
      }
  }
  return bSucc;
}
 
bool csAVIFormat::GetChunk (ULong frameindex, AVIDataChunk *pChunk)
{
  char *pp = NULL;
  ULong maxsize=0, n=0;

  if (HasChunk (frameindex))
  {
    if (!pChunk->currentframepos)
    {
      pChunk->currentframe = 0;
      pChunk->currentframepos = startframepos;
    }
    if (pChunkList)
    {
      pp = pChunkList->GetPos (frameindex);
      if (!no_recl)
      {
	RIFFlist ch;
	memcpy (&ch, pp, sizeof (RIFFlist));
	ch.listsize = little_endian_long (ch.listsize);
	pp += sizeof (RIFFlist);
	maxsize = ch.listsize - sizeof (RIFFlist);
      }
      else
      {
	RIFFchunk ch;
	memcpy (&ch, pp, sizeof (RIFFchunk));
	ch.chunksize = little_endian_long (ch.chunksize);
	maxsize = ch.chunksize;
      }
    }
    else
    {  
      ULong startfrom = (frameindex < pChunk->currentframe ? 0 
			 : frameindex < maxframe ? pChunk->currentframe : maxframe);
      pp = (frameindex < pChunk->currentframe ? startframepos
	    : frameindex < maxframe ? pChunk->currentframepos : maxframepos);
    
      if (!no_recl)
      {
	RIFFlist ch;
	while (startfrom < frameindex)
	{
	  memcpy (&ch, pp, sizeof (RIFFlist));
	  ch.listsize = little_endian_long (ch.listsize);
	  maxframepos = pp;
	  pp += ch.listsize;
	  startfrom++;
	}

	// now look for the stream number
	pp -= ch.listsize;
	pp += sizeof (RIFFlist);
	maxsize = ch.listsize - sizeof (RIFFlist);
      }
      else 
      { // no_recl == true
	RIFFchunk ch;
	while (startfrom < frameindex)
	{
	  memcpy (&ch, pp, sizeof (RIFFchunk));
	  ch.chunksize = little_endian_long (ch.chunksize);
	  maxframepos = pp;
	  pp += ch.chunksize;
	  startfrom++;
	}
	pp -= ch.chunksize;
	maxsize = ch.chunksize;
      }
    }

    RIFFchunk chunk;

    while (n<maxsize)
    {
      memcpy (&chunk, pp, sizeof (RIFFchunk));
      chunk.chunksize = little_endian_long (chunk.chunksize);
      if (!strncmp (pChunk->id, chunk.id, 3))
      {
	pChunk->data = pp + sizeof (RIFFchunk);
	pChunk->length = chunk.chunksize - sizeof (RIFFchunk);
	return true;
      }
      else
      {
	pp += chunk.chunksize;
	n  += chunk.chunksize;
      }
    }
  }
  return false;
}

UShort csAVIFormat::stream_number (const char c1, const char c2)
{
#define AVI_DECODE_HEX(c) ((c)<='9'?(c)-'0':((c)<='F'?(c)-'A'+10:(c)-'a'+10))

  UShort num = AVI_DECODE_HEX(c1);
  num <<=4;
  num += AVI_DECODE_HEX(c2);

#undef AVI_DECODE_HEX
  return num;
}

iStreamIterator* csAVIFormat::GetStreamIterator ()
{
  return new streamiterator (this);
}

void csAVIFormat::Select (iAudioStream *pAudio, iVideoStream *pVideo)
{
  this->pAudio = pAudio;
  this->pVideo = pVideo;
}

void csAVIFormat::NextFrame ()
{
  pAudio->NextFrame ();
  pVideo->NextFrame ();
}

IMPLEMENT_IBASE (csAVIFormat::streamiterator)
  IMPLEMENTS_INTERFACE (iStreamIterator)
  IMPLEMENTS_INTERFACE (iBase)
IMPLEMENT_IBASE_END

csAVIFormat::streamiterator::streamiterator (iBase *pBase)
{
  CONSTRUCT_IBASE (pBase);
   pAVI = (csAVIFormat*)pBase;
   pos = 0;
}

csAVIFormat::streamiterator::~streamiterator ()
{
	
}

bool csAVIFormat::streamiterator::HasNext ()
{
  return pos < pAVI->vStream.Length ();
}

iStream* csAVIFormat::streamiterator::GetNext ()
{
  if (HasNext ())
    return (iStream*)pAVI->vStream.Get (++pos);
   return NULL;
}

