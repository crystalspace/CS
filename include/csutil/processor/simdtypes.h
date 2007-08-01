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

/*
Type:
Vector4

Functions:

VectorSet(Float)
VectorSet(Float, Float, Float, Float)
VectorSetZero()
VectorGetX(Vector4)
VectorGetY(Vector4)
VectorGetZ(Vector4)
VectorGetW(Vector4)
VectorLoad(Float[])
VectorLoad(csVector3)
VectorLoad4(csVector3)
VectorStore(Float[], Vector4)
VectorStore(csVector3, Vector4)
VectorAdd(Vector4, Vector4)
VectorSub(Vector4, Vector4)
VectorMult(Vector4, Vector4)
VectorDiv(Vector4, Vector4)
VectorNeg(Vector4)
VectorMultAdd(Vector4, Vector4, Vector4)
VectorSqrt(Vector4)
VectorRecip(Vector4)
VectorRecipSqrt(Vector4)
VectorMin(Vector4, Vector4)
VectorMax(Vector4, Vector4)
VectorAND(Vector4, Vector4)
VectorOR(Vector4, Vector4)
VectorNOT(Vector4)
VectorXOR(Vector4, Vector4)
VectorEqual(Vector4, Vector4)
VectorLess(Vector4, Vector4)
VectorLessEqual(Vector4, Vector4)
VectorNotEqual(Vector4, Vector4)
MatrixTranspose(Vector4, Vector4, Vector4, Vector4)
VectorSplatX(Vector4)
VectorSplatY(Vector4)
VectorSplatZ(Vector4)
VectorSplatW(Vector4)
VectorSelect(Vector4, Vector4, Vector4)
VectorAllTrue(Vector4)
VectorAnyTrue(Vector4)
VectorMergeXY(Vector4, Vector4)
VectorMergeZW(Vector4, Vector4)

*/

#include "csgeom/vector3.h"

namespace CS
{
    namespace SIMD
    {
        struct Vector4
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct ReturnVector4 : public Vector4
        {
            CS_FORCEINLINE ReturnVector4(float _x, float _y, float _z, float _w)
            {
                x = _x;
                y = _y;
                z = _z;
                w = _w;
            }
        };

        struct AccessVector4
        {
            union
            {
                Vector4 vec;
                float fdata[4];
                int udata[4];
            };

            CS_FORCEINLINE AccessVector4(const Vector4& vec) : vec(vec) {}
        };


        CS_FORCEINLINE Vector4 VectorSet(float x)
        {
            return ReturnVector4(x, x, x, x);
        }

        CS_FORCEINLINE Vector4 VectorSet(float x, float y, float z, float w)
        {
            return ReturnVector4(x, y, z, w);
        }
        
        CS_FORCEINLINE Vector4 VectorSetZero()
        {
            return ReturnVector4(0, 0, 0, 0);
        }

        CS_FORCEINLINE float VectorGetX(Vector4 a)
        {
            return a.x;
        }

        CS_FORCEINLINE float VectorGetY(Vector4 a)
        {
            return a.y;
        }

        CS_FORCEINLINE float VectorGetZ(Vector4 a)
        {
            return a.z;
        }

        CS_FORCEINLINE float VectorGetW(Vector4 a)
        {
            return a.w;
        }

        CS_FORCEINLINE Vector4 VectorLoad(const float* data)
        {
            return ReturnVector4(data[0], data[1], data[2], data[3]);
        }

        CS_FORCEINLINE Vector4 VectorLoad(const csVector3& vec)
        {
            return ReturnVector4(vec.x, vec.y, vec.z, 0.0f);
        }

        CS_FORCEINLINE void VectorStore(float* data, Vector4 a)
        {
            data[0] = a.x;
            data[1] = a.y;
            data[2] = a.z;
            data[3] = a.w;
        }

        CS_FORCEINLINE void VectorStore(csVector3* vec, Vector4 a)
        {
            vec->x = a.x;
            vec->y = a.y;
            vec->z = a.z;
        }

        CS_FORCEINLINE Vector4 VectorAdd(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                 a.x + b.x, a.y + b.y,
                 a.z + b.z, a.w + b.w);
        }

        CS_FORCEINLINE Vector4 VectorSub(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                   a.x - b.x, a.y - b.y,
                   a.z - b.z, a.w - b.w);
        }

        CS_FORCEINLINE Vector4 VectorMul(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                 a.x * b.x, a.y * b.y,
                 a.z * b.z, a.w * b.w);
        }

        CS_FORCEINLINE Vector4 VectorDiv(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                 a.x / b.x, a.y / b.y,
                 a.z / b.z, a.w / b.w);
        }
        CS_FORCEINLINE Vector4 VectorNeg(Vector4 a)
        {
            return ReturnVector4(-a.x, -a.y, -a.z, -a.w);
        }
        CS_FORCEINLINE Vector4 VectorMultAdd(Vector4 a, Vector4 b, Vector4 c)
        {
            return ReturnVector4(
                 a.x * b.x + c.x, a.y * b.y + c.y,
                 a.z * b.z + c.z, a.w * b.w + c.w);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 a)
        {
            return ReturnVector4(sqrtf(a.x), sqrtf(a.y), sqrtf(a.z), sqrtf(a.w));
        }

        CS_FORCEINLINE Vector4 VectorRecip(Vector4 a)
        {
            return ReturnVector4(1/a.x, 1/a.y, 1/a.z, 1/a.w);
        }

        CS_FORCEINLINE Vector4 VectorRecipSqrt(Vector4 a)
        {
            return ReturnVector4(
                 1/sqrtf(a.x), 1/sqrtf(a.y),
                 1/sqrtf(a.z), 1/sqrtf(a.w));
        }

        CS_FORCEINLINE Vector4 VectorMin(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                a.x < b.x ? a.x : b.x,
                a.y < b.y ? a.y : b.y,
                a.z < b.z ? a.z : b.z,
                a.w < b.w ? a.w : b.w);
        }

        CS_FORCEINLINE Vector4 VectorMax(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                a.x > b.x ? a.x : b.x,
                a.y > b.y ? a.y : b.y,
                a.z > b.z ? a.z : b.z,
                a.w > b.w ? a.w : b.w);
        }

        CS_FORCEINLINE Vector4 VectorAND(Vector4 a, Vector4 b)
        {
            AccessVector4 _a (a);
            AccessVector4 _b (b);
            return ReturnVector4(
                _a.udata[0] & _b.udata[0],
                _a.udata[1] & _b.udata[1],
                _a.udata[2] & _b.udata[2],
                _a.udata[3] & _b.udata[3]);
        }

        CS_FORCEINLINE Vector4 VectorOR(Vector4 a, Vector4 b)
        {
            AccessVector4 _a (a);
            AccessVector4 _b (b);
            return ReturnVector4(
                _a.udata[0] | _b.udata[0],
                _a.udata[1] | _b.udata[1],
                _a.udata[2] | _b.udata[2],
                _a.udata[3] | _b.udata[3]);
        }

        CS_FORCEINLINE Vector4 VectorNOT(Vector4 a)
        {
            AccessVector4 _a (a);
            return ReturnVector4(
               ~_a.udata[0],
               ~_a.udata[1],
               ~_a.udata[2],
               ~_a.udata[3]);
        }

        CS_FORCEINLINE Vector4 VectorXOR(Vector4 a, Vector4 b)
        {
            AccessVector4 _a (a);
            AccessVector4 _b (b);
            return ReturnVector4(
                _a.udata[0] ^ _b.udata[0],
                _a.udata[1] ^ _b.udata[1],
                _a.udata[2] ^ _b.udata[2],
                _a.udata[3] ^ _b.udata[3]);
        }

        CS_FORCEINLINE Vector4 VectorEqual(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                (a.x == b.x) ? 0xffffffff : 0x0,
                (a.y == b.y) ? 0xffffffff : 0x0, 
                (a.z == b.z) ? 0xffffffff : 0x0,
                (a.w == b.w) ? 0xffffffff : 0x0);
        }

        CS_FORCEINLINE Vector4 VectorLess(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                (a.x < b.x) ? 0xffffffff : 0x0,
                (a.y < b.y) ? 0xffffffff : 0x0, 
                (a.z < b.z) ? 0xffffffff : 0x0,
                (a.w < b.w) ? 0xffffffff : 0x0);
        }

        CS_FORCEINLINE Vector4 VectorLessEqual(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                (a.x <= b.x) ? 0xffffffff : 0x0,
                (a.y <= b.y) ? 0xffffffff : 0x0, 
                (a.z <= b.z) ? 0xffffffff : 0x0,
                (a.w <= b.w) ? 0xffffffff : 0x0);
        }

        CS_FORCEINLINE Vector4 VectorNotEqual(Vector4 a, Vector4 b)
        {
            return ReturnVector4(
                (a.x != b.x) ? 0xffffffff : 0x0,
                (a.y != b.y) ? 0xffffffff : 0x0, 
                (a.z != b.z) ? 0xffffffff : 0x0,
                (a.w != b.w) ? 0xffffffff : 0x0);
        }

        CS_FORCEINLINE void MatrixTranspose(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
        {
            ReturnVector4 _a(a.x, b.x, c.x, d.x);
            ReturnVector4 _b(a.y, b.y, c.y, d.y);
            ReturnVector4 _c(a.z, b.z, c.z, d.z);
            ReturnVector4 _d(a.w, b.w, c.w, d.w);

            a = (Vector4)_a;
            b = (Vector4)_b;
            c = (Vector4)_c;
            d = (Vector4)_d;
        }

        CS_FORCEINLINE Vector4 VectorSplat(Vector4 a, uint i)
        {
	        AccessVector4 a_ (a);
	        return ReturnVector4(
                a_.fdata[i], a_.fdata[i],
                a_.fdata[i], a_.fdata[i]);
        }

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
            AccessVector4 _a (a);
            AccessVector4 _b (b);
            AccessVector4 _c (c);
            return ReturnVector4(
                (_a.udata[0] & ~_c.udata[0]) | (_b.udata[0] & _c.udata[0]),
                (_a.udata[1] & ~_c.udata[1]) | (_b.udata[1] & _c.udata[1]),
                (_a.udata[2] & ~_c.udata[2]) | (_b.udata[2] & _c.udata[2]),
                (_a.udata[3] & ~_c.udata[3]) | (_b.udata[3] & _c.udata[3]));
        }

        CS_FORCEINLINE bool VectorAllTrue(Vector4 a)
        {
            return (a.x == 0 && a.y == 0 && a.z == 0 && a.w == 0);
        }

        CS_FORCEINLINE bool VectorAnyTrue(Vector4 a)
        {
            return (a.x != 0 || a.y != 0 || a.z != 0 || a.w != 0);
        }

        CS_FORCEINLINE Vector4 VectorMergeXY(Vector4 a, Vector4 b)
        {
            return ReturnVector4(a.x, b.x, a.y, b.y);
        }

        CS_FORCEINLINE Vector4 VectorMergeZW(Vector4 a, Vector4 b)
        {
            return ReturnVector4(a.z, b.z, a.w, b.w);
        }
    }
}

#endif // __SIMD_TYPES_H__
