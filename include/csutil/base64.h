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

#ifndef __CS_CSUTIL_BASE64_H__
#define __CS_CSUTIL_BASE64_H__

#include "csextern.h"
#include "iutil/databuff.h"
#include "csutil/csstring.h"

struct iDataBuffer;

namespace CS
{
  namespace Utility
  {
    //@{
    /// Base64-encode the given data buffer.
    CS_CRYSTALSPACE_EXPORT csString EncodeBase64 (void* data, size_t size);
    inline csString EncodeBase64 (iDataBuffer* data)
    {
      if (!data) return "";
      return EncodeBase64 (data->GetData(), data->GetSize());
    }
    //@}
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_BASE64_H__
