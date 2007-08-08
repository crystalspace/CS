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

namespace CS
{
    namespace SIMD
    {
        typedef vector float Vector4;

        CS_FORCEINLINE Vector4 VectorSet(const float x)
        {
            Vector4 vec = (Vector4) (x, x, x, x);
            return vec;
        }

        CS_FORCEINLINE Vector4 VectorSet(const float x, const float y, const float z, const float w)
        {
            Vector4 vec = (Vector4) (x, y, z, w);
            return vec;
        }
        
        CS_FORCEINLINE Vector4 VectorSetZero()
        {
            Vector4 vec = (Vector4) (0, 0, 0, 0);
            return vec;
        }

        CS_FORCEINLINE Vector4 VectorLoad(const float* data)
        {
            return vec_ld(0, data);
        }

        CS_FORCEINLINE Vector4 VectorLoad(csVector3 vec)
        {
            return vec_sel(vec, VectorSetZero(), VectorSet(0,0,0,1));
        }

        CS_FORCEINLINE Vector4 VectorLoad4(csVector3 vec)
        {
            return vec_ld(0, vec.m);
        }

        CS_FORCEINLINE void VectorStore(float* data, Vector4 a)
        {
            vec_st(data,0,a);
        }

        CS_FORCEINLINE void VectorStore(csVector3* vec, Vector4 a)
        {
            float f1[4];

            vec_st(f1, a);
            vec->x = f1[0];
            vec->y = f1[1];
            vec->z = f1[2];
        }

        CS_FORCEINLINE void VectorStore4(csVector3* vec, Vector4 a)
        {
            vec_st(vec->m,0,a);
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
            return vec_add(a, b);
        }

        CS_FORCEINLINE Vector4 VectorSub(Vector4 a, Vector4 b)
        {
            return vec_sub(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMul(Vector4 a, Vector4 b)
        {
            return vec_madd(a, b, VectorSetZero());
        }

        CS_FORCEINLINE Vector4 VectorDiv(Vector4 a, Vector4 b)
        {
            return VectorMul(a, vec_re(b))
        }
        CS_FORCEINLINE Vector4 VectorNeg(Vector4 a)
        {
            return VectorSub(VectorSetZero(), a);
        }
        CS_FORCEINLINE Vector4 VectorMultAdd(Vector4 a, Vector4 b, Vector4 c)
        {
            return vec_madd(a, b, c);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 a)
        {
            return vec_re(vec_rsqrte(a));
        }

        CS_FORCEINLINE Vector4 VectorRecip(Vector4 a)
        {
            return vec_re(a);
        }

        CS_FORCEINLINE Vector4 VectorRecipSqrt(Vector4 a)
        {
            return vec_rsqrte(a);
        }

        CS_FORCEINLINE Vector4 VectorMin(Vector4 a, Vector4 b)
        {
            return vec_min(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMax(Vector4 a, Vector4 b)
        {
            return vec_max(a, b);
        }

        CS_FORCEINLINE Vector4 VectorAND(Vector4 a, Vector4 b)
        {
            return vec_and(a, b);
        }

        CS_FORCEINLINE Vector4 VectorOR(Vector4 a, Vector4 b)
        {
            return vec_or(a, b);
        }

        CS_FORCEINLINE Vector4 VectorNOT(Vector4 a)
        {
            return vec_xor(a, vec_cmpeq(a, a));
        }

        CS_FORCEINLINE Vector4 VectorXOR(Vector4 a, Vector4 b)
        {
            return vec_xor(a, b);
        }

        CS_FORCEINLINE Vector4 VectorEqual(Vector4 a, Vector4 b)
        {
            return vec_cmpeq(a, b);
        }

        CS_FORCEINLINE Vector4 VectorLess(Vector4 a, Vector4 b)
        {
            return vec_cmplt(a, b);
        }

        CS_FORCEINLINE Vector4 VectorLessEqual(Vector4 a, Vector4 b)
        {
            return vec_cmple(a, b);
        }

        CS_FORCEINLINE Vector4 VectorNotEqual(Vector4 a, Vector4 b)
        {
            return vec_nor(vec_cmpeq(a, b), VectorSetZero());
        }

        CS_FORCEINLINE void MatrixTranspose(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
        {
            Vector4 AC1 = vec_mergeh(a, c);
            Vector4 AC2 = vec_mergel(a, c);
            Vector4 BD1 = vec_mergeh(b, d);
            Vector4 BD2 = vec_mergel(b, d);
            a = vec_mergeh(AC1, BD1);
            b = vec_mergel(AC1, BD1);
            c = vec_mergeh(AC2, BD2);
            d = vec_mergel(AC2, BD2);
        }

        CS_FORCEINLINE Vector4 VectorSplatX(Vector4 a)
        {
            return vec_splat(a, 0);
        }

        CS_FORCEINLINE Vector4 VectorSplatY(Vector4 a)
        {
            return vec_splat(a, 1);
        }

        CS_FORCEINLINE Vector4 VectorSplatZ(Vector4 a)
        {
            return vec_splat(a, 2);
        }

        CS_FORCEINLINE Vector4 VectorSplatW(Vector4 a)
        {
            return vec_splat(a, 3);
        }

        CS_FORCEINLINE Vector4 VectorSelect(Vector4 a, Vector4 b, Vector4 c)
        {
            return vec_sel(a, b, c);
        }

        CS_FORCEINLINE bool VectorAllTrue(Vector4 a)
        {
            return vec_all_ne(a, 0);
        }

        CS_FORCEINLINE bool VectorAnyTrue(Vector4 a)
        {
            return vec_any_ne(a, 0);
        }

        CS_FORCEINLINE Vector4 VectorMergeXY(Vector4 a, Vector4 b)
        {
            return vec_mergel(a, b);
        }

        CS_FORCEINLINE Vector4 VectorMergeZW(Vector4 a, Vector4 b)
        {
            return vec_mergeh(a, b);
        }
    }
}

#else // Fall back to the C++ functions.

#include "simdtypes.h"

#endif // CS_HAS_ALTIVEC_H

#endif // __AV_SIMD_TYPES_H__
