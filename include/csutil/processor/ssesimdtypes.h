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

#ifndef __SSE_SIMD_TYPES_H__
#define __SSE_SIMD_TYPES_H__

#ifdef CS_HAS_XMMINTRIN_H

#include <xmmintrin.h>

namespace CS
{
    namespace SIMD
    {
        typedef __m128 Vector4;

        CS_FORCEINLINE Vector4 VectorMul(Vector4 a, Vector4 b)
        {
            return _mm_mul_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorAdd(Vector4 a, Vector4 b)
        {
            return _mm_add_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 a)
        {
            return _mm_sqrt_ps(a);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_XMMINTRIN_H

#endif // __SSE_SIMD_TYPES_H__
