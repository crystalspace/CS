/*
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

#include "cssysdef.h"
#include "csutil/radixsort.h"

csRadixSorter::csRadixSorter ()
: currentSize(0), ranks1(0), ranks2(0), ranksValid(false)
{
}

csRadixSorter::~csRadixSorter ()
{
  delete[] ranks1;
  delete[] ranks2;
}

void csRadixSorter::Resize (size_t size)
{
  size_t allocatedSize = (currentSize+31)&~31;
  if(size > allocatedSize ||
     size < allocatedSize/2)
  {
    currentSize = size;
    allocatedSize = (currentSize+31)&~31;
    delete[] ranks1;
    delete[] ranks2;
    ranks1 = new size_t[allocatedSize];
    ranks2 = new size_t[allocatedSize];
  }
}

template<class T>
bool csRadixSorter::CreateHistogram (T* data, size_t size, uint32* histogram)
{
  memset(histogram, 0, sizeof(uint32)*256*4);

  uint8* d = (uint8*)data;
  uint8* dend = d + sizeof(T)*size;

  // The four histogram parts. Endian-specific for the negative sorting
#ifdef CS_BIG_ENDIAN
  //OSX etc
  uint32* H0 = histogram + 768;
  uint32* H1 = histogram + 512;
  uint32* H2 = histogram + 128;
  uint32* H3 = histogram + 0;
#else
  uint32* H0 = histogram + 0;
  uint32* H1 = histogram + 256;
  uint32* H2 = histogram + 512;
  uint32* H3 = histogram + 768;
#endif

  bool alreadySorted = true; //yea, sure
  if(!ranksValid)
  {
    T* runner = (T*)data;
    T prevVal = *runner;
    while(d != dend)
    {
      // Read input 
      T val = *runner++;
      if(val < prevVal)
      {
        alreadySorted = false;
        break; // early out
      }
      prevVal = val;

      //Update histogram
      H0[*d++]++;
      H1[*d++]++;
      H2[*d++]++;
      H3[*d++]++;
    }

    if(alreadySorted)
    {
      return true; //already sorted, nothing more to do
    }
  }
  else
  {
    // Ranks are valid, use them as a starting-point
    size_t* indices = ranks1;
    T prevVal = data[*indices];

    while(d != dend)
    {
      // Read input as it is already sorted
      T val = data[*indices++];
      if(val < prevVal)
      {
        alreadySorted = false;
        break; // early out
      }
      prevVal = val;

      //Update histogram
      H0[*d++]++;
      H1[*d++]++;
      H2[*d++]++;
      H3[*d++]++;
    }

    if(alreadySorted)
    {
      return true; //already sorted, nothing more to do
    }
  }

  //Make sure histograms are finished if we early-out above
  while(d != dend)
  {
    //Update histogram
    H0[*d++]++;
    H1[*d++]++;
    H2[*d++]++;
    H3[*d++]++;
  }

  return false;
}

template<class T>
bool csRadixSorter::DoPass (size_t pass, T* data, size_t size, uint32* histogram)
{
  uint32* Hcurr = histogram + (pass << 8);
  uint8 uniqueVal = *(((uint8*)data)+pass);
  
  return Hcurr[uniqueVal] != size;
}

void csRadixSorter::Sort (uint32* array, size_t size)
{
  //Unsigned integer version
  if(!array || size == 0)
    return;

  //Reserve space
  if(size != currentSize)
    ranksValid = false;

  Resize(size);

  //Stack-allocated histogram. As we do generation in a single pass we need
  //1024 entries in the histogram (4kb in total)
  uint32 histogram[256*4];
  //Link-table. Used to avoid one extra-level of indirection on offset calculation
  size_t* links[256];

  //Create histograms in a single pass over the data
  if(CreateHistogram(array, size, histogram))
  {
    //Already sorted, make sure ranks is up-to-date
    if(!ranksValid)
    {
      for(size_t i = 0; i < size; ++i)
      {
        ranks1[i] = i;
      }
    }
  }

  // Radix-sort. 4 passes at most
  for(size_t pass = 0; pass < 4; pass++)
  {
    if (DoPass (pass, array, size, histogram))
    {
      // Only have to deal with positives here
      links[0] = ranks2;
      uint32* Hpass = &histogram[pass<<8];
      for(size_t i = 1; i < 256; i++)
      {
        links[i] = links[i-1] + Hpass[i-1];
      }

      //Update ranks-list
      uint8* inputBytes = (uint8*)array;
#ifdef CS_BIG_ENDIAN
      inputBytes += (3-pass);
#else
      inputBytes += pass;
#endif
      if(!ranksValid)
      {
        for(size_t i = 0; i<size; i++)
        {
          *links[inputBytes[i<<2]]++ = i;
        }
        ranksValid = true;
      }
      else
      {
        size_t* indices = ranks1;
        size_t* indicesEnd = ranks1+size;
        while(indices != indicesEnd)
        {
          size_t id = *indices++;
          *links[inputBytes[id<<2]]++ = id;
        }
      }
      // Swap for next pass
      size_t* t = ranks1; ranks1 = ranks2; ranks2 = t;
    }
  }
}


void csRadixSorter::Sort (int32* array, size_t size)
{
  //Unsigned integer version
  if(!array || size == 0)
    return;

  //Reserve space
  if(size != currentSize)
    ranksValid = false;

  Resize(size);

  //Stack-allocated histogram. As we do generation in a single pass we need
  //1024 entries in the histogram (4kb in total)
  uint32 histogram[256*4];
  //Link-table. Used to avoid one extra-level of indirection on offset calculation
  size_t* links[256];

  //Create histograms in a single pass over the data
  if(CreateHistogram(array, size, histogram))
  {
    //Already sorted, make sure ranks is up-to-date
    if(!ranksValid)
    {
      for(size_t i = 0; i < size; ++i)
      {
        ranks1[i] = i;
      }
    }
  }

  // Handle negatives
  // An efficient way to compute the number of negatives values we'll have to deal with is simply to sum the 128
  // last values of the last histogram. Last histogram because that's the one for the Most Significant Byte,
  // responsible for the sign. 128 last values because the 128 first ones are related to positive numbers.
  size_t numNegatives = 0;
  uint32* H3neg = &histogram[768];
  for(size_t i = 128; i < 256; i++)
  {
    numNegatives += H3neg[i];
  }

  // Radix-sort. 4 passes at most
  for(size_t pass = 0; pass < 4; pass++)
  {
    if(DoPass(pass, array, size, histogram))
    {
      uint32* Hpass = &histogram[pass<<8];

      if(pass != 3)
      {
        // Only have to deal with positives here
        links[0] = ranks2;
        for(size_t i = 1; i < 256; i++)
        {
          links[i] = links[i-1] + Hpass[i-1];
        }
      }
      else
      {
        //negatives
        links[0] = ranks2 + numNegatives;
        for(size_t i = 1; i < 128; i++)
        {
          links[i] = links[i-1] + Hpass[i-1];
        }

        //fix the offset
        links[128] = ranks2;
        for(size_t i = 129; i < 256; i++)
        {
          links[i] = links[i-1] + Hpass[i-1];
        }
      }

      //Update ranks-list
      uint8* inputBytes = (uint8*)array;
#ifdef CS_BIG_ENDIAN
      inputBytes += (3-pass);
#else
      inputBytes += pass;
#endif
      if(!ranksValid)
      {
        for(size_t i = 0; i<size; i++)
        {
          *links[inputBytes[i<<2]]++ = i;
        }
        ranksValid = true;
      }
      else
      {
        size_t* indices = ranks1;
        size_t* indicesEnd = ranks1+size;
        while(indices != indicesEnd)
        {
          size_t id = *indices++;
          *links[inputBytes[id<<2]]++ = id;
        }
      }
      // Swap for next pass
      size_t* t = ranks1; ranks1 = ranks2; ranks2 = t;
    }
  }
}


void csRadixSorter::Sort (float* array, size_t size)
{
  //Unsigned integer version
  if(!array || size == 0)
    return;

  //Reserve space, 
  if(size != currentSize || true) //@@DISABLE temporal coherency for now, bugs in it
    ranksValid = false;

  Resize(size);

  //Stack-allocated histogram. As we do generation in a single pass we need
  //1024 entries in the histogram (4kb in total)
  uint32 histogram[256*4];
  //Link-table. Used to avoid one extra-level of indirection on offset calculation
  size_t* links[256];

  //Create histograms in a single pass over the data
  if(CreateHistogram(array, size, histogram))
  {
    //Already sorted, make sure ranks is up-to-date
    if(!ranksValid)
    {
      for(size_t i = 0; i < size; ++i)
      {
        ranks1[i] = i;
      }
    }
    return;
  }

  // Handle negatives
  // An efficient way to compute the number of negatives values we'll have to deal with is simply to sum the 128
  // last values of the last histogram. Last histogram because that's the one for the Most Significant Byte,
  // responsible for the sign. 128 last values because the 128 first ones are related to positive numbers.
  size_t numNegatives = 0;
  uint32* H3neg = &histogram[768];
  for(size_t i = 128; i < 256; i++)
  {
    numNegatives += H3neg[i];
  }

  // Radix-sort. 4 passes at most
  for(size_t pass = 0; pass < 3; pass++)
  {
    //only positives
    if(DoPass(pass, array, size, histogram))
    {
      uint32* Hpass = &histogram[pass<<8];

      // Only have to deal with positives here
      links[0] = ranks2;
      for(size_t i = 1; i < 256; i++)
      {
        links[i] = links[i-1] + Hpass[i-1];
      }

      //Update ranks-list
      uint8* inputBytes = (uint8*)array;
#ifdef CS_BIG_ENDIAN
      inputBytes += (3-pass);
#else
      inputBytes += pass;
#endif
      if(!ranksValid)
      {
        for(size_t i = 0; i<size; i++)
        {
          *links[inputBytes[i<<2]]++ = i;
        }
        ranksValid = true;
      }
      else
      {
        size_t* indices = ranks1;
        size_t* indicesEnd = ranks1+size;
        while(indices != indicesEnd)
        {
          size_t id = *indices++;
          *links[inputBytes[id<<2]]++ = id;
        }
      }
      // Swap for next pass
      size_t* t = ranks1; ranks1 = ranks2; ranks2 = t;
    }
  }

  //Last pass to handle negatives
  if(DoPass(3, array, size, histogram))
  {
    uint32* Hpass = &histogram[3<<8];
    links[0] = ranks2 + numNegatives;
    for(size_t i = 1; i < 128; i++)
    {
      links[i] = links[i-1] + Hpass[i-1];
    }
    //reverse order for negative numbers
    links[255] = ranks2;
    for(size_t i = 1; i < 128; i++)
    {
      links[255-i] = links[256-i] + Hpass[256-i];
    }
    for(size_t i = 128; i < 256; i++)
    {
      links[i] += Hpass[i];
    }

    //Update ranks-list
    uint32* inputAsUint = (uint32*)array;
    if(!ranksValid)
    {
      for(size_t i = 0; i<size; i++)
      {
        uint32 radix = inputAsUint[i]>>24;
        if(radix<128)
          *links[radix]++ = i;
        else
          *(--links[radix]) = i; //negative, reverse order
      }
      ranksValid = true;
    }
    else
    {
      for(size_t i = 0; i < size; i++)
      {
        uint32 radix = inputAsUint[ranks1[i]]>>24;
        if(radix<128)
          *links[radix]++ = ranks1[i];
        else
          *(--links[radix]) = ranks1[i]; //negative, reverse order
      }
    }
    // Swap for next pass
    size_t* t = ranks1; ranks1 = ranks2; ranks2 = t;
  }
  else
  {
    //still have to fix negatives :/
    uint8 uniqueVal = *(((uint8*)array)+3);
    if(uniqueVal >= 128)
    {
      if(!ranksValid)
      {
        for(size_t i = 0; i < size; i++)
        {
          ranks2[i] = size-i-1;
        }
        ranksValid = true;
      }
      else
      {
        for(size_t i = 0; i < size; i++)
        {
          ranks2[i] = ranks1[size-i-1];
        }
      }
      // Swap for next pass
      size_t* t = ranks1; ranks1 = ranks2; ranks2 = t;
    }
  }
}
