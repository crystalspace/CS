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

#ifndef __SIMD_TYPES_H__
#define __SIMD_TYPES_H__

#include "csutil/processor/sse2cppsimdtypes.h"
#include "csgeom/vector3.h"

namespace CS
{
    namespace SIMD
    {
        struct Vector4f
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct ReturnVector4f : public Vector4f
        {
            CS_FORCEINLINE ReturnVector4f(const float _x, const float _y, const float _z, const float _w)
            {
                x = _x;
                y = _y;
                z = _z;
                w = _w;
            }
        };

        struct AccessVector4f
        {
            union
            {
                Vector4f vec;
                float fdata[4];
                int udata[4];
            };

            CS_FORCEINLINE AccessVector4f(const Vector4f& vec) : vec(vec) {}
        };


        /// Sets the 4 floating-point values to x.
        CS_FORCEINLINE Vector4f VectorSet(const float x)
        {
            return ReturnVector4f(x, x, x, x);
        }

        /// Sets highest to lowest floating-point values to x, y, z, w respectively.
        CS_FORCEINLINE Vector4f VectorSet(const float x, const float y, const float z, const float w)
        {
            return ReturnVector4f(x, y, z, w);
        }
        
        /// Sets the 4 floating-point values to zero.
        CS_FORCEINLINE Vector4f VectorSetZerof()
        {
            return ReturnVector4f(0.0f, 0.0f, 0.0f, 0.0f);
        }

        /// Returns the x floating-point value of a.
        CS_FORCEINLINE float VectorGetX(const Vector4f& a)
        {
            return a.x;
        }

        /// Returns the y floating-point value of a.
        CS_FORCEINLINE float VectorGetY(const Vector4f& a)
        {
            return a.y;
        }

        /// Returns the z floating-point value of a.
        CS_FORCEINLINE float VectorGetZ(const Vector4f& a)
        {
            return a.z;
        }

        /// Returns the w floating-point value of a.
        CS_FORCEINLINE float VectorGetW(const Vector4f& a)
        {
            return a.w;
        }

        /// Loads four floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE Vector4f VectorLoad(const float* data)
        {
            return ReturnVector4f(data[0], data[1], data[2], data[3]);
        }

        /// Loads three floating-point values from a csVector3, forth is empty.
        CS_FORCEINLINE Vector4f VectorLoad(const csVector3& vec)
        {
            Vector4f vec4;
            AccessVector4f a (vec4);
            a.fdata[0] = vec.x;
            a.fdata[1] = vec.y;
            a.fdata[2] = vec.z;
            return vec4;
        }

        /// Loads three floating-point values from a csVector3, forth is garbage data.
        CS_FORCEINLINE Vector4f VectorLoad4(const csVector3& vec)
        {
            return VectorLoad(vec);
        }

        /// Stores 4 floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE void VectorStore(float* data, const Vector4f& a)
        {
            data[0] = a.x;
            data[1] = a.y;
            data[2] = a.z;
            data[3] = a.w;
        }

        /// Stores 3 floating-point values into a csVector3. Forth is not stored.
        CS_FORCEINLINE void VectorStore(csVector3* vec, const Vector4f& a)
        {
            vec->x = a.x;
            vec->y = a.y;
            vec->z = a.z;
        }

        /// Stores 3 floating-point values into a csVector3. Forth is stored in next memory address.
        CS_FORCEINLINE void VectorStore4(csVector3* vec, const Vector4f& a)
        {
           vec->m[0] = a.x;
           vec->m[1] = a.y;
           vec->m[2] = a.z;
           vec->m[4] = a.w;
        }

        /// Adds the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorAdd(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                 a.x + b.x, a.y + b.y,
                 a.z + b.z, a.w + b.w);
        }

        /// Subtracts the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorSub(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                   a.x - b.x, a.y - b.y,
                   a.z - b.z, a.w - b.w);
        }

        /// Multiplies the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMul(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                 a.x * b.x, a.y * b.y,
                 a.z * b.z, a.w * b.w);
        }

        /// Divides the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorDiv(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                 a.x / b.x, a.y / b.y,
                 a.z / b.z, a.w / b.w);
        }

        /// Returns the negatives of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNeg(const Vector4f& a)
        {
            return ReturnVector4f(-a.x, -a.y, -a.z, -a.w);
        }

        /// Returns the four floating-point values of 'a multiplied by b and added to c'.
        CS_FORCEINLINE Vector4f VectorMultAdd(const Vector4f& a, const Vector4f& b, const Vector4f& c)
        {
            return ReturnVector4f(
                 a.x * b.x + c.x, a.y * b.y + c.y,
                 a.z * b.z + c.z, a.w * b.w + c.w);
        }

        /// Returns the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorSqrt(const Vector4f& a)
        {
            return ReturnVector4f(sqrtf(a.x), sqrtf(a.y), sqrtf(a.z), sqrtf(a.w));
        }

        /// Returns the reciprocals of the four floating-point values of a. 
        CS_FORCEINLINE Vector4f VectorRecip(const Vector4f& a)
        {
            return ReturnVector4f(1/a.x, 1/a.y, 1/a.z, 1/a.w);
        }

        /// Returns the reciprocals of the square roots of the four floating-point values of a.
        CS_FORCEINLINE Vector4f VectorRecipSqrt(const Vector4f& a)
        {
            return ReturnVector4f(
                 1/sqrtf(a.x), 1/sqrtf(a.y),
                 1/sqrtf(a.z), 1/sqrtf(a.w));
        }

        /// Returns the minima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMin(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                a.x < b.x ? a.x : b.x,
                a.y < b.y ? a.y : b.y,
                a.z < b.z ? a.z : b.z,
                a.w < b.w ? a.w : b.w);
        }

        /// Returns the maxima of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorMax(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                a.x > b.x ? a.x : b.x,
                a.y > b.y ? a.y : b.y,
                a.z > b.z ? a.z : b.z,
                a.w > b.w ? a.w : b.w);
        }

        /// Returns the bitwise AND of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorAND(const Vector4f& a, const Vector4f& b)
        {
            AccessVector4f _a (a);
            AccessVector4f _b (b);
            return ReturnVector4f(
                _a.udata[0] & _b.udata[0],
                _a.udata[1] & _b.udata[1],
                _a.udata[2] & _b.udata[2],
                _a.udata[3] & _b.udata[3]);
        }

        /// Returns the bitwise OR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorOR(const Vector4f& a, const Vector4f& b)
        {
            AccessVector4f _a (a);
            AccessVector4f _b (b);
            return ReturnVector4f(
                _a.udata[0] | _b.udata[0],
                _a.udata[1] | _b.udata[1],
                _a.udata[2] | _b.udata[2],
                _a.udata[3] | _b.udata[3]);
        }

        /// Returns the bitwise NOT of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorNOT(const Vector4f& a)
        {
            AccessVector4f _a (a);
            return ReturnVector4f(
               ~_a.udata[0],
               ~_a.udata[1],
               ~_a.udata[2],
               ~_a.udata[3]);
        }

        /// Returns the bitwise XOR of the four floating-point values of a and b.
        CS_FORCEINLINE Vector4f VectorXOR(const Vector4f& a, const Vector4f& b)
        {
            AccessVector4f _a (a);
            AccessVector4f _b (b);
            return ReturnVector4f(
                _a.udata[0] ^ _b.udata[0],
                _a.udata[1] ^ _b.udata[1],
                _a.udata[2] ^ _b.udata[2],
                _a.udata[3] ^ _b.udata[3]);
        }

        /// Compares the four floating-point values of a and b for equality.
        CS_FORCEINLINE Vector4f VectorEqual(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                (a.x == b.x) ? 0xffffffff : 0x0,
                (a.y == b.y) ? 0xffffffff : 0x0, 
                (a.z == b.z) ? 0xffffffff : 0x0,
                (a.w == b.w) ? 0xffffffff : 0x0);
        }

        /// Compares the four floating-point values of a to see if they are less than b.
        CS_FORCEINLINE Vector4f VectorLess(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                (a.x < b.x) ? 0xffffffff : 0x0,
                (a.y < b.y) ? 0xffffffff : 0x0, 
                (a.z < b.z) ? 0xffffffff : 0x0,
                (a.w < b.w) ? 0xffffffff : 0x0);
        }

        /// Compares the four floating-point values of a to see if they are less than or equal to b.
        CS_FORCEINLINE Vector4f VectorLessEqual(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                (a.x <= b.x) ? 0xffffffff : 0x0,
                (a.y <= b.y) ? 0xffffffff : 0x0, 
                (a.z <= b.z) ? 0xffffffff : 0x0,
                (a.w <= b.w) ? 0xffffffff : 0x0);
        }

        /// Compares the four floating-point values of a and b for non-equality.
        CS_FORCEINLINE Vector4f VectorNotEqual(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(
                (a.x != b.x) ? 0xffffffff : 0x0,
                (a.y != b.y) ? 0xffffffff : 0x0, 
                (a.z != b.z) ? 0xffffffff : 0x0,
                (a.w != b.w) ? 0xffffffff : 0x0);
        }

        /// Transposes the matrix with rows a, b, c, d.
        CS_FORCEINLINE void MatrixTranspose(Vector4f& a, Vector4f& b, Vector4f& c, Vector4f& d)
        {
            ReturnVector4f _a(a.x, b.x, c.x, d.x);
            ReturnVector4f _b(a.y, b.y, c.y, d.y);
            ReturnVector4f _c(a.z, b.z, c.z, d.z);
            ReturnVector4f _d(a.w, b.w, c.w, d.w);

            a = (Vector4f)_a;
            b = (Vector4f)_b;
            c = (Vector4f)_c;
            d = (Vector4f)_d;
        }

        /// Returns four floating-point values of the i'th element of a.
        CS_FORCEINLINE Vector4f VectorSplat(const Vector4f& a, const uint i)
        {
	        AccessVector4f a_ (a);
	        return ReturnVector4f(
                a_.fdata[i], a_.fdata[i],
                a_.fdata[i], a_.fdata[i]);
        }

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
            AccessVector4f _a (a);
            AccessVector4f _b (b);
            AccessVector4f _c (c);
            return ReturnVector4f(
                (_a.udata[0] & ~_c.udata[0]) | (_b.udata[0] & _c.udata[0]),
                (_a.udata[1] & ~_c.udata[1]) | (_b.udata[1] & _c.udata[1]),
                (_a.udata[2] & ~_c.udata[2]) | (_b.udata[2] & _c.udata[2]),
                (_a.udata[3] & ~_c.udata[3]) | (_b.udata[3] & _c.udata[3]));
        }

        /// Returns true if all four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAllTrue(const Vector4f& a)
        {
            return (a.x != 0 && a.y != 0 && a.z != 0 && a.w != 0);
        }

        /// Returns true if any four floating-point values of a are true.
        CS_FORCEINLINE bool VectorAnyTrue(const Vector4f& a)
        {
            return (a.x != 0 || a.y != 0 || a.z != 0 || a.w != 0);
        }

        /// Selects and interleaves the lower two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeXY(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(a.x, b.x, a.y, b.y);
        }

        /// Selects and interleaves the upper two floating-point values from a and b.
        CS_FORCEINLINE Vector4f VectorMergeZW(const Vector4f& a, const Vector4f& b)
        {
            return ReturnVector4f(a.z, b.z, a.w, b.w);
        }
    }
}

#endif // __SIMD_TYPES_H__
