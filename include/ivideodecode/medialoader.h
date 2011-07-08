/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_MEDIALOADER_H__
#define __CS_MEDIALOADER_H__

/**\file
  * Video Player: loader 
  */

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iMediaContainer;

/**
  * The video loader is used to initialize the decoder
  */
struct iMediaLoader : public virtual iBase
{
  SCF_INTERFACE (iMediaLoader,0,1,0);

  /// Create an codec iMediaContainer from raw input data.
  //
  //  Optional pDescription may point to a brief description that will follow this data
  //   through any streams or sources created from it, and may be useful for display or
  //   diagnostic purposes.
  //
  //  Optional pMediaType may suggest the media type to be loaded, to reduce the guess work.
  //   Originally intended to be an enum, but res suggested a string of characters
  virtual csRef<iMediaContainer> LoadMedia (const char * pFileName, const char *pDescription=0, const char* pMediaType = "AutoDetect") = 0;
};

/** @} */

#endif // __CS_MEDIALOADER_H__
