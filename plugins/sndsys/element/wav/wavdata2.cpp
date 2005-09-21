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

#include "cssysdef.h"
#include "iutil/comp.h"
#include "isndsys/ss_loader.h"
#include "wavstream2.h"
#include "wavdata2.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (SndSysWavSoundData)
  SCF_IMPLEMENTS_INTERFACE (iSndSysData)
SCF_IMPLEMENT_IBASE_END


// Microsoft Wav file loader
// support 8 and 16 bits PCM (RIFF)

/* ====================================
   short description of wav-file-format
   (from www.wotsit.org, look there for details)
   ====================================

  WAV-data is contained within the RIFF-file-format. This file format
  consists of headers and "chunks".

  A RIFF file has an 8-byte RIFF header.

  struct
  {
    char  id[4];   // identifier string = "RIFF"
    DWORD len;     // remaining length after this header
  } riff_hdr;

  The riff_hdr is immediately followed by a 4-byte data type identifier.
  For .WAV files this is "WAVE" as follows:

  char wave_id[4]; // WAVE file identifier = "WAVE"

  The remaining file consists of "chunks". A chunk is following:

  struct
  {
    char  id[4];   // identifier, e.g. "fmt " or "data"
    DWORD len;     // remaining chunk length after this header
  } chunk_hdr;

  The chunk is followed by the data of the chunk.

  Unexpected chunks should be allowed (and ignored) within a RIFF-file.

  For WAV-files following chunks can be expected:
  (only necessary chunks are listed)

  WAVE Format Chunk, this has to appear before the WAVE Data Chunk:

  struct _FMTchk
  {
    char chunk_id[4];  // for the format-chunk of wav-files always "fmt "
    unsigned long len; // length of this chunk after this 8 bytes of header
    unsigned short fmt_tag; // format category of file. 0x0001 = Microsoft PCM
    unsigned short channel; // number of channels (1 = mono, 2 = stereo)
    unsigned long samples_per_sec; // sampling rate
    unsigned long avg_bytes_per_sec; // for buffer estimation
    unsigned short blk_align; // data block size
    unsigned short bits_per_sample; // sample size  (only if Microsoft PCM)
  } fmtchk;

  WAVE Data Chunk:

  struct _WAVchk
  {
    char chunk_id[4];  // for the data-chunk of wav-files always "data"
    unsigned long len; // length of this chunk after this 8 bytes of header
  }

   ====================================
   End of short format-description
   ====================================

*/




// helper functions
/// Byte swap 32 bit data.
static inline unsigned long csByteSwap32bit( const unsigned long value )
{
  return ((value >> 24 ) & 0x000000FF ) | ((value >> 8) & 0x0000FF00)
    | ((value << 8) & 0x00FF0000) | (( value << 24) & 0xFF000000);
}

/// Byte swap 16 bit data.
static inline unsigned short csByteSwap16bit( const unsigned short value )
{
  return (( value >> 8 ) & 0x000000FF ) | (( value << 8 ) & 0x0000FF00 );
}

static inline void csByteSwap16bitBuffer (uint16* ptr, size_t count)
{
  for (; count > 0; --count, ++ptr)
  {
    *ptr = csByteSwap16bit (*ptr);
  }
}

static inline void csByteSwap32bitBuffer (uint32* ptr, size_t count)
{
  for (; count > 0; --count, ++ptr)
  {
    *ptr = csByteSwap32bit (*ptr);
  }
}


SndSysWavSoundData::SndSysWavSoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);

  ds = new WavDataStore (data, len, true);
  fmt.Bits = 16;
  fmt.Channels = 2;
  data_ready = false;
}

SndSysWavSoundData::~SndSysWavSoundData ()
{
  delete ds;
  SCF_DESTRUCT_IBASE();
}

const csSndSysSoundFormat *SndSysWavSoundData::GetFormat()
{
  if (!data_ready)
    Initialize();
  return &fmt;
}

long SndSysWavSoundData::GetSampleCount()
{
  if (!data_ready)
    Initialize();
  return sample_count;
}

long SndSysWavSoundData::GetDataSize()
{
  return (long)(wavedata_len & 0x7FFFFFFF);
}

iSndSysStream *SndSysWavSoundData::CreateStream (
  csSndSysSoundFormat *renderformat, int mode3d)
{
  if (!data_ready)
    Initialize();

  SndSysWavSoundStream *stream=new SndSysWavSoundStream(this, 
    (char *)wavedata, wavedata_len, renderformat, mode3d);

  return (stream);
}

void SndSysWavSoundData::Initialize()
{

  if (ReadHeaders(ds->data,ds->length,&riffhdr,&fmthdr,&wavhdr,&wavedata,&wavedata_len))
  {
    sample_count=wavedata_len/ (fmthdr.bits_per_sample/8);
    fmt.Freq=fmthdr.samples_per_sec;
    fmt.Bits=fmthdr.bits_per_sample;
    fmt.Channels=fmthdr.channel;
    data_ready=true;
  }
}

bool SndSysWavSoundData::ReadHeaders(void *Buffer, size_t len, _RIFFchk *p_riffchk, _FMTchk *p_fmtchk, _WAVchk *p_wavchk, void **data_start, size_t *data_len)
{
  if (!Buffer)
    return false;

  // check if this is a valid wav-file
  struct _RIFFchk riffchk;
  struct _FMTchk fmtchk;
  struct _WAVchk wavchk;
  char *buf=(char *)Buffer;
  int index = 0;

  // check if file has the size to be able to contain all necessary chunks
  if (len < (sizeof (riffchk) + sizeof (fmtchk) + sizeof (wavchk)))
    return false;

  // copy RIFF-header
  memcpy(&riffchk, &buf[0], sizeof (riffchk));

  // check RIFF-header
  if (memcmp (riffchk.riff_id, "RIFF", 4) != 0)
    return false;

  // check WAVE-id
  if (memcmp (riffchk.wave_id, "WAVE", 4) != 0)
    return false;

  // find format-chunk, copy it into struct and make corrections if necessary

  // set index after riff-header to the first chunk inside
  index += sizeof (riffchk);

  // find format-chunk
  bool found = false; // true, if format-chunk was found
  for ( ;
    (found == false) && ((index + sizeof (fmtchk)) < len) ;
    // +8, because chunk_id + len are not counted in len
    index += fmtchk.len + 8
    )
  {
    memcpy(&fmtchk, &buf[index], sizeof (fmtchk));

    if (memcmp(fmtchk.chunk_id, "fmt ", 4) == 0)
      found = true;

    // correct length of chunk on big endian system
#ifdef CS_BIG_ENDIAN
    fmtchk.len = csByteSwap32bit (fmtchk.len);
#endif
  }

  // no format-chunk found -> no valid file
  if (found == false)
    return false;

  // correct the chunk, if under big-endian system
#ifdef CS_BIG_ENDIAN // @@@ is this correct?
  fmtchk.fmt_tag = csByteSwap16bit (fmtchk.fmt_tag);
  fmtchk.channel = csByteSwap16bit (fmtchk.channel);
  fmtchk.samples_per_sec = csByteSwap32bit (fmtchk.samples_per_sec);
  fmtchk.avg_bytes_per_sec = csByteSwap32bit (fmtchk.avg_bytes_per_sec);
  fmtchk.blk_align = csByteSwap16bit (fmtchk.blk_align);
  fmtchk.bits_per_sample = csByteSwap16bit (fmtchk.bits_per_sample);
#endif // CS_BIG_ENDIAN

  // check format of file

  // only 1 or 2 channels are valid
  if (!((fmtchk.channel == 1) || (fmtchk.channel == 2)))
    return false;

  // only Microsoft PCM wave files are valid
  if (!(fmtchk.fmt_tag == 0x0001))
    return false;

  // find wav-data-chunk
  found = false; // true, if wav-data-chunk was found
  for ( ;
    (found == false) && ((index + sizeof (wavchk)) < len) ;
    // +8, because chunk_id and len are not counted in len
    index += wavchk.len + 8
    )
  {
    memcpy(&wavchk, &buf[index], sizeof (wavchk));

    if (memcmp(wavchk.chunk_id, "data", 4) == 0)
    {
      found = true;
      break; // Leave the loop without advancing index beyond this section
    }

    // correct length of chunk on big endian systems
#ifdef CS_BIG_ENDIAN
    wavchk.len = csByteSwap32bit (wavchk.len);
#endif
  }

  // no wav-data-chunk found -> no valid file
  if (found == false)
    return false;

  // index still points to the wavchk header
  if (data_start)
    *data_start=&buf[index+sizeof(wavchk)];
  if (data_len)
  {
    *data_len=wavchk.len;

    // Report only the size that's in the buffer
    if ((wavchk.len + index + sizeof(wavchk)) >  len)
      *data_len=len-(index + sizeof(wavchk));
  }
  if (p_riffchk)
    memcpy(p_riffchk, &riffchk, sizeof(_RIFFchk));
  if (p_fmtchk)
    memcpy(p_fmtchk, &fmtchk, sizeof(_FMTchk));
  if (p_wavchk)
    memcpy(p_wavchk, &wavchk, sizeof(_WAVchk));


  // This is a good wav file
  return true;
}


bool SndSysWavSoundData::IsWav (void *Buffer, size_t len)
{
  return ReadHeaders(Buffer,len,0,0,0,0,0);
}



