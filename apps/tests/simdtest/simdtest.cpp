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

#include "cssysdef.h"
#include "csutil/processorspecdetection.h"
#include "csutil/sysfunc.h"
#include "simdtest.h"

using namespace CS::Platform;
using namespace CS::SIMD;

CS_IMPLEMENT_APPLICATION

bool SIMDTest::testCPP(float* a, float* b, float* c, int size)
{

    float holder = 0;
    float holder2 = 0;
    float holder3 = 0;

        for(int j=0; j<size; j++)
        {
            holder = (*a) * (*a);
            holder2 = (*b) * (*b);
            holder3 = holder + holder2;
            *c = sqrt(holder3);
            a++;
            b++;
            c++;
        }
    return true;
}

int main(int argc, char* argv[])
{
    ProcessorSpecDetection detect;

    if(detect.HasMMX())
    {
        printf("MMX is supported! :-)\n");
    }
    else
    {
        printf("MMX is not supported! :-( \n");
    }
    if(detect.HasSSE())
    {
        printf("SSE is supported! :-)\n");
    }
    else
    {
        printf("SSE is not supported! :-(\n");
    }
    if(detect.HasSSE2())
    {
        printf("SSE2 is supported! :-)\n");
    }
    else
    {
        printf("SSE2 is not supported! :-(\n");
    }
    if(detect.HasSSE3())
    {
        printf("SSE3 is supported! :-)\n");
    }
    else
    {
        printf("SSE3 is not supported! :-(\n");
    }
    if(detect.HasAltiVec())
    {
        printf("AltiVec is supported! :-)\n");
    }
    else
    {
        printf("AltiVec is not supported! :-(\n");
    }


    const int size = 30000;
#ifdef CS_COMPILER_MSVC
    __declspec(align(16)) float a[size];
    __declspec(align(16)) float b[size];
    __declspec(align(16)) float c[size];
#else
    float a[size] __attribute__((aligned(16)));
    float b[size] __attribute__((aligned(16)));
    float c[size] __attribute__((aligned(16)));
#endif

    for(int i=0; i<size; i++)
    {
        a[i] = 2.0f;
        b[i] = 2.0f;
        c[i] = 0;
    }

    printf("Running SIMD test 1.\n");
    csTicks start = csGetMicroTicks();
    if(SIMDCheck<bool, SSEType, float*, float*, float*, int>((*SIMDTest::testSSE), (*SIMDTest::testCPP), a, b, c, size))
    {
        printf("Time taken: %lldus \n", csGetMicroTicks()-start);
        float output = 0.0f;
        for(int i=0; i<size; i++)
        {
            output += c[i];
        }
        printf("Output is %f, expected output is 84852.812290\n", output);
    }
    return 0;
}
