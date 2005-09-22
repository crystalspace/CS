/*
    Copyright (C) 2004 by Andrew Mann

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
#include "csplugincommon/sndsys/cyclicbuf.h"

namespace CrystalSpace
{

SoundCyclicBuffer::SoundCyclicBuffer(size_t buffer_size)
{
  buffer_base = new uint8[buffer_size];
  write_ptr=buffer_base;
  start_value=0;
  end_value=0;
  length=buffer_size;
}

SoundCyclicBuffer::~SoundCyclicBuffer()
{
  delete[] buffer_base;
}

size_t SoundCyclicBuffer::GetFreeBytes()
{
  return (length - (end_value-start_value));
}

void SoundCyclicBuffer::AddBytes(void *bytes_ptr,size_t bytes_length)
{
  size_t pass_bytes;
  uint8 *end_ptr, *read_ptr;
  CS_ASSERT(bytes_length <= (length - (end_value-start_value)));

  read_ptr=(uint8 *)bytes_ptr;

  // Potential 2-pass copying
  end_ptr=(buffer_base+length);
  pass_bytes=end_ptr-write_ptr;
  if (bytes_length < (size_t)pass_bytes)
    pass_bytes=bytes_length;

  memcpy(write_ptr,read_ptr,pass_bytes);
  write_ptr+=pass_bytes;
  end_value+=pass_bytes;
  if (write_ptr >= end_ptr)
    write_ptr=buffer_base;

  if (bytes_length > (size_t)pass_bytes)
  {
    // Second pass
    read_ptr+=pass_bytes;
    bytes_length-=pass_bytes;

    memcpy(write_ptr,read_ptr,bytes_length);
    write_ptr+=bytes_length;
    end_value+=bytes_length;
  }
}

size_t SoundCyclicBuffer::GetStartValue()
{
  return start_value;
}

size_t SoundCyclicBuffer::GetEndValue()
{
  return end_value;
}

void SoundCyclicBuffer::Clear(long value)
{
  start_value=value;
  end_value=value;
  write_ptr=buffer_base;
}

void SoundCyclicBuffer::AdvanceStartValue (size_t advance_amount)
{
  start_value+=advance_amount;

  /* end_value must always be greater than or equal to start value.
   * If start value is advanced past the end, the buffer still cannot get any 
   * more empty than empty. */
  if (end_value < start_value)
    end_value=start_value;
}

void SoundCyclicBuffer::GetDataPointersFromPosition(size_t *position_value, 
						    size_t max_length, 
						    uint8 **buffer1, 
						    size_t *buffer1_length, 
						    uint8 **buffer2, 
						    size_t *buffer2_length)
{
  uint8 *read_ptr, *end_ptr;
  size_t filled_length, copy_length, available_length;

  if (*position_value < start_value)
    *position_value=start_value; /* Cannot read data we don't have.  
                                  * This likely means a source isn't keeping 
				  * up */

  filled_length=end_value-start_value;
  end_ptr=(buffer_base+length);
  available_length=end_value - *position_value;

  // More data requested than available, reduce to available amount
  if (max_length > available_length)
    max_length=available_length;

  // No data available
  if (max_length <= 0)
  {
    *buffer1_length=0;
    *buffer2_length=0;
    return;
  }

  // First buffer
  // Calculate read position
  
  //read_ptr=(write_ptr-filled_length)+((*position_value)-start_value);
  read_ptr = (write_ptr-end_value) + *position_value;
  if (read_ptr < buffer_base)
    read_ptr+=length;

  // Calculate first buffer size
  copy_length=end_ptr-read_ptr;
  if (copy_length > max_length)
    copy_length=max_length;

  // Set values;
  *buffer1_length=copy_length;
  *buffer1=read_ptr;

  (*position_value)+=max_length;

  // Second buffer
  if (max_length <= copy_length)
  {
    *buffer2_length=0;
    return;
  }

  *buffer2=buffer_base;
  *buffer2_length=max_length-copy_length;
}

} // namespace CrystalSpace
