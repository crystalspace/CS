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
#include "csgeom/vector3.h"
#include "csutil/processor/sse2cppsimdtypes.h"

namespace CS
{
    namespace SIMD
    {
        typedef vector float Vector4f;

        /// Sets the 4 floating-point values to x.
        CS_FORCEINLINE Vector4f VectorSet(const float x)
        {
            Vector4 vec = (Vector4f) (x, x, x, x);
            return vec;
        }

        /// Sets highest to lowest floating-point values to x, y, z, w respectively.
        CS_FORCEINLINE Vector4f VectorSet(const float x, const float y, const float z, const float w)
        {
            Vector4f vec = (Vector4f) (x, y, z, w);
            return vec;
        }
        
        /// Sets the 4 floating-point values to zero.
        CS_FORCEINLINE Vector4f VectorSetZerof()
        {
            Vector4f vec = (Vector4f) (0, 0, 0, 0);
            return vec;
        }

        /// Loads four floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE Vector4f VectorLoad(const float* data)
        {
            return vec_ld(0, data);
        }

        /// Loads three floating-point values from a csVector3, forth is empty.
        CS_FORCEINLINE Vector4f VectorLoad(const csVector3& vec)
        {
            return vec_sel(vec, VectorSetZero(), VectorSet(0,0,0,1));
        }

        /// Loads three floating-point values from a csVector3, forth is garbage data.
        CS_FORCEINLINE Vector4f VectorLoad4(const csVector3& vec)
        {
            return vec_ld(0, vec.m);
        }

        /// Stores 4 floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE void VectorStore(float* data, const Vector4f& a)
        {
            vec_st(data,0,a);
        }

        /// Stores 3 floating-point values into a csVector3. Forth is not stored.
        CS_FORCEINLINE void VectorStore(csVector3* vec, const Vector4f& a)
        {
            float f1[4];

            vec_st(f1, a);
            vec->x = f1[0];
            vec->y = f1[1];
            vec->z = f1[2];
        }

        /// Stores 3 floating-point values into a csVector3. Forth is stored in next memory address.
        CS_FORCEINLINE void VectorStore4(csVector3* vec, const Vector4f& a)
        {
            vec_st(vec->m,0,a);
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
            return vec_add(a, b);
        }

        /// Subtracts the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorSub(const Vector4f& a, const Vector4f& b)
        {
            return vec_sub(a, b);
        }

        /// Multiplies the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMul(const Vector4f& a, const Vector4f& b)
        {
            return vec_madd(a, b, VectorSetZero());
        }

        /// Divides the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorDiv(const Vector4f& a, const Vector4f& b)
        {
            return VectorMul(a, vec_re(b))
        }

        /// Returns the negatives of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNeg(const Vector4f& a)
        {
            return VectorSub(VectorSetZero(), a);
        }

        /// Returns the four floating-point values of 'a multiplied by b and added to c'.
        CS_FORCEINLINE Vector4f VectorMultAdd(const Vector4f& a, const Vector4f& b, const Vector4f& c)
        {
            return vec_madd(a, b, c);
        }

        /// Returns the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorSqrt(const Vector4f& a)
        {
            return vec_re(vec_rsqrte(a));
        }

        /// Returns the reciprocals of the four floating-point values of a. 
        CS_FORCEINLINE Vector4f VectorRecip(const Vector4f& a)
        {
            return vec_re(a);
        }

        /// Returns the reciprocals of the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorRecipSqrt(const Vector4f& a)
        {
            return vec_rsqrte(a);
        }

        /// Returns the minima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMin(const Vector4f& a, const Vector4f& b)
        {
            return vec_min(a, b);
        }

        /// Returns the maxima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMax(const Vector4f& a, const Vector4f& b)
        {
            return vec_max(a, b);
        }

        /// Returns the bitwise AND of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorAND(const Vector4f& a, const Vector4f& b)
        {
            return vec_and(a, b);
        }

        /// Returns the bitwise OR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorOR(const Vector4f& a, const Vector4f& b)
        {
            return vec_or(a, b);
        }

        /// Returns the bitwise NOT of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNOT(const Vector4f& a)
        {
            return vec_xor(a, vec_cmpeq(a, a));
        }

        /// Returns the bitwise XOR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorXOR(const Vector4f& a, const Vector4f& b)
        {
            return vec_xor(a, b);
        }

        /// Compares the four floating-point values of a and b for equality.
        CS_FORCEINLINE Vector4f VectorEqual(const Vector4f& a, const Vector4f& b)
        {
            return vec_cmpeq(a, b);
        }

        /// Compares the four floating-point values of a to see if they are less than b.
        CS_FORCEINLINE Vector4f VectorLess(const Vector4f& a, const Vector4f& b)
        {
            return vec_cmplt(a, b);
        }

        /// Compares the four floating-point values of a to see if they are less than or equal to b.
        CS_FORCEINLINE Vector4f VectorLessEqual(const Vector4f& a, const Vector4f& b)
        {
            return vec_cmple(a, b);
        }

        /// Compares the four floating-point values of a and b for non-equality.
        CS_FORCEINLINE Vector4f VectorNotEqual(const Vector4f& a, const Vector4f& b)
        {
            return vec_nor(vec_cmpeq(a, b), VectorSetZero());
        }

        /// Transposes the matrix with rows a, b, c, d.
        CS_FORCEINLINE void MatrixTranspose(Vector4f& a, Vector4f& b, Vector4f& c, Vector4f& d)
        {
            Vector4f AC1 = vec_mergeh(a, c);
            Vector4f AC2 = vec_mergel(a, c);
            Vector4f BD1 = vec_mergeh(b, d);
            Vector4f BD2 = vec_mergel(b, d);
            a = vec_mergeh(AC1, BD1);
            b = vec_mergel(AC1, BD1);
            c = vec_mergeh(AC2, BD2);
            d = vec_mergel(AC2, BD2);
        }

        /// Returns four floating-point values of the x element of a.
        CS_FORCEINLINE Vector4f VectorSplatX(const Vector4f& a)
        {
            return vec_splat(a, 0);
        }

        /// Returns four floating-point values of the y element of a.
        CS_FORCEINLINE Vector4f VectorSplatY(const Vector4f& a)
        {
            return vec_splat(a, 1);
        }

        /// Returns four floating-point values of the z element of a.
        CS_FORCEINLINE Vector4f VectorSplatZ(const Vector4f& a)
        {
            return vec_splat(a, 2);
        }

        /// Returns four floating-point values of the w element of a.
        CS_FORCEINLINE Vector4f VectorSplatW(const Vector4f& a)
        {
            return vec_splat(a, 3);
        }

        /// Selectively merges the two vectors a and b.
        CS_FORCEINLINE Vector4f VectorSelect(const Vector4f& a, const Vector4f& b, const Vector4f& c)
        {
            return vec_sel(a, b, c);
        }

        /// Returns true if all four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAllTrue(const Vector4f& a)
        {
            return vec_all_ne(a, 0);
        }

        /// Returns true if any four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAnyTrue(const Vector4f& a)
        {
            return vec_any_ne(a, 0);
        }

        /// Selects and interleaves the lower two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeXY(const Vector4f& a, const Vector4f& b)
        {
            return vec_mergel(a, b);
        }

        /// Selects and interleaves the upper two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeZW(const Vector4f& a, const Vector4f& b)
        {
            return vec_mergeh(a, b);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_ALTIVEC_H

#endif // __AV_SIMD_TYPES_H__
