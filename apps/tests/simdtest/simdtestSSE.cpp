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

#include "csutil/processor/simdtypes.h"
#include "simdtest.h"

using namespace CS::SIMD;

bool SIMDTest::testSSE(float* a, float* b, float* c, int size)
{

    SIMDVector4* ap = (SIMDVector4*) a;
    SIMDVector4* bp = (SIMDVector4*) b;
    SIMDVector4* cp = (SIMDVector4*) c;

    SIMDVector4 simd;
    SIMDVector4 simd2;
    SIMDVector4 simd3;

        for(int j=0; j<size; j+=4)
        {
            simd = CS::SIMD::csMulSIMD(*ap, *ap);
            simd2 = CS::SIMD::csMulSIMD(*bp, *bp);
            simd3 = CS::SIMD::csAddSIMD(simd, simd2);
            *cp = CS::SIMD::csSqrtSIMD(simd3);
            ap++;
            bp++;
            cp++;
        }
    return true;
}
