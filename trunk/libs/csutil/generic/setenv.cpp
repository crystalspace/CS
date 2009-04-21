/*
    Copyright (C) 2008 by Eric Sunshine <sunshine@sunshineco.com>

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

#include "cssysdef.h"
#include "csutil/setenv.h"

namespace CS
{
  namespace Utility
  {

    int setenv(char const* key, char const* value, bool overwrite)
    {
#if defined(CS_HAVE_SETENV)
      return ::setenv(key, value, overwrite);
#elif defined(CS_HAVE_PUTENV)
      if (overwrite || getenv(key) == 0)
	{
	  // The string given to putenv() becomes part of the environment and,
	  // thus, can not be transient. Therefore, we never free the allocated
	  // buffer.
	  char* buff = (char*)malloc(strlen(key) + strlen(value) + 2);
	  sprintf(buff, "%s=%s", key, value);
	  return putenv(buff);
	}
      return 0;
#else
#error Platform supports neither setenv() nor putenv()
#endif
    }

  } // namespace Utility
} // namespace CS
