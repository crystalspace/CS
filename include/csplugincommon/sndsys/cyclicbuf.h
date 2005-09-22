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

#ifndef __CS_SNDSYS_CYCLICBUF_H__
#define __CS_SNDSYS_CYCLICBUF_H__

/**\file
 * An implementation of a cyclic buffer oriented for sound functionality.
 */
 
namespace CrystalSpace
{

/** 
 * An implementation of a cyclic buffer oriented for sound functionality.
 */
class SoundCyclicBuffer
{
public:
  /// Construct the cycling buffer with a specific maximum storage size.
  SoundCyclicBuffer(size_t buffer_size);
  ~SoundCyclicBuffer();

  /// Return the number of free bytes in the cyclic buffer
  size_t GetFreeBytes();
  
  /**
   * Add bytes to the cyclic buffer.  The bytes must fit.
   * Use GetFreeBytes() to check for available space and AdvanceStartValue() 
   * to make more space available.
   */
  void AddBytes(void *bytes_ptr, size_t bytes_length);

  /**
   * The positional value associated with the first byte of data in the buffer.
   */
  size_t GetStartValue();
  /**
   * The positional value associated with one byte beyond the last byte in the 
   * buffer.
   */
  size_t GetEndValue();
  /**
   * Advance the first byte pointer of the cyclic buffer so that data below 
   * this value can be overwritten.
   */
  void AdvanceStartValue (size_t advance_amount);

  /** 
   * Clear the buffer and reset the start and end values to the provided value.
   */
  void Clear (long value=0);

  /**
   * Get data pointers to copy data out of the cyclic buffer.
   */
  void GetDataPointersFromPosition (size_t* position_value, size_t max_length, 
    uint8 **buffer1, size_t* buffer1_length, uint8 **buffer2, 
    size_t* buffer2_length);

  /**
   * Get the buffer length of the cyclic buffer in bytes.
   */
  size_t GetLength() { return length; };

private:
  /// Length of the buffer in bytes
  size_t length;
  //@{
  /** 
   * Values associated with the start and end of the buffer.  
   * end_value - start_value = size of valid data in buffer.
   */
  size_t start_value;
  size_t end_value;
  //@}

  //@{
  /**
   * Pointers to the base of the buffer and the write pointer into the buffer 
   * respectively.
   */
  uint8 *buffer_base;
  uint8 *write_ptr;
  //@}
};

} // namespace CrystalSpace

#endif // __CS_SNDSYS_CYCLICBUF_H__
