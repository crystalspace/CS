/*
  Copyright (C) 2011 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csutil/digest.h"

namespace CS
{
  namespace Utility
  {
    namespace Checksum
    {
      /* These methods are here (and not inline in the header) b/c
       * usage of the PRI* macros breaks the Python bindings
       * building... :| */
      csString DigestFormat::HexString (const uint8* data, uint size)
      {
	csString s;
	for (uint i = 0; i < size; i++)
	  s.AppendFmt ("%02" PRIx8, data[i]);
	return s;
      }
      
      csString DigestFormat::HEXString (const uint8* data, uint size)
      {
	csString s;
	for (uint i = 0; i < size; i++)
	  s.AppendFmt ("%02" PRIX8, data[i]);
	return s;
      }
    } // namespace Checksum
  } // namespace Utility
} // namespace CS
