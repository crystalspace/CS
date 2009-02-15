/*
  Copyright (C) 2008 by Frank Richter

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

#include "csutil/base64.h"

#include "iutil/databuff.h"

namespace CS
{
  namespace Utility
  {
    csString EncodeBase64 (iDataBuffer* data)
    {
      // Empty buffer is encoded to empty string.
      if (!data || (data->GetSize() == 0))
        return "";
    
      static const char encodeChars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
      csString ret;
      uint8* ptr = data->GetUint8();
      size_t bytes = data->GetSize();
      ret.SetCapacity (((bytes+2)/3)*4);
      while (bytes >= 3)
      {
	uint v = (ptr[0] << 16) | (ptr[1] << 8) | (ptr[2]);
	ptr += 3;
	bytes -= 3;
	ret << encodeChars[(v >> 18)];
	ret << encodeChars[(v >> 12) & 0x3f];
	ret << encodeChars[(v >> 6) & 0x3f];
	ret << encodeChars[v & 0x3f];
      }
      if (bytes > 0)
      {
	uint v = (ptr[0] << 16);
	if (bytes > 1)
	  v |= (ptr[1] << 8);
	data += 3;
	bytes -= 3;
	ret << encodeChars[(v >> 18)];
	ret << encodeChars[(v >> 12) & 0x3f];
	if (bytes > 1)
	{
	  ret << encodeChars[(v >> 6) & 0x3f];
	  ret << "=";
	}
	else
	  ret << "==";
      }
      
      return ret;
    }
  } // namespace Utility
} // namespace CS
