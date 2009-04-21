/*
  Crystal Space Radix Sort
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CSUTIL_RADIX_SORT_H__
#define __CSUTIL_RADIX_SORT_H__

/**\file
 * General radix-sorter
 */

#include "csextern.h"

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

/**
 * A radix-sorter for signed and unsigned integers as well as floats.
 * Creates an index-table instead of reordering elements.
 * Based on ideas by Pierre Terdiman
 */
class CS_CRYSTALSPACE_EXPORT csRadixSorter
{
public:
  csRadixSorter();
  ~csRadixSorter();

  /**
   * Sort array of unsigned integers.
   * \param array Array of integers to sort
   * \param size Number of elements in array
   */
  void Sort(uint32* array, size_t size);

  /**
   * Sort array of signed integers.
   * \param array Array of integers to sort
   * \param size Number of elements in array
   */
  void Sort(int32* array, size_t size);

  /**
   * Sort array of floats.
   * \param array Array of floats to sort
   * \param size Number of elements in array
   */
  void Sort(float* array, size_t size);

  /**
   * Get the last generated ranks array.
   */
  inline size_t* GetRanks() const
  {
    return ranks1;
  }

  /**
   * Reorder a list with the ranks in-place
   */
  template<class T>
  void ReorderInplace (T* source, size_t size)
  {
    if(size*sizeof(T) < 0x4000) //Max out to 16kb
    {
      //Stack-allocated temp-array
      CS_ALLOC_STACK_ARRAY(uint8, tmpStorage, size*sizeof(T));
      T* dest = (T*)tmpStorage;
      for(size_t i = 0; i < size; i++)
      {
        new (&dest[i]) T(source[ranks1[i]]); //to make sure it is initialized
      }
      for(size_t i = 0; i < size; i++)
      {
        source[i] = dest[i];
        dest[i].~T();
      }
    }
    else
    {
      //Heap-allocated
      uint8* tmpStorage = (uint8*)cs_malloc(size*sizeof(T));
      T* dest = (T*)tmpStorage;
      for(size_t i = 0; i < size; i++)
      {
        new (&dest[i]) T(source[ranks1[i]]); //to make sure it is initialized
      }
      for(size_t i = 0; i < size; i++)
      {
        source[i] = dest[i];
        dest[i].~T();
      }
      cs_free(tmpStorage);
    }
  }

  /**
   * Reorder a list with the ranks in-place. 
   * Source and destination arrays should not overlap in memory!
   */
  template<class T>
  bool Reorder(const T* source, T* dest, size_t size)
  {
    //make sure ranges don't overlap
#ifdef _DEBUG
    if((source + size - dest > 0 && dest - source < size) ||
       (dest + size - source > 0 && source - dest < size))
       return false;
#endif

    for(size_t i = 0; i < size; i++)
    {
      dest[i] = source[ranks1[i]];
    }
    return true;
  }

private:
  // Resize the rank arrays
  void Resize(size_t size);

  //Helper-function to create histograms. Returns true if values are already sorted
  template<class T>
  bool CreateHistogram(T* data, size_t size, uint32* histogram);

  // Check if pass should be performed
  template<class T>
  bool DoPass(size_t pass, T* data, size_t size, uint32* histogram);

  size_t currentSize;
  size_t* ranks1;
  size_t* ranks2;

  bool ranksValid;
};

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif
