/*
Copyright (C) 2005 by Andrew Mann

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

#ifndef SNDSYS_COMMON_STRUCTS_H
#define SNDSYS_COMMON_STRUCTS_H

#include "csutil/scf.h"

/**
* The sound format. This keeps information about the frequency, bits and
* channels of a sound data object.
*/
struct SndSysSoundFormat
{
  /// Frequency of the sound (hz)
  int Freq;
  /// number of bits per sample (8 or 16)
  int Bits;
  /// number of channels (1 or 2)
  int Channels;
};

// A single sound sample will be stored as a signed int
typedef int32 SoundSample;



#endif // #ifndef SNDSYS_COMMON_STRUCTS_H

