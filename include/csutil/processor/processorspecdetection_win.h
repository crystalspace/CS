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

#ifndef __PROCESSORSPECDETECTION_WIN_H__
#define __PROCESSORSPECDETECTION_WIN_H__

// Not defined in some SDKs.
#ifndef PF_SSE3_INSTRUCTIONS_AVAILABLE
#define PF_SSE3_INSTRUCTIONS_AVAILABLE 13
#endif

namespace Implementation
{
    class DetectInstructionsWin
    {
    public:

        uint CheckSupportedInstruction()
        {
            uint instructionBitMask = 0U;
            // We know 64-bit processors on windows will support MMX, SSE and SSE2 instructions.
            bool plat64bit = false;
#if defined(CS_PLATFORM_WIN32) && (CS_PROCESSOR_SIZE == 64)
            plat64bit = true;
#endif

            // Check for instruction sets and set flags.
            if(plat64bit || IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) != 0)
                instructionBitMask |= MMX;
            if(plat64bit || IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0)
                instructionBitMask |= SSE;
            if(plat64bit || IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0)
                instructionBitMask |= SSE2;
            if(IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != 0)
                instructionBitMask |= SSE3;

            return instructionBitMask;
        }
    };
}

#endif // __PROCESSORSPECDETECTION_WIN_H__
