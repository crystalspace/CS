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

#ifndef __SSE2_SIMD_TYPES_H__
#define __SSE2_SIMD_TYPES_H__

#ifdef CS_HAS_EMMINTRIN_H

#include <emmintrin.h>
#include "csutil/processor/ssesimdtypes.h"

namespace CS
{
    namespace SIMD
    {
        typedef __m128d Vector4d;

        /// Sets the 2 double-precision floating-point values to w.
        CS_FORCEINLINE Vector4d VectorSet(const double w)
        {
            return _mm_set1_pd(w);
        }

        /// Sets the upper double-precision floating-point value to w and the lower double-precision floating-point value to x.
        CS_FORCEINLINE Vector4d VectorSet(const double w, const double x)
        {
            return _mm_set_pd(w, x);
        }

        /// Sets the 2 double-precision floating-point values to zero.
        CS_FORCEINLINE Vector4d VectorSetZerod()
        {
            return _mm_setzero_pd();
        }

        /// Loads two double-precision floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE Vector4d VectorLoad(const double* data)
        {
            return _mm_loadu_pd(data);
        }

        /// Stores two double-precision, floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE void VectorStore(double* data, Vector4d a)
        {
            _mm_storeu_pd(data, a);
        }

        /// Returns the upper double-precision floating-point value of a.
        CS_FORCEINLINE double VectorGetW(Vector4d a)
        {
            float temp[2];
            VectorStore(temp, a);
            return temp[0];
        }

        /// Returns the lower double-precision floating-point value of a.
        CS_FORCEINLINE double VectorGetX(Vector4d a)
        {
            float temp[2];
            VectorStore(temp, a);
            return temp[1];
        }

        /// Adds the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorAdd(Vector4d a, Vector4d b)
        {
            return _mm_add_pd(a, b);
        }

        /// Subtracts the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorSub(Vector4d a, Vector4d b)
        {
            return _mm_sub_pd(a, b);
        }

        /// Multiplies the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMul(Vector4d a, Vector4d b)
        {
            return _mm_mul_pd(a, b);
        }

        /// Divides the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorDiv(Vector4d a, Vector4d b)
        {
            return _mm_div_pd(a, b);
        }

        /// Returns the square roots of the two double-precision floating point values of a.
        CS_FORCEINLINE Vector4d VectorSqrt(Vector4d a)
        {
            return _mm_sqrt_pd(a);
        }

        /// Returns the minima of the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMin(Vector4d a, Vector4d b)
        {
            return _mm_min_pd(a, b);
        }

        /// Returns the maxima of the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMax(Vector4d a, Vector4d b)
        {
            return _mm_max_pd(a, b);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_EMMINTRIN_H

#endif // __SSE2_SIMD_TYPES_H__
