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

#ifndef __PROCESSORSPECDETECTION_GCC_PPC_H__
#define __PROCESSORSPECDETECTION_GCC_PPC_H__

#include <sys/sysctl.h>

namespace Implementation
{
    class DetectInstructionsGCCPPC
    {
    public:
        uint CheckSupportedInstruction()
        {
	    uint instructionBitMask = 0U;

            if(GetAltiVecTypeAvailable() > 0)
                instructionBitMask |= ALTIVEC;

            return instructionBitMask;
        }

    private:
        // Returns 0 for scalar only, >0 for AltiVec
        int GetAltiVecTypeAvailable()
        {

            int sels[2] = { CTL_HW, HW_VECTORUNIT };
            int vType = 0; // 0 == scalar only
            size_t length = sizeof(vType);
            int error = sysctl(sels, 2, &vType, &length, NULL, 0);

            if( 0 == error ) return vType;

            return 0;
        }

    };
}

#endif // __PROCESSORSPECDETECTION_GCC_PPC_H__
