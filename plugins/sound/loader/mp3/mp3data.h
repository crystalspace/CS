/*
    Copyright (C) 2001 by Norman Krämer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.
  
    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _CS_OGG_SOUNDDATA_H_
#define _CS_OGG_SOUNDDATA_H_

/**
 * iSoundData implementation for ogg bitdata. Hmm, sounds ogg.
 */

#include "isound/data.h"
#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"

class csMp3SoundData : public iSoundData
{
 public:
  struct datastore
  {
    unsigned char *data;
    size_t pos;
    size_t length;
    bool bOwn;
    datastore (uint8 *d, size_t l, bool own)
    {
      if (own)
      {
	data=new unsigned char[l]; 
	memcpy (data, d, l);
      }
      else
	data = d;
      pos=0; length = l;
      bOwn = own;
    }
    ~datastore(){ if (bOwn) free(data);}
  };


 protected:
  datastore *ds;
  csSoundFormat fmt;
  bool mp3_ok;
  mpstr mp;
  int bytes_left;
  uint8 *buf, *pos;
  long len;

 public:
  SCF_DECLARE_IBASE;

  csMp3SoundData (iBase *parent, uint8 *data, size_t len);
  virtual ~csMp3SoundData ();

  /// Prepare the sound for output using the given format.
  virtual bool Initialize(const csSoundFormat *fmt);
  /// Get the format of the sound data.
  virtual const csSoundFormat *GetFormat();
  /// Return true if this is a static sound, false if it is streamed.
  virtual bool IsStatic();

  /// Get size of this sound in samples (static sounds only).
  virtual long GetStaticSampleCount();
  /// Get a pointer to the data buffer (static sounds only).
  virtual void *GetStaticData();

  /// Reset the sound to the beginning (streamed sounds only).
  virtual void ResetStreamed();
  /**
   * Read a data buffer from the sound (streamed sounds only). The NumSamples
   * parameter is modified to a smaller value if not all samples could be
   * read (i.e. the stream is finished). The returned buffer is valid until
   * the next call to Read().
   */
  virtual void *ReadStreamed(long &NumSamples);

  static bool IsMp3 (void *Buffer, size_t len);
};

#endif
