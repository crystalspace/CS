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

#ifndef __CS_MEDIAPARSER_H__
#define __CS_MEDIAPARSER_H__

/**\file
  * Video Player: loader 
  */

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "mediastructs.h"
#include <iutil/document.h>

/**
  * The media parser is used to read 
  */
struct iMadiaParser : public virtual iBase
{
  SCF_INTERFACE (iMadiaParser,0,1,0);

  // Parse an iDocumentNode into useable data for the iMediaLoader
  virtual bool Parse (iDocumentNode* doc) = 0;

  // Return the path for the media file that needs to be loaded
  virtual const char* GetMediaPath () = 0;

  // Return the type of the media that needs to be loaded
  virtual const char* GetMediaType () = 0;

  // Return a list of available language streams
  virtual csArray<Language> GetLanguages () = 0;
};
/** @} */

#endif // __CS_MEDIAPARSER_H__