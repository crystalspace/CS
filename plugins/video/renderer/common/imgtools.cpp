/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "sysdef.h"
#include "cs3d/common/imgtools.h"
#include "csutil/csvector.h"

//---------------------------------------------------------------------------

// Bits per pixel in hash
#define HASH_BPP 5 
// Mask for each color
#define HASH_MASK ( ( 1 << HASH_BPP ) - 1 )
// Size of the hash table
#define HASH_SIZE ( 1 << (3*HASH_BPP) )

// Hash structure.
struct RGBHash : public RGBPixel
{
  int count;
  int palIndex;
  RGBHash* next;

  inline static int index(const RGBPixel& c)
  { return ( (  c.red & HASH_MASK) << (2*HASH_BPP) ) + 
           ( (c.green & HASH_MASK) <<    HASH_BPP  ) + 
             ( c.blue & HASH_MASK); }
  ///
  RGBHash (const RGBPixel& c, RGBHash* hashnext = NULL) :
   RGBPixel(c), count(0), next(hashnext) {}
};

// comparison function for two RGBPalEntry values, for the qsort
int RGBPal_cmp (const void* v1, const void* v2)
{
  const RGBPalEntry* u1 = (RGBPalEntry*)v1;
  const RGBPalEntry* u2 = (RGBPalEntry*)v2;
  if (u1->count < u2->count) return 1;
   else if (u1->count > u2->count) return -1;

  // If the count is equal we sort on RGB value to make
  // sure that we get the same order every time.
  if (u1->red < u2->red) return 1;
   else if (u1->red > u2->red) return -1;
  if (u1->green < u2->green) return 1;
   else if (u1->green > u2->green) return -1;
  if (u1->blue < u2->blue) return 1;
   else if (u1->blue > u2->blue) return -1;

  return 0;
}

void ImageColorInfo::calc_table(const RGBPixel* buf, long size)
{
  RGBHash** hash; // hash table for color values
  color_table = NULL;  num_colors = 0;


  // allocate the palette entry table for the image
  palInd = new int [size];
  // initialize to the first entry in color table
  for (long k = 0; k < size; k++)
    palInd[k] = 0;

  // create a hash table
  CHK (hash = new RGBHash* [HASH_SIZE]);
  if (!hash) return;

  int i;
  long j;

  for (i = 0 ; i < HASH_SIZE ; i++) hash[i] = NULL;

  const RGBPixel* p = buf;
  for (j = 0; j < size; j++)
  {
    int idx = RGBHash::index(*p);
    RGBHash* h = hash[idx];
    while (h)
    {
      if (*h == *p) { h->count++;  break; }
      h = h->next;
    }
    if (!h)
    {
      CHK ( h = new RGBHash(*p, hash[idx]) );
      hash[idx] = h;
      num_colors++;
    }
    p++;
  } /* for (j) */

  // now create the color table
  CHK (color_table = new RGBPalEntry[num_colors]);
  for (i = 0, j = 0; i < HASH_SIZE; i++) {
    RGBHash* h = hash[i];
    while (h)
     // in theory, both conditions are always true, but we'll check anyways
     if (j <= num_colors && color_table) 
     {
      color_table[j].red = h->red;
      color_table[j].green = h->green;
      color_table[j].blue = h->blue;
      color_table[j].count = h->count;
      h->palIndex = j;
      j++;
      h = h->next;
     }
  }

  // build the palette entry table for the image
  p = buf;
  for (j = 0; j < size; j++)
  {
    int idx = RGBHash::index(*p);
    RGBHash* h = hash[idx];
    while (h && *h != *p )
      h = h->next;
    /// h is supposed to not NULL, but check it anyway
    if (h)
      palInd[j] =  h->palIndex ;
    p++;
  }

  // delete the hash entry
  for (i = 0; i < HASH_SIZE; i++)
    while (hash[i]) {
      RGBHash* h = hash[i]->next;
      CHK (delete hash[i]);
      hash[i] = h;
    }

      // free the hash table that was created
  CHK (delete [] hash);

}

ImageColorInfo::~ImageColorInfo()
{
  if (color_table) CHKB( delete[] color_table );
  if (palInd) CHKB( delete[] palInd );
}

//---------------------------------------------------------------------------
