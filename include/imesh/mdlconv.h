/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __IVARIA_MDLCONV_H__
#define __IVARIA_MDLCONV_H__

#include "csutil/scf.h"

struct iModelData;
struct iDataBuffer;

SCF_VERSION (iModelConverter, 0, 0, 1);

struct iModelConverter
{
  /// Return the number of supported formats
  virtual int GetNumFormats () = 0;
  /// Return the description of a supported format
  virtual const char *GetFormat (int idx) = 0;
  /// Check whether the given format is supported
  virtual bool SupportsFormat (const char *Format) = 0;

  /// Read a model file
  virtual iModelData* Load (UByte* Buffer, ULong Size) = 0;

  /// Write data to a file
  virtual iDataBuffer* Save (iModelData*, const char *Format) = 0;
};

#endif // __IVARIA_MDLCONV_H__
