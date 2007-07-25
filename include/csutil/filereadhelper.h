/*
    Copyright (C) 2001-2005 by Jorrit Tyberghein
	      (C) 2001 by Martin Geisse
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSUTIL_FILEREADHELPER_H__
#define __CS_CSUTIL_FILEREADHELPER_H__

/**\file
 * Convenience class for simple file element reading.
 */
 
#include "csextern.h"
#include "csutil/ref.h"
#include "iutil/vfs.h"

/**
 * Convenience class for simple file element reading.
 */
class CS_CRYSTALSPACE_EXPORT csFileReadHelper
{
  csRef<iFile> file;
public:
  /**
   * Initialize reader.
   * \param file The file to subsequently read from.
   */
  csFileReadHelper (iFile* file) : file (file) {}
    
  /// Get the wrapped file
  iFile* GetFile() { return file; }
  
  /// Skip a given amount of bytes
  void Skip (size_t num) { file->SetPos (file->GetPos() + num); }
  
  /**\name Sized type reading
   * \remarks No endian conversion is done.
   * @{ */
  //@{
  /**
   * Read a specifically sized data value from the file.
   * \param val The variable to read to.
   * \return False on EOF.
   */
  bool ReadInt8 (int8 &val);
  bool ReadUInt8 (uint8 &val);
  bool ReadInt16 (int16 &val);
  bool ReadUInt16 (uint16 &val);
  bool ReadInt32 (int32 &val);
  bool ReadUInt32 (uint32 &val);
  //@}
  /** @} */

  /**\name Character reading
   * @{ */
  /// Read a single character. Returns EOF if the stream has finished.
  int GetChar ();
  /// Return the next character (or EOF), but don't move forward
  int LookChar ();
  /** @} */

  /**\name String reading
   * @{ */
  /**
   * Read a line of text. Returns false if the stream has finished. If
   * 'OmitNewline' is true then the newline character will be thrown away.
   */
  bool GetString (char* buf, size_t len, bool OmitNewline = true);
  /**
   * Read an integer value from the stream that is stored as ASCII.
   */
  int ReadTextInt ();
  /**
   * Read a floating-point value from the stream that is stored as ASCII.
   */
  float ReadTextFloat ();
  /**
   * Skip any whitespace characters.
   */
  void SkipWhitespace ();
  /** @} */
};

#endif // __CS_CSUTIL_FILEREADHELPER_H__
