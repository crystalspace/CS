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
#include "odivx.h"
#include "imater.h"
#include "itxtmgr.h"
#include "csgfxldr/rgbpixel.h"

IMPLEMENT_IBASE (csAVIPlayer)
  IMPLEMENTS_INTERFACE (iVideoPlayer)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csAVIPlayer)
EXPORT_CLASS_TABLE (aviplay)
  EXPORT_CLASS (csOpenDivX, "crystalspace.video.aviplayer", 
		"CrystalSpace AVI player interface")
EXPORT_CLASS_TABLE_END

csAVIPlayer::csAVIPlayer (iBase *pParent): memimage(1,1)
{
  CONSTRUCT_IBASE (pParent);
}

csAVIPlayer::~csAVIPlayer ()
{
  if (bOK && pFile)
  {
    if (pCodec) pCodec->DecRef ();
    pFile->DecRef ();
    delete [] pData;
  }

  if (pMaterial) pMaterial->DecRef ();
  if (pG3D) pG3D->DecRef ();
  if (pSystem) pSystem->DecRef ();
}

bool csAVIPlayer::Initialize (iSystem *iSys)
{
  rc.Set (0, 0, 0, 0);
  fxmode = CS_FX_COPY;
  bTimeSynced = false;
  bOK = false;
  datalen = 0;

  pG3D = QUERY_PLUGIN (iSys, iGraphics3D);
  pSystem = iSys;
  pSystem->IncRef ();
  pMaterial = NULL;
  pFile = NULL;
  pData = NULL;

  polyfx.num = 4;
  polyfx.use_fog = false;
  for (int i=0; i<4; i++) 
  {
    polyfx.vertices[i].r=1;
    polyfx.vertices[i].g=1;
    polyfx.vertices[i].b=1;
    polyfx.vertices[i].z=1;
  }

  return pG3D != NULL;
}

void csAVIPlayer::GetDecoderCaps (csVideoDecoderCap &caps)
{
  caps = (csVideoDecoderCap)0; // currently we can only play one after another, no positioning or whatsoever
}

bool csAVIPlayer::Load (iFile *pVideoData)
{
  bOK = false;
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
  {
    InitVideoData ()
  }
  return false;
}

bool csAVIPlayer::InitVideoData ()
{
  bool bSucc;

  // first remove all previous streams
  DropStreams ();
  p = pData;
  memcpy (fileheader, p, sizeof (RIFFheader));
  fileheader.filesize = little_endian_long (fileheader.filesize);
  bSucc = (!strcmp (fileheader.id, "RIFF") && !strcmp (fileheader.type, "AVI ") 
	   && fileheader.filesize <= datalen);
  if (bSucc)
  {
    bSucc = false;
    p += sizeof (RIFFheader);
    memcpy (headerlist, p, sizeof (RIFFlist));
    hdrl.listsize = little_endian_long (hdrl.listsize);
    if (!strcmp (hdrl.id, "hdrl"))
    {
      pmovi = p + hdrl.listsize; // audio/video data begins here
      p += sizeof (RIFFlist);
      // read the AVI header chunk
      memcpy (avichunk, p, sizeof (RIFFchunk));
      if (!strcmp (avichunk.id, "avih"))
      {
	// read the avi header data
	memcpy (aviheader, p + sizeof (RIFFchunk), sizeof (AVIheader));
	p += avichunk.chunksize;
	// now read all streamlists
	while (p < pmovi)
	{
	  memcpy (strl, p, sizeof (RIFFlist));
	  strl.listsize = little_endian_long (strl.listsize);
	  if (!strcmp (strl.id, "strl"))
	  {
	    p += sizeof (RIFFlist);
	    n = sizeof (RIFFlist);
	    // read all chunks in the list
	    while (n < strl.listsize)
	    {
	      memcpy (strh, p, sizeof (RIFFchunk));
	      strh.chunksize = little_endian_long (strh.chunksize);
	      if (!strcmp (strh.id, "strh"))
	      {
		p += sizeof (RIFFchunk);
		memcpy (streamheader, p, sizeof (StreamHeader));
		p += sizeof (strh.chunksize) - sizeof (RIFFchunk);
		pStream = CreateStream (streamheader);
	      }
	      else
	      if (!strcmp (strh.id, "strf"))
	      {
		p += sizeof (RIFFchunk);
		if (pStream)
		  memcpy (pStream->GetFormat (), p, pStream->GetFormatLength ());
		p += sizeof (strh.chunksize) - sizeof (RIFFchunk);
	      }
	      else
		p += sizeof (strh.chunksize);
	      n += sizeof (strh.chunksize);
	    }
	  }
	  else
	    p += strl.listsize;
	}
	p = pmovi;
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
  ValidateStreams ();
  if (vVideoStream.Length () == 0)
    pSystem->Printf (MSG_WARNING, "No valid videostream found !\n");
  return vVideoStream.Length () > 0;
}

bool csAVIPlayer::GotoFrame (ULong frameindex)
{
  (void)frameindex;
  return false;
}

bool csAVIPlayer::GotoTime (ULong timeindex)
{
  (void)timeindex;
  return false;
}

bool csAVIPlayer::SetPlayMethod (bool bTimeSynced)
{
  (void)bTimeSynced;
  return bTimeSynced == true;
}


