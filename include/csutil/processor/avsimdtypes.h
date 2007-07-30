/*
Copyright (C) 2007 by Michael Gist

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

#ifndef __AV_SIMD_TYPES_H__
#define __AV_SIMD_TYPES_H__

#ifdef CS_HAS_ALTIVEC_H

#include <altivec.h>

namespace CS
{
    namespace SIMD
    {
        typedef vector float Vector4;

        CS_FORCEINLINE Vector4 VectorMul(Vector4 a, Vector4 b)
        {
            return vec_madd(a, b, 0.0f);
        }

        CS_FORCEINLINE Vector4 VectorAdd(Vector4 a, Vector4 b)
        {
            return vec_add(a, b);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 a)
        {
            return vec_re(vec_rsqrte(a));
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_ALTIVEC_H

#endif // __AV_SIMD_TYPES_H__
