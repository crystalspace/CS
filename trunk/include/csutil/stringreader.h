/*
    Crystal Space utility library: string reader
    Copyright (C) 2004 by Jorrit Tyberghein

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
#ifndef __CS_STRINGREADER_H__
#define __CS_STRINGREADER_H__

/**\file
 * Line-by-line reader
 */

#include "csutil/csstring.h"

/**
 * This is a convenience class that reads an input buffer line by line.
 * It takes care of OS specific line endings (CR or CR/LF).
 */
class CS_CRYSTALSPACE_EXPORT csStringReader
{
private:
  const char* input;
  const char* cur;

public:
  /**
   * Create an empty string reader.
   */
  csStringReader ();

  /**
   * Open a new string reader on the given input.
   * 'input' must point to a null terminated character buffer.
   */
  csStringReader (const char* input);

  /**
   * Set input buffer.
   * 'input' must point to a null terminated character buffer.
   */
  void SetInput (const char* input);

  /**
   * Reset the reading offset at the start of the input.
   */
  void Reset ();

  /**
   * Get the next line from the input and put it in the string
   * argument. Returns true on success or false if there are no more
   * lines left (the string will be made empty in that case).
   * Note that the returned string will not contain a newline at the end.
   */
  bool GetLine (csString& line);

  /**
   * Returns true if there are more lines.
   */
  bool HasMoreLines ();
};

#endif // __CS_STRINGREADER_H__

