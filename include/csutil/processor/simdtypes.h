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

namespace CS
{
    namespace SIMD
    {
        struct Vector4
        {
            float a;
            float b;
            float c;
            float d;
        };

        struct ReturnVector4 : public Vector4
        {
            CS_FORCEINLINE ReturnVector4(float _a, float _b, float _c, float _d)
            {
                a = _a;
                b = _b;
                c = _c;
                d = _d;
            }
        };

        CS_FORCEINLINE Vector4 VectorMul(Vector4 x, Vector4 y)
        {
            return ReturnVector4(x.a * y.a, x.b * y.b, x.c * y.c, x.d * y.d);
        }

        CS_FORCEINLINE Vector4 VectorAdd(Vector4 x, Vector4 y)
        {
            return ReturnVector4(x.a + y.a, x.b + y.b, x.c + y.c, x.d + y.d);
        }

        CS_FORCEINLINE Vector4 VectorSqrt(Vector4 x)
        {
            return ReturnVector4(sqrtf(x.a), sqrtf(x.b), sqrtf(x.c), sqrtf(x.d));
        }
    }
}

#endif // __SIMD_TYPES_H__
