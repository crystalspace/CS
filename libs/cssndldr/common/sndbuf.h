/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#if !defined(__CSSOUNDBUFFER_H__)
#define __CSSOUNDBUFFER_H__

/**
 * Wave class :<br>
 *    It's the 'sound' PCM stored data<br>
 *    16 bits is signed (short)<br>
 *    8 bits is unsigned (unsigned char)<p>
 *
 * Warning : size is the number of samples not the size of data
 *  i.e. if it's mono 8 bit and size = 800, data size is 800
 *       if it's stereo 16 bit and size = 800, data size is 800*2*2 (two channels and 16 bits)
 */

class csSoundBuffer
{
public:
  ///
  csSoundBuffer(int frequency, bool bit16, bool stereo, bool sign, long size, void *data);
  ///
  ~csSoundBuffer();

  /// delete data
  void Clean();

  ///
  bool is16Bits();
  ///
  bool isStereo();
  ///
  int getFrequency();

  /// return number of samples ! not the size of data
  long getSize();
  ///
  void * getData();

  /// some functions of convertion (very usefull for adapt Wave to SoundDriver)
  bool convertTo(int toFrequence, bool toBit16, bool toStereo);
  ///
  bool convertStereoTo(bool toStereo);
  ///
  bool convert16bitTo(bool toBit16);
  ///
  bool convertFrequencyTo(int toFrequency);

  /// set 'void' sound in data
  void forceMute();
private:
  void *Data;
  long Size;
  int Frequency;
  bool Bit16;
  bool Stereo;
  bool Sign;
};

#endif // __CSSOUNDBUFFER_H__
