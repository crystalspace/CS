/*
    Copyright (C) 2005 by Jorrit Tyberghein
		  2005 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_IMAGELOADER_OPTIONSPARSER_H__
#define __CS_CSPLUGINCOMMON_IMAGELOADER_OPTIONSPARSER_H__

/**\file
 * Base classes for image loaders.
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/hash.h"

/**
 * \addtogroup plugincommon
 * @{ */

/**
 * Helper class to parse options strings passed to iImageIO::Save().
 * Options are a comma-separated list and can be specified either like 
 * "option" or "option=value".
 */
class CS_CRYSTALSPACE_EXPORT csImageLoaderOptionsParser
{
  csHash<csString, csString> optValues;
public:
  /// Initialize parser from a given options string.
  csImageLoaderOptionsParser (const char* options);

  //@{
  /**
   * Fetch an option.
   * \param key The name of the option to retrieve.
   * \param v The variable that will receive the parsed value.
   * \return Whether the value could be fetched.
   */
  bool GetInt (const char* key, int& v) const;
  bool GetBool (const char* key, bool& v) const;
  bool GetFloat (const char* key, float& v) const;
  bool GetString (const char* key, csString& v) const;
  //@}
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_IMAGELOADER_OPTIONSPARSER_H__
