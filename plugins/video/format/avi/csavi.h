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

#include "ivideo/ividecod.h"
#include "isys/ivfs.h"
#include "csutil/csvector.h"
#include "cssys/csendian.h"

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
    enum { 
      LIST     = 0x00000001, // the chunk is a list
      KEYFRAME = 0x00000010, // frame is a keyframe
      NOTIME   = 0x00000100, // dont use this frame for time calculations - it takes no time 
                             // (palette change for example)
      COMPUSE  = 0x0fff0000  // masks the bits used for compression
    };

    struct indexentry
    {
      ULong id;
      ULong flags;
      ULong offset;
      ULong length;
      void Endian ()
      { 
	id = little_endian_long (id); 
	flags = little_endian_long (flags); 
	offset = little_endian_long (offset); 
	length = little_endian_long (length); 
      }
    };

    class StreamIdx : public csVector
    {
    public:
      ULong id;
    public:
      StreamIdx (ULong id) : csVector (8,8) {this->id = id;}
      virtual ~StreamIdx (){}
      virtual indexentry *Get (int idx)const {return (indexentry*)csVector::Get(idx); }
    };

    class StreamList : public csVector
    {
    public:
      StreamList () : csVector (8, 8){}
      virtual ~StreamList () {}
      virtual int Compare (csSome Item1, csSome Item2, int) const
      { 
	StreamIdx *i1 = (StreamIdx*)Item1, *i2 = (StreamIdx*)Item2;
        return (i1->id < i2->id ? -1 : i1->id > i2->id ? 1 : 0);
      }
      virtual int CompareKey (csSome Item1, csConstSome Item2, int) const
      { 
	StreamIdx *i1 = (StreamIdx*)Item1;
	ULong id = (ULong)Item2;
        return (i1->id < id ? -1 : i1->id > id ? 1 : 0);
      }
      virtual StreamIdx *Get (int idx)const {return (StreamIdx*)csVector::Get(idx); }
      virtual bool FreeItem (csSome Item) { delete (StreamIdx*)Item; return true; }
    };

    StreamList streamlist;
    char *start;
  public:
    ChunkList (char *start) {this->start = start;}
    void LoadList (UByte *data, ULong length);
    bool HasChunk (ULong id, ULong idx);
    bool GetPos (ULong id, ULong idx, char *&pos, ULong &size);
  };

  friend class streamiterator;
  friend class csAVIStreamVideo;
  friend class csAVIStreamAudio;

  struct AVIDataChunk
  {
    char id[5];
    SLong currentframe;
    char *currentframepos;
    void *data;
    ULong length; // in byte
  };

  // this is used to read in the id and length of an AVI fileheader, 
  // list or chunk
  struct hcl
  {
    char id[4];
    ULong size;
    void Endian (){ size = little_endian_long (size); }
    bool Is (const char *theID, const char* theType, const char* p)
      {return !strncmp (id, theID, 4) && !strncmp (p, theType, 4);}
    bool Is (const char* theID)
      {return !strncmp (id, theID, 4);}
  };

  const int len_hcl;
  const int len_id;
  const char *RIFF_ID;
  const char *LIST_ID;

  // recognized RIFF types
  const char *RIFF_AVI;
  // recognized LIST types
  const char *LIST_HDRL;
  const char *LIST_STRL;
  const char *LIST_MOVI;
  const char *LIST_REC;
  // recognized chunk types
  const char *CHUNK_AVIH;
  const char *CHUNK_STRH;
  const char *CHUNK_STRF;
  const char *CHUNK_STRD;
  const char *CHUNK_STRN;
  const char *CHUNK_IDX1;

  struct AVIHeader
  {
    enum { 
      HASINDEX       = 0x00000010,
      MUSTUSEINDEX   = 0x00000020,
      ISINTERLEAVED  = 0x00000100,
      TRUSTCKTYPE    = 0x00000800,
      WASCAPTUREFILE = 0x00010000,
      COPYRIGHTED    = 0x00020000
    };
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
    void Endian ()
    { 
      msecperframe = little_endian_long (msecperframe); 
      maxbytespersec = little_endian_long (maxbytespersec);
      padsize = little_endian_long (padsize);
      flags = little_endian_long (flags);
      framecount = little_endian_long (framecount);
      initialframecount = little_endian_long (initialframecount);
      streamcount = little_endian_long (streamcount);
      suggestedbuffersize = little_endian_long (suggestedbuffersize);
      width = little_endian_long (width);
      height = little_endian_long (height);
    }
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
    void Endian ()
    {
      flags = little_endian_long (flags);
      priority = little_endian_short (priority);
      language = little_endian_short (language);
      initialframecount = little_endian_long (initialframecount);
      scale = little_endian_long (scale);
      rate = little_endian_long (rate);
      start = little_endian_long (start);
      length = little_endian_long (length);
      suggestedbuffersize = little_endian_long (suggestedbuffersize);
      quality = little_endian_long (quality);
      samplesize = little_endian_long (samplesize);
      top = little_endian_long (top);
      left = little_endian_long (left);
      right = little_endian_long (right);
      bottom = little_endian_long (bottom);
    }
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
    void Endian ()
    {
      size = little_endian_long (size);
      width = little_endian_long (width);
      height = little_endian_long (height);
      planes = little_endian_short (planes);
      bitcount = little_endian_short (bitcount);
      compression = little_endian_long (compression);
      sizeimage = little_endian_long (sizeimage);
      xpelspermeter = little_endian_long (xpelspermeter);
      ypelspermeter = little_endian_long (ypelspermeter);
      colorsused = little_endian_long (colorsused);
      colorsimportant = little_endian_long (colorsimportant);
    }
  };
  struct AudioStreamFormat
  {
    UShort formattag;
    UShort channels;
    ULong samplespersecond;
    ULong avgbytespersecond;
    UShort blockalign;
    UShort bitspersample;
    UShort extra;
    void Endian ()
    {
      formattag = little_endian_short (formattag);
      channels = little_endian_short (channels);
      samplespersecond = little_endian_long (samplespersecond);
      avgbytespersecond = little_endian_long (avgbytespersecond);
      blockalign = little_endian_short (blockalign);
      bitspersample = little_endian_short (bitspersample);
      extra = little_endian_short (extra);
    }
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


  hcl fileheader;
  hcl hdrl, strl;
  hcl avih, strh, avichunk;
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
  bool HasChunk (ULong id, ULong frameindex);
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
