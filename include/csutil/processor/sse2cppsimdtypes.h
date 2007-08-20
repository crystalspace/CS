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

#ifndef __SSE2_CPP_SIMD_TYPES_H__
#define __SSE2_CPP_SIMD_TYPES_H__

namespace CS
{
    namespace SIMD
    {
        struct Vector4d
        {
            double w; // Highest
            double x; // Lowest
        };

        struct ReturnVector4d : public Vector4d
        {
            CS_FORCEINLINE ReturnVector4d(const double _w, const double _x)
            {
                w = _w;
                x = _x;
            }
        };

        /// Sets the 2 double-precision floating-point values to w.
        CS_FORCEINLINE Vector4d VectorSet(const double w)
        {
            return ReturnVector4d(w, w);
        }

        /// Sets the upper double-precision floating-point value to w and the lower double-precision floating-point value to x.
        CS_FORCEINLINE Vector4d VectorSet(const double w, const double x)
        {
            return ReturnVector4d(w, x);
        }

        /// Sets the 2 double-precision floating-point values to zero.
        CS_FORCEINLINE Vector4d VectorSetZerod()
        {
            return ReturnVector4d(0.0, 0.0);
        }

        /// Loads two double-precision floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE Vector4d VectorLoad(const double* data)
        {
            return ReturnVector4d(data[0], data[1]);
        }

        /// Stores two double-precision, floating-point values. Address 'data' can be unaligned.
        CS_FORCEINLINE void VectorStore(double* data, const Vector4d a)
        {
            data[0] = a.x;
            data[1] = a.w;
        }

        /// Returns the upper double-precision floating-point value of a.
        CS_FORCEINLINE double VectorGetW(const Vector4d a)
        {
            return a.w;
        }

        /// Returns the lower double-precision floating-point value of a.
        CS_FORCEINLINE double VectorGetX(const Vector4d a)
        {
            return a.x;
        }

        /// Adds the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorAdd(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w + b.w, a.x + b.x);
        }

        /// Subtracts the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorSub(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w - b.w, a.x - b.x);
        }

        /// Multiplies the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMul(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w * b.w, a.x * b.x);
        }

        /// Divides the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorDiv(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w / b.w, a.x / b.x);
        }

        /// Returns the square roots of the two double-precision floating point values of a.
        CS_FORCEINLINE Vector4d VectorSqrt(const Vector4d a)
        {
            return ReturnVector4d(
                sqrt(a.w), sqrt(a.x));
        }

        /// Returns the minima of the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMin(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w < b.w ? a.w : b.w,
                a.x < b.x ? a.x : b.x);
        }

        /// Returns the maxima of the two double-precision floating point values of a and b.
        CS_FORCEINLINE Vector4d VectorMax(const Vector4d a, const Vector4d b)
        {
            return ReturnVector4d(
                a.w > b.w ? a.w : b.w,
                a.x > b.x ? a.x : b.x);
        }
    }
}

#endif // __SSE2_CPP_SIMD_TYPES_H__
