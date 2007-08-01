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
#include "csgeom/vector3.h"

namespace CS
{
    namespace SIMD
    {
        typedef __m128 Vector4;

        CS_FORCEINLINE Vector4 VectorSet(const float x)
        {
            return _mm_set1_ps(x);
        }

        CS_FORCEINLINE Vector4 VectorSet(const float x, const float y, const float z, const float w)
        {
            return _mm_setr_ps(x, y, z, w);
        }
        
        CS_FORCEINLINE Vector4 VectorSetZero()
        {
            return _mm_setzero_ps();
        }

        CS_FORCEINLINE Vector4 VectorLoad(const float* data)
        {
            return _mm_load_ps(data);
        }

        CS_FORCEINLINE Vector4 VectorLoad(csVector3 vec)
        {
            return _mm_load_ps(vec.m);
        }

        CS_FORCEINLINE void VectorStore(float* data, Vector4 a)
        {
            _mm_store_ps(data, a);
        }

        CS_FORCEINLINE void VectorStore(csVector3* vec, Vector4 a)
        {
            _mm_store_ps(vec->m, a);
        }

        CS_FORCEINLINE float VectorGetX(Vector4 a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[0];
        }

        CS_FORCEINLINE float VectorGetY(Vector4 a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[1];
        }

        CS_FORCEINLINE float VectorGetZ(Vector4 a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[2];
        }

        CS_FORCEINLINE float VectorGetW(Vector4 a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[3];
        }

        CS_FORCEINLINE Vector4 VectorAdd(Vector4 a, Vector4 b)
        {
            return _mm_add_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorSub(Vector4 a, Vector4 b)
        {
            return _mm_sub_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMul(Vector4 a, Vector4 b)
        {
            return _mm_mul_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorDiv(Vector4 a, Vector4 b)
        {
            return _mm_div_ps(a, b);
        }
        CS_FORCEINLINE Vector4 VectorNeg(Vector4 a)
        {
            return _mm_sub_ps(_mm_setzero_ps(), a);
        }
        CS_FORCEINLINE Vector4 VectorMultAdd(Vector4 a, Vector4 b, Vector4 c)
        {
            return _mm_add_ps(_mm_mul_ps(a, b), c);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 a)
        {
            return _mm_sqrt_ps(a);
        }

        CS_FORCEINLINE Vector4 VectorRecip(Vector4 a)
        {
            return _mm_rcp_ps(a);
        }

        CS_FORCEINLINE Vector4 VectorRecipSqrt(Vector4 a)
        {
            return _mm_rsqrt_ps(a);
        }

        CS_FORCEINLINE Vector4 VectorMin(Vector4 a, Vector4 b)
        {
            return _mm_min_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMax(Vector4 a, Vector4 b)
        {
            return _mm_max_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorAND(Vector4 a, Vector4 b)
        {
            return _mm_and_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorOR(Vector4 a, Vector4 b)
        {
            return _mm_or_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorNOT(Vector4 a)
        {
            return _mm_xor_ps(a, _mm_cmpeq_ps(a, a));
        }

        CS_FORCEINLINE Vector4 VectorXOR(Vector4 a, Vector4 b)
        {
            return _mm_xor_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorEqual(Vector4 a, Vector4 b)
        {
            return _mm_cmpeq_ss(a, b);
        }

        CS_FORCEINLINE Vector4 VectorLess(Vector4 a, Vector4 b)
        {
            return _mm_cmplt_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorLessEqual(Vector4 a, Vector4 b)
        {
            return _mm_cmple_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorNotEqual(Vector4 a, Vector4 b)
        {
            return _mm_cmpneq_ps(a, b);
        }

        CS_FORCEINLINE void MatrixTranspose(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
        {
            _MM_TRANSPOSE4_PS(a, b, c, d);
        }

#define VectorSplat(a, component)_mm_shuffle_ps((a), (a), _MM_SHUFFLE((component), (component), (component), (component)))

        CS_FORCEINLINE Vector4 VectorSplatX(Vector4 a)
        {
            return VectorSplat(a, 0);
        }

        CS_FORCEINLINE Vector4 VectorSplatY(Vector4 a)
        {
            return VectorSplat(a, 1);
        }

        CS_FORCEINLINE Vector4 VectorSplatZ(Vector4 a)
        {
            return VectorSplat(a, 2);
        }

        CS_FORCEINLINE Vector4 VectorSplatW(Vector4 a)
        {
            return VectorSplat(a, 3);
        }

        CS_FORCEINLINE Vector4 VectorSelect(Vector4 a, Vector4 b, Vector4 c)
        {
            return _mm_or_ps(_mm_andnot_ps(c, a), _mm_and_ps(c, b));
        }

        CS_FORCEINLINE bool VectorAllTrue(Vector4 a)
        {
            return (_mm_movemask_ps(a) == 0xf);
        }

        CS_FORCEINLINE bool VectorAnyTrue(Vector4 a)
        {
            return (_mm_movemask_ps(a) != 0);
        }

        CS_FORCEINLINE Vector4 VectorMergeXY(Vector4 a, Vector4 b)
        {
            return _mm_unpacklo_ps(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMergeZW(Vector4 a, Vector4 b)
        {
            return _mm_unpackhi_ps(a, b);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_XMMINTRIN_H

#endif // __SSE_SIMD_TYPES_H__
