/*
    Copyright (C) 2004 by Andrew Mann

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

#include "cssysdef.h"
#include "csplugincommon/sndsys/convert.h"


#define SWAP16(x) ((x)<<8 | (x)>>8)


/*
  A series of samples must be converted from one frequency to another.

*/

inline long InterpolateSample(int sample0,int sample1, int sample_position)
{
  return ((sample0 * (INTERNAL_FREQUENCY_DIVISOR-sample_position)) + (sample1 * sample_position))/INTERNAL_FREQUENCY_DIVISOR;
}


PCMSampleConverter::PCMSampleConverter(int source_channels, int source_bitspersample, int source_frequency, bool swap16)
{
  src_channels=source_channels;
  src_bytes=source_bitspersample/8;
  src_frequency=source_frequency;
  swap_16=swap16;


  // Set to the 1th sample.  The 0th sample is the last sample of the previous buffer
  //  which will initially be set to silence.  The 1th sample will be the first sample 
  //  passed in.
  position_offset=INTERNAL_FREQUENCY_DIVISOR;

    // Fill the sample buffer with silence
  if (src_bytes==1)
  {
    int channel;
    // Silence is 128 for 8 bit samples
    for (channel=0;channel<SOUND_ELEMENT_MAX_CHANNELS;channel++)
      last_sample[channel]=128;
  }
  else
    // Silence is 0 for 16 bit samples
    memset(last_sample,0,sizeof(int)*SOUND_ELEMENT_MAX_CHANNELS);

}

PCMSampleConverter::~PCMSampleConverter()
{
}

int PCMSampleConverter::GetRequiredOutputBufferMultiple(int dest_channels, int dest_bitspersample, int dest_frequency)
{
  // This is purely a relation of bits per second in one format to bits per second in another
  //  We multiply by 1024 so that smaller destination buffers can be represented
  int multiple_1k=((dest_channels * dest_bitspersample * dest_frequency * 128)/(src_channels*src_bytes*src_frequency));
  if (multiple_1k<1) multiple_1k=1;
  return multiple_1k;
}

bool PCMSampleConverter::ReadFullSample8(const void **source, int *source_len, int *sample_buffer)
{
  int channel,max_channels;
  const unsigned char *src=(const unsigned char *)(*source);

  if (src_channels > (*source_len))
    return false;

  // Fill the sample buffer with no sound, which is 128 for 8 bit samples
  for (channel=0;channel<SOUND_ELEMENT_MAX_CHANNELS;channel++)
    sample_buffer[channel]=128;

  // Determine the number of samples to read - max of either the build-time configured maximum channels
  //  or the number of channels in the passed in source data
  max_channels=src_channels;
  if (max_channels > SOUND_ELEMENT_MAX_CHANNELS)
    max_channels=SOUND_ELEMENT_MAX_CHANNELS;

  // Read the channels
  for (channel=0;channel<max_channels;channel++)
    sample_buffer[channel]=*(src++);

  // Merge mono into stereo if source is mono
  if (src_channels==1)
    sample_buffer[1]=sample_buffer[0];

  // Adjust the source pointer
  *source=(const void *)src;
  // Adjust the source length
  *source_len-=src_channels;

  return true;
}
  
bool PCMSampleConverter::ReadFullSample16(const void **source, int *source_len, int *sample_buffer)
{
  int channel,max_channels;
  const short *src=(const short *)(*source);

  if ((src_channels*2) > (*source_len))
    return false;

  // Fill the sample buffer with no sound, which is 0 for 16 bit samples
  memset(sample_buffer,0,sizeof(int)*SOUND_ELEMENT_MAX_CHANNELS);

  // Determine the number of samples to read - max of either the build-time configured maximum channels
  //  or the number of channels in the passed in source data
  max_channels=src_channels;
  if (max_channels > SOUND_ELEMENT_MAX_CHANNELS)
    max_channels=SOUND_ELEMENT_MAX_CHANNELS;

  // Read the channels
  if(swap_16)
  {
    for (channel=0;channel<max_channels;channel++)
      sample_buffer[channel]=SWAP16(*(src++));
  }
  else
  {
    for (channel=0;channel<max_channels;channel++)
      sample_buffer[channel]=*(src++);
  }

  // Merge mono into stereo if source is mono
  if (src_channels==1)
    sample_buffer[1]=sample_buffer[0];

  // Adjust the source pointer
  *source=(const void *)src;
  // Adjust the source length
  *source_len-=(src_channels*2);

  return true;
}


bool PCMSampleConverter::ReadFullSample(const void **source, int *source_len, int *sample_buffer)
{
  if (src_bytes==1)
    return ReadFullSample8(source,source_len,sample_buffer);

  return ReadFullSample16(source,source_len,sample_buffer);
}


int PCMSampleConverter::WriteSample8(int *sample_buffer, void **dest, int dest_channels)
{
  int channel;
  unsigned char *dst_buf=(unsigned char *)(*dest);

  if (src_bytes==1)
  {
    // 8 bit source, 8 bit dest

    // Stereo into mono if dest is mono and source is stereo
    if ((dest_channels==1) && (src_channels==2))
    {
      dst_buf[0] = (unsigned char)((sample_buffer[0] + sample_buffer[1])/2);
      dst_buf++;
      *dest=(void *)dst_buf;
      return 1; 
    }

    // Otherwise, copy all channels available
    for (channel=0;channel<dest_channels;channel++)
    {
      if (channel>=SOUND_ELEMENT_MAX_CHANNELS)
        dst_buf[channel]=128;
      else
        dst_buf[channel]=(unsigned char)(sample_buffer[channel]);
    }
  }
  else
  {
    // 16 bit source, 8 bit dest

    // Stereo into mono if dest is mono and source is stereo
    if ((dest_channels==1) && (src_channels==2))
    {
      dst_buf[0] = (unsigned char)(( ((sample_buffer[0] + sample_buffer[1])/2) + 32768) >> 8);
      dst_buf++;
      *dest=(void *)dst_buf;
      return 1; 
    }

    // Otherwise, copy all channels available
    for (channel=0;channel<dest_channels;channel++)
    {
      if (channel>=SOUND_ELEMENT_MAX_CHANNELS)
        dst_buf[channel]=128;
      else
        dst_buf[channel]=(unsigned char)((sample_buffer[channel] + 32768) >> 8);
    }
  }

  dst_buf+=dest_channels;
  *dest=(void *)dst_buf;

  return dest_channels;
}

int PCMSampleConverter::WriteSample16(int *sample_buffer, void **dest, int dest_channels)
{
  int channel;
  short *dst_buf=(short *)(*dest);

  if (src_bytes==1)
  {
    // 8 bit source, 16 bit dest

    // Stereo into mono if dest is mono and source is stereo
    if ((dest_channels==1) && (src_channels==2))
    {
      dst_buf[0] = (short) ((((sample_buffer[0] + sample_buffer[1])/2) - 128) << 8);
      dst_buf++;
      *dest=(void *)dst_buf;
      return 2; 
    }

    // Otherwise, copy all channels available
    for (channel=0;channel<dest_channels;channel++)
    {
      if (channel>=SOUND_ELEMENT_MAX_CHANNELS)
        dst_buf[channel]=0;
      else
        dst_buf[channel]=(short)((sample_buffer[channel]-128) << 8);
    }
  }
  else
  {
    // 16 bit source, 16 bit dest

    // Stereo into mono if dest is mono and source is stereo
    if ((dest_channels==1) && (src_channels==2))
    {
      dst_buf[0] = (short)((sample_buffer[0] + sample_buffer[1])/2);
      dst_buf++;
      *dest=(void *)dst_buf;
      return 2; 
    }

    // Otherwise, copy all channels available
    for (channel=0;channel<dest_channels;channel++)
    {
      if (channel>=SOUND_ELEMENT_MAX_CHANNELS)
        dst_buf[channel]=0;
      else
        dst_buf[channel]=(short)(sample_buffer[channel]);
    }
  }

  dst_buf+=dest_channels;
  *dest=(void *)dst_buf;


  return dest_channels*2;
}

int PCMSampleConverter::WriteSample(int *sample_buffer, void **dest, int dest_channels, int dest_bitspersample)
{
  if (dest_bitspersample==8)
    return WriteSample8(sample_buffer,dest,dest_channels);
  
  return WriteSample16(sample_buffer,dest,dest_channels);
}

int PCMSampleConverter::AdvanceSourceSamples(const void **source, int *source_len, int samples_to_advance, int *sample_buffer)
{
  samples_to_advance--;
  int advance_bytes=src_channels*src_bytes*samples_to_advance;

  // Recalculate the number of samples we can advance if there's not enough space
  if (*source_len < advance_bytes)
  {
    samples_to_advance=(*source_len/(src_channels*src_bytes))-1;
    advance_bytes=src_channels*src_bytes*samples_to_advance;
  }
  *source_len-=advance_bytes;
  *source= (const void *)(((unsigned char *)(*source))+advance_bytes);

  if (ReadFullSample(source,source_len,sample_buffer))
    samples_to_advance++;

  return samples_to_advance;
}


int PCMSampleConverter::ConvertBuffer(const void *source, int source_len, void *dest, int dest_channels, int dest_bitspersample, int dest_frequency)
{
  int bank[SOUND_ELEMENT_MAX_CHANNELS],merge[SOUND_ELEMENT_MAX_CHANNELS];
  int advance_amount=(src_frequency*INTERNAL_FREQUENCY_DIVISOR)/dest_frequency;
  int dest_bytes_written=0;

  
  if (position_offset<=INTERNAL_FREQUENCY_DIVISOR)
  {
    // Read next sample if available
    if (!ReadFullSample(&source,&source_len,bank))
      return dest_bytes_written;
  }
  else
  {
    // Advance through any whole samples we need to beforehand
    int advanced=AdvanceSourceSamples(&source,&source_len,(position_offset-1)/INTERNAL_FREQUENCY_DIVISOR,last_sample);
    position_offset-=INTERNAL_FREQUENCY_DIVISOR*advanced;
    if (position_offset>INTERNAL_FREQUENCY_DIVISOR)
      return dest_bytes_written;
    // Read next sample if available
    if (!ReadFullSample(&source,&source_len,bank))
      return dest_bytes_written;
  }


  do
  {
    // Advance through any whole samples we need to beforehand
    if (position_offset>=INTERNAL_FREQUENCY_DIVISOR)
    {
      position_offset-=INTERNAL_FREQUENCY_DIVISOR;

      // Need to advance at least 2 full samples
      if (position_offset>INTERNAL_FREQUENCY_DIVISOR)
      {
        int advanced=AdvanceSourceSamples(&source,&source_len,(position_offset-1)/INTERNAL_FREQUENCY_DIVISOR,last_sample);
        position_offset-=INTERNAL_FREQUENCY_DIVISOR*advanced;
        if (position_offset>INTERNAL_FREQUENCY_DIVISOR)
          return dest_bytes_written;
      }
      else
      {
        // Only advancing one sample, this is a shift from the local bank to the last_sample bank
        // and then a read
        memcpy(last_sample,bank,sizeof(int)*SOUND_ELEMENT_MAX_CHANNELS);
      }
      // Read next sample if available
      if (!ReadFullSample(&source,&source_len,bank))
        return dest_bytes_written;
    }


    // Interpolate
    if (position_offset==0)
      dest_bytes_written+=WriteSample(last_sample,&dest,dest_channels,dest_bitspersample);
    else
    {
      if (position_offset==INTERNAL_FREQUENCY_DIVISOR)
        dest_bytes_written+=WriteSample(bank,&dest,dest_channels,dest_bitspersample);
      else
      {
        int channel;
        for (channel=0;channel<SOUND_ELEMENT_MAX_CHANNELS;channel++)
          merge[channel]=InterpolateSample(last_sample[channel],bank[channel],position_offset);
        dest_bytes_written+=WriteSample(merge,&dest,dest_channels,dest_bitspersample);
      }
    }

    // Advance position
    position_offset+=advance_amount;
  } while (source_len>0);

  return dest_bytes_written;
}


void PCMSampleConverter::SetSourceProperties(int source_channels, int source_bitspersample, int source_frequency)
{
  src_channels=source_channels;
  src_bytes=source_bitspersample/8;
  src_frequency=source_frequency;
}







