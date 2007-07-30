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

/*
 * Our 'generic' function. In this case, it can be either C++ or SSE or Altivec.
 * All three versions are compiled, and then the correct one is 
 * chosen at runtime. If the instruction set isn't supported by the compiler,
 * then the C++ abstraction is used as a fallback. It's the users responsibility
 * to check for this in their CPP file, if they don't want multiple versions of 
 * the C++ code.
 */

using namespace CS::SIMD;

CS_FORCEINLINE bool SIMDFunc(float* a, float* b, float* c, int size)
{
    Vector4* ap = (Vector4*) a;
    Vector4* bp = (Vector4*) b;
    Vector4* cp = (Vector4*) c;

    Vector4 simd;
    Vector4 simd2;
    Vector4 simd3;

        for(int j=0; j<size; j+=4)
        {
            simd = VectorMul(*ap, *ap);
            simd2 = VectorMul(*bp, *bp);
            simd3 = VectorAdd(simd, simd2);
            *cp = VectorSqrt(simd3);
            ap++;
            bp++;
            cp++;
        }

    return true;
}
