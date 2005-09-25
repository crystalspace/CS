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

#ifndef __CS_IMESH_MDLCONV_H__
#define __CS_IMESH_MDLCONV_H__

/**\file
 * Mesh model converter
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

struct iDataBuffer;
struct iModelData;

/**
 * Structure describing a single format supported by a model converter.
 */
struct csModelConverterFormat
{
  /// Name of this format
  char *Name;
  /// Can this format be loaded ?
  bool CanLoad;
  /// Can this format be saved ?
  bool CanSave;

  // information about supported features of the loader may be added here.
  // A is supported only if the format *and* the loader support it,
  // unsupported otherwise.
};

SCF_VERSION (iModelConverter, 0, 0, 1);

/**
 * Model converter. Used to convert model data from an arbitrary format to
 * iModelData or vice versa.
 */
struct iModelConverter : public iBase
{
  /// Return the number of supported formats
  virtual size_t GetFormatCount () = 0;
  /// Return the description of a supported format
  virtual const csModelConverterFormat *GetFormat (size_t idx) = 0;

  /// Read a model file
  virtual csPtr<iModelData> Load (uint8* Buffer, size_t Size) = 0;

  /// Write data to a file
  virtual csPtr<iDataBuffer> Save (iModelData*, const char *Format) = 0;
};

/** @} */

#endif // __CS_IMESH_MDLCONV_H__

