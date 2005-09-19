/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

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

#ifndef SNDSYS_DATA_OGG_H
#define SNDSYS_DATA_OGG_H

/**
 * iSndSysData implementation for ogg bitdata.
 */

#include "ss_structs.h"
#include "ss_data.h"



// This hack works around a build problem with some installations of Ogg/Vorbis
// on Cygwin where it attempts to #include the non-existent <_G_config.h>.  We
// must ensure that _WIN32 is not defined prior to #including the Vorbis
// headers.  This problem is specific to C++ builds; it does not occur with
// plain C builds (which explains why the CS configure check does not require
// this hack).
#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


struct OggDataStore
{
  unsigned char *data;
  size_t length;
  bool local_data;

  OggDataStore (uint8 *d, size_t l, bool copy_data)
  {
    if (copy_data)
    {
      data=new unsigned char[l];
      memcpy (data, d, l);
    }
    else
      data = d;

    length = l;
    local_data=copy_data;
  }
  ~OggDataStore()
  {
    if (local_data)
      delete[] data;
  }
};


struct OggStreamData
{
  OggDataStore *datastore;
  long position;
};

struct cs_ov_callbacks
{
  cs_ov_callbacks ();
  size_t (*read_func) (void *ptr, size_t sz, size_t nmemb, void *datasource);
  int    (*seek_func) (void *datasource, ogg_int64_t offset, int whence);
  int    (*close_func)(void *datasource);
  long   (*tell_func) (void *datasource);
};


class SndSysOggSoundData : public iSndSysData
{
 public:
  SCF_DECLARE_IBASE;

  SndSysOggSoundData (iBase *parent, uint8 *data, size_t len);
  virtual ~SndSysOggSoundData ();


  /// Get the format of the sound data.
  virtual const SndSysSoundFormat *GetFormat();

  /// Get size of this sound in samples.
  virtual long GetSampleCount();

  /** Return the size of the data stored in bytes.  This is informational only and is not guaranteed to be a number usable for sound calculations.
   *  For example, an audio file compressed with variable rate compression may result in a situation where FILE_SIZE is not equal to
   *   SAMPLE_COUNT * SAMPLE_SIZE since SAMPLE_SIZE may vary throughout the audio data.
   */
  virtual long GetDataSize();

  /// Creates a stream associated with this sound data positioned at the begining of the sound data and initially paused if possible.
  virtual iSndSysStream *CreateStream(SndSysSoundFormat *renderformat, int mode3d);

  void Initialize();

  static bool IsOgg (void *Buffer, size_t len);

 protected:
  OggDataStore *ds;
  int endian;
  bool data_ready;
  SndSysSoundFormat fmt;
  long sample_count;

};

#endif // #ifndef SNDSYS_DATA_OGG_H


