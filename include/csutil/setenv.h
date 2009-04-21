/*
    Copyright (C)2008 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_CSUTIL_SETENV_H__
#define __CS_CSUTIL_SETENV_H__

#include "csextern.h"

namespace CS
{
  namespace Utility
  {

    /**
     * Set an environment variable, much like POSIX setenv() but portably.
     * \param key Environment variable name.
     * \param value Value for environment variable.
     * \param overwrite If false, set the environment variable only if it does
     * not already have a value, else set it unconditionally.
     * \return 0 upon success, else -1 and errno is set with failure reason.
     */
    CS_CRYSTALSPACE_EXPORT int setenv(
	char const* key, char const* value, bool overwrite);

  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_SETENV_H__
