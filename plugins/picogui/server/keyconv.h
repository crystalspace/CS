/*
    Crystal Space Key Code Converter for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_PICOGUI_SERVER_KEYCONV_H__
#define __CS_PICOGUI_SERVER_KEYCONV_H__

#include "iutil/evdefs.h"

struct iEvent;

/**
 * This class is to help converting CS events into PicoGUI events.
 */
class csPGKeyConverter
{
 private:
  static int Chars [256];
  static int Codes [CSKEY_SPECIAL_LAST - CSKEY_SPECIAL_FIRST + 1];

 public:
  /// Initialize the converter.
  static void Construct ();

  /// Converts a CS key stored in an event into a PicoGUI key.
  static int CS2PG (iEvent &);

  /// Converts a CS modifier (shift, etc.) in an event into one for PicoGui.
  static int CS2PGMod (iEvent &);
};

#endif
