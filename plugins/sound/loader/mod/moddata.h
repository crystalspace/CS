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

#ifndef __CS_MOD_SOUNDDATA_H__
#define __CS_MOD_SOUNDDATA_H__

/**
 * iSoundData implementation for mod bitdata using mikmod.
 */

#include "isound/data.h"
#ifndef _DLL
#define _DLL
#endif
#include <mikmod.h>

class csModSoundData : public iSoundData
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
    ~datastore(){ if (bOwn) delete[] data;}
  };


 public:
  struct cs_mod_reader
  {
    cs_mod_reader (uint8 *d, size_t l, bool own);
    int (*Seek) (MREADER*, long offset, int whence);
    long (*Tell) (MREADER*);
    int (*Read) (MREADER*, void *dest, size_t length);
    int  (*Get)  (MREADER*);
    int (*Eof)  (MREADER*);
    datastore ds;
    bool bEof;
  };

 protected:
  cs_mod_reader *mod_reader;
  MODULE *module;

  static int bits, channels;

  csSoundFormat fmt;
  bool mod_ok;
  uint8 *buf, *pos;
  size_t len, bytes_left;

 public:
  static bool mikmod_init, mikmod_reinit;

  SCF_DECLARE_IBASE;

  csModSoundData (iBase *parent, uint8 *data, size_t len);
  virtual ~csModSoundData ();

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

  static bool IsMod (void *Buffer, size_t len);
};

#endif // __CS_MOD_SOUNDDATA_H__
