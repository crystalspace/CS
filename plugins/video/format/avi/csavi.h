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
#include "csutil/csvector.h"

struct iSystem;
class csAVIStreamVideo;
class csAVIStreamAudio;

class csAVIFormat : public iStreamFormat
{
 protected:
  class streamiterator : public iStreamIterator
  {
    friend class csAVIFormat;
  protected:
    streamiterator (iBase *pBase);
    csAVIFormat *pAVI;
    int pos;
  public:
    DECLARE_IBASE;
    virtual ~streamiterator ();
    bool HasNext ();
    iStream *GetNext ();
  };

  // @@@TODO: handle optional chunklists in AVI files
  class ChunkList
  {
  public:
    ChunkList () {}
    bool HasChunk (ULong idx){(void)idx; return false;}
    char *GetPos (ULong idx) {(void)idx; return NULL;}
  };

  friend class streamiterator;
  friend class csAVIStreamVideo;
  friend class csAVIStreamAudio;

  struct AVIDataChunk
  {
    char id[4];
    ULong currentframe;
    char *currentframepos;
    void *data;
    ULong length; // in byte
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
  struct AVIHeader
  {
    ULong msecperframe; // milliseconds per frame
    ULong maxbytespersec; // max. transfer rate in bytes/sec
    ULong padsize;
    ULong flags;
    ULong framecount;
    ULong initialframecount;
    ULong streamcount;
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
  };
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

  char *pData, *p;
  UShort nAudio, nVideo; // streamcounter
  ULong maxframe; // max. frame visited to date
  char *maxframepos;
  char *pmovi; // right before movi LIST
  char *moviendpos; // right behind last LIST within movi LIST
  char *startframepos; // right before 1st LIST within movi LIST
  bool no_recl;


  RIFFheader fileheader;
  RIFFlist hdrl, strl;
  RIFFchunk avih, strh, avichunk;
  ChunkList *pChunkList;

  AVIHeader aviheader;
  StreamHeader streamheader;
  AudioStreamFormat audsf;
  VideoStreamFormat vidsf;

  csVector vStream;
  iAudioStream *pAudio;
  iVideoStream *pVideo;

  bool InitVideoData ();
  bool ValidateStreams ();
  ULong CreateStream (StreamHeader *streamheader);
  bool HasChunk (ULong frameindex);
  bool GetChunk (ULong frameindex, AVIDataChunk *pChunk);
  UShort stream_number (const char c1, const char c2);

 public:
  DECLARE_IBASE;

  csAVIFormat (iBase *pParent);
  virtual ~csAVIFormat ();

  virtual bool Initialize (iSystem *iSys);

  virtual void GetCaps (csStreamFormatCap &caps);
  virtual iStreamIterator* GetStreamIterator ();
  virtual void Select (iAudioStream *pAudio, iVideoStream *pVideo);
  virtual void NextFrame ();
  virtual bool Load (iFile *pVideoData);
  virtual void Unload ();
};
#endif
