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
#include "csutil/processor/sse2cppsimdtypes.h"

namespace CS
{
    namespace SIMD
    {
        typedef __m128 Vector4f;

        /// Sets the 4 floating-point values to x.
        CS_FORCEINLINE Vector4f VectorSet(const float x)
        {
            return _mm_set1_ps(x);
        }

        /// Sets highest to lowest floating-point values to x, y, z, w respectively.
        CS_FORCEINLINE Vector4f VectorSet(const float x, const float y, const float z, const float w)
        {
            return _mm_set_ps(x, y, z, w);
        }
        
        /// Sets the 4 floating-point values to zero.
        CS_FORCEINLINE Vector4f VectorSetZerof()
        {
            return _mm_setzero_ps();
        }


        /// Loads four floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE Vector4f VectorLoad(const float* data)
        {
            return _mm_loadu_ps(data);
        }

        /// Loads three floating-point values from a csVector3, forth is empty.
        CS_FORCEINLINE Vector4f VectorLoad(const csVector3& vec)
        {
            Vector4f x = _mm_load_ss(&vec.x); //x 0 0 0
            Vector4f y = _mm_load_ss(&vec.y); //y 0 0 0
            Vector4f z = _mm_load_ss(&vec.z); //z 0 0 0
 
            Vector4f tmp = _mm_unpacklo_ps(x,y); //x y 0 0
            return _mm_shuffle_ps(tmp,z, _MM_SHUFFLE(0,1,0,1)); // x y z 0
        }

        /// Loads three floating-point values from a csVector3, forth is garbage data.
        CS_FORCEINLINE Vector4f VectorLoad4(const csVector3& vec)
        {
            return _mm_loadu_ps(vec.m);
        }

        /// Stores 4 floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE void VectorStore(float* data, const Vector4f& a)
        {
            _mm_storeu_ps(data, a);
        }

        /// Stores 3 floating-point values into a csVector3. Forth is not stored.
        CS_FORCEINLINE void VectorStore(csVector3* vec, const Vector4f& a)
        {
            _mm_store_ss(&vec->x, a);
            _mm_store_ss(&vec->y, _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1)));
            _mm_store_ss(&vec->z, _mm_unpackhi_ps(a,a));
        }

        /// Stores 3 floating-point values into a csVector3. Forth is stored in next memory address.
        CS_FORCEINLINE void VectorStore4(csVector3* vec, const Vector4f& a)
        {
            _mm_storeu_ps(vec->m, a);
        }

        /// Returns the x floating-point value of a.
        CS_FORCEINLINE float VectorGetX(const Vector4f& a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[0];
        }

        /// Returns the y floating-point value of a.
        CS_FORCEINLINE float VectorGetY(const Vector4f& a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[1];
        }

        /// Returns the z floating-point value of a.
        CS_FORCEINLINE float VectorGetZ(const Vector4f& a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[2];
        }

        /// Returns the w floating-point value of a.
        CS_FORCEINLINE float VectorGetW(const Vector4f& a)
        {
            float temp[4];
            VectorStore(temp, a);
            return temp[3];
        }

        /// Adds the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorAdd(const Vector4f& a, const Vector4f& b)
        {
            return _mm_add_ps(a, b);
        }

        /// Subtracts the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorSub(const Vector4f& a, const Vector4f& b)
        {
            return _mm_sub_ps(a, b);
        }

        /// Multiplies the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMul(const Vector4f& a, const Vector4f& b)
        {
            return _mm_mul_ps(a, b);
        }

        /// Divides the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorDiv(const Vector4f& a, const Vector4f& b)
        {
            return _mm_div_ps(a, b);
        }

        /// Returns the negatives of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNeg(const Vector4f& a)
        {
            return _mm_sub_ps(_mm_setzero_ps(), a);
        }

        /// Returns the four floating-point values of 'a multiplied by b and added to c'.
        CS_FORCEINLINE Vector4f VectorMultAdd(const Vector4f& a, const Vector4f& b, const Vector4f& c)
        {
            return _mm_add_ps(_mm_mul_ps(a, b), c);
        }

        /// Returns the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorSqrt(const Vector4f& a)
        {
            return _mm_sqrt_ps(a);
        }

        /// Returns the reciprocals of the four floating-point values of a. 
        CS_FORCEINLINE Vector4f VectorRecip(const Vector4f& a)
        {
            return _mm_rcp_ps(a);
        }

        /// Returns the reciprocals of the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorRecipSqrt(const Vector4f& a)
        {
            return _mm_rsqrt_ps(a);
        }

        /// Returns the minima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMin(const Vector4f& a, const Vector4f& b)
        {
            return _mm_min_ps(a, b);
        }

        /// Returns the maxima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMax(const Vector4f& a, const Vector4f& b)
        {
            return _mm_max_ps(a, b);
        }

        /// Returns the bitwise AND of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorAND(const Vector4f& a, const Vector4f& b)
        {
            return _mm_and_ps(a, b);
        }

        /// Returns the bitwise OR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorOR(const Vector4f& a, const Vector4f& b)
        {
            return _mm_or_ps(a, b);
        }

        /// Returns the bitwise NOT of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNOT(const Vector4f& a)
        {
            return _mm_xor_ps(a, _mm_cmpeq_ps(a, a));
        }

        /// Returns the bitwise XOR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorXOR(const Vector4f& a, const Vector4f& b)
        {
            return _mm_xor_ps(a, b);
        }

        /// Compares the four floating-point values of a and b for equality.
        CS_FORCEINLINE Vector4f VectorEqual(const Vector4f& a, const Vector4f& b)
        {
            return _mm_cmpeq_ps(a, b);
        }

        /// Compares the four floating-point values of a to see if they are less than b.
        CS_FORCEINLINE Vector4f VectorLess(const Vector4f& a, const Vector4f& b)
        {
            return _mm_cmplt_ps(a, b);
        }

        /// Compares the four floating-point values of a to see if they are less than or equal to b.
        CS_FORCEINLINE Vector4f VectorLessEqual(const Vector4f& a, const Vector4f& b)
        {
            return _mm_cmple_ps(a, b);
        }

        /// Compares the four floating-point values of a and b for non-equality.
        CS_FORCEINLINE Vector4f VectorNotEqual(const Vector4f& a, const Vector4f& b)
        {
            return _mm_cmpneq_ps(a, b);
        }

        /// Transposes the matrix with rows a, b, c, d.
        CS_FORCEINLINE void MatrixTranspose(Vector4f& a, Vector4f& b, Vector4f& c, Vector4f& d)
        {
            _MM_TRANSPOSE4_PS(a, b, c, d);
        }

#define VectorSplat(a, component)_mm_shuffle_ps((a), (a), _MM_SHUFFLE((component), (component), (component), (component)))

        /// Returns four floating-point values of the x element of a.
        CS_FORCEINLINE Vector4f VectorSplatX(const Vector4f& a)
        {
            return VectorSplat(a, 0);
        }

        /// Returns four floating-point values of the y element of a.
        CS_FORCEINLINE Vector4f VectorSplatY(const Vector4f& a)
        {
            return VectorSplat(a, 1);
        }

        /// Returns four floating-point values of the z element of a.
        CS_FORCEINLINE Vector4f VectorSplatZ(const Vector4f& a)
        {
            return VectorSplat(a, 2);
        }

        /// Returns four floating-point values of the w element of a.
        CS_FORCEINLINE Vector4f VectorSplatW(const Vector4f& a)
        {
            return VectorSplat(a, 3);
        }

        /// Selectively merges the two vectors a and b.
        CS_FORCEINLINE Vector4f VectorSelect(const Vector4f& a, const Vector4f& b, const Vector4f& c)
        {
            return _mm_or_ps(_mm_andnot_ps(c, a), _mm_and_ps(c, b));
        }

        /// Returns true if all four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAllTrue(const Vector4f& a)
        {
            return (_mm_movemask_ps(a) == 0xf);
        }

        /// Returns true if any four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAnyTrue(const Vector4f& a)
        {
            return (_mm_movemask_ps(a) != 0);
        }

        /// Selects and interleaves the lower two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeXY(const Vector4f& a, const Vector4f& b)
        {
            return _mm_unpacklo_ps(a, b);
        }

        /// Selects and interleaves the upper two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeZW(const Vector4f& a, const Vector4f& b)
        {
            return _mm_unpackhi_ps(a, b);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_XMMINTRIN_H

#endif // __SSE_SIMD_TYPES_H__
