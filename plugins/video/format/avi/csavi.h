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

#ifndef _CSAVI_H_
#define _CSAVI_H_

#include "ividecod.h"
#include "ivfs.h"

struct iSystem;

class csAVIFormat : public iStreamFormat
{
  class streamiterator : public iStreamIterator
  {
  private:
    streamiterator (iBase *pBase);
    csAVIFormat *pAVI;
    int pos;
  public:
    DECALRE_IBASE;
    bool HasNext ();
    iStream *GetNext ();
  };

  struct RIFFheader
  {
    char id[4];
    ULong filesize;
    char type[4];
  };
  struct RIFFchunk
  {
    char id[4];
    ULong chunksize;
  };
  struct RIFFlist
  {
    char id[4];
    ULong listsize;
    char type[4];
  };
  struct AVIheader
  {
    ULong msecperframe; // milliseconds per frame
    ULong maxbytespersec; // max. transfer rate in bytes/sec
    ULong padsize;
    ULong flags;
    ULong framecount;
    ULong initialframecount;
    ULong framecount;
    ULong suggestedbuffersize;
    ULong width;
    ULong height;
    ULong reserved[4];
  };
  struct StreamHeader
  {
    char type[4];
    char handler[4];
    ULong flags;
    UShort priority;
    UShort language;
    ULong initialframecount;
    ULong scale;
    ULong rate;
    ULong start;
    ULong length;
    ULong suggestedbuffersize;
    ULong quality;
    ULong samplesize;
    SLong left, top, right, bottom;
  };
  struct VideoStreamFormat
  {
    ULong size;
    ULong width;
    ULong height;
    UShort planes;
    UShort bitcount;
    ULong compression;
    ULong sizeimage;
    ULong xpelspermeter;
    ULong ypelspermeter;
    ULong colorsused;
    ULong colorsimportant;
  }
  struct AudioStreamFormat
  {
    UShort formattag;
    UShort channels;
    ULong samplespersecond;
    ULong avgbytespersecond;
    UShort blockalign;
    UShort bitspersample;
  };
 protected:
  size_t datalen;

  iSystem *pSystem;
  iFile *pFile;

  char *pData;

 public:
  DECLARE_IBASE;
  csAVIPlayer (iBase *pParent);
  virtual ~csAVIPlayer ();

  virtual bool Initialize (iSystem *iSys);

  virtual void GetCaps (csStreamFormatCap &caps);
  virtual iStreamIterator& GetStreamIterator ();
  virtual void Select (iAudioStream *pAudio, iVideoStream *pVideo);
  virtual void NextFrame ();
  virtual bool Load (iFile *pVideoData);
};
#endif
