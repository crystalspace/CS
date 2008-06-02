/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

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

#ifndef SNDSYS_DATA_WAV_H
#define SNDSYS_DATA_WAV_H

/**\file
 * iSndSysData implementation for waveform PCM audio
 */

#include "iutil/databuff.h"
#include "csplugincommon/sndsys/snddata.h"

CS_PLUGIN_NAMESPACE_BEGIN(SndSysWav)
{

using namespace CS::SndSys;

// header of the RIFF-chunk
struct _RIFFchk
{
  char riff_id[4]; // for RIFF-files always "RIFF"
  uint32 len; // length of chunk after this 8 bytes of header
  char wave_id[4]; // for wav-files always "WAVE"
};

// header of the wav-format-chunk
struct _FMTchk
{
  char chunk_id[4]; // for the format-chunk of wav-files always "m_SoundFormat "
  uint32 len; // length of this chunk after this 8 bytes of header
  uint16 fmt_tag;
  uint16 channel;
  uint32 samples_per_sec;
  uint32 avg_bytes_per_sec;
  uint16 blk_align;
  uint16 bits_per_sample;
};

// header of the wav-data-chunk
struct _WAVchk
{
  char chunk_id[4]; // for wav-data-chunk this is always "data"
  uint32 len; // length of chunk after this 8 bytes of header
};


/// A simple structure used to hold the wav audio data
struct WavDataStore
{
  /// A reference to the buffer holding the data
  csRef<iDataBuffer> buf;
  /// A pointer to the raw buffer for the data
  unsigned char *data;
  /// The length of the data contained in the buffer
  size_t length;

  /// Construct this store from an iDataBuffer interface
  WavDataStore (iDataBuffer* buf) : buf(buf)
  {
    data = buf->GetUint8();
    length = buf->GetSize();
  }
};

/// Structure used by the Stream to track position and reference the static sound data.
struct WavStreamData
{
  WavDataStore *datastore;
  int32 position;
};

/// The implementation of iSndSysData for PCM Waveform audio
class SndSysWavSoundData : public SndSysBasicData
{
public:
  /// Construct this object from an iDataBuffer containing wav format PCM audio
  SndSysWavSoundData (iBase *parent, iDataBuffer* data);
  virtual ~SndSysWavSoundData();


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysData
  //------------------------
public:

  /** 
   * Return the size of the data stored in bytes.  This is informational only 
   * and is not guaranteed to be a number usable for sound calculations.
   * For example, an audio file compressed with variable rate compression may 
   * result in a situation where FILE_SIZE is not equal to 
   * FRAME_COUNT * FRAME_SIZE since FRAME_SIZE may vary throughout the 
   * audio data.
   */
  virtual size_t GetDataSize();

  /**
   * Creates a stream associated with this sound data positioned at the
   * beginning of the sound data and initially paused if possible.
   */
  virtual iSndSysStream *CreateStream(csSndSysSoundFormat *renderformat,
  	int mode3d);



  ////
  // Internal functions
  ////
protected:


  /// Initializes member variables that must be calculated.
  //  This is called when information that must be calculated
  //  is requested for the first time 
  void Initialize();

  /// Read and verify the RIFF wav headers from the provided buffer
  static bool ReadHeaders(void *Buffer, size_t len, _RIFFchk *p_riffchk, 
    _FMTchk *p_fmtchk, _WAVchk *p_wavchk, void **data_start, 
    size_t *data_len);

public:
  /// Determine if the passed data is of wav audio format
  static bool IsWav (iDataBuffer* Buffer);


  ////
  //  Member variables
  ////
protected:
  /// Storage of the wav audio data
  WavDataStore m_DataStore;

  // Stored headers related to the sound format.

  /// Stored RIFF header extracted from the sound data
  _RIFFchk m_RIFFHeader;

  /// Stored FMT header extracted from the sound data
  _FMTchk m_FMTHeader;

  /// Stored WAV header extracted from the sound data
  _WAVchk m_WAVHeader;

  /// Pointer to the beginning of samples
  void *m_pPCMData;

  /// Length of the samples in bytes
  size_t m_PCMDataLength;
};

}
CS_PLUGIN_NAMESPACE_END(SndSysWav)

#endif // #ifndef SNDSYS_DATA_WAV_H


