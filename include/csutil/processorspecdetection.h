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

#ifndef __PROCESSORSPECDETECTION_H__
#define __PROCESSORSPECDETECTION_H__

/**\file
 * Processor feature detection
 */

#include "csextern.h"
#include "cstypes.h"

namespace CS
{
    namespace Platform
    {
        enum InstructionSetFlags
        { 
            ALTIVEC = 1 << 0,
            MMX = 1 << 1,
            SSE = 1 << 2, 
            SSE2 = 1 << 3,
            SSE3 = 1 << 4
        };

#ifdef CS_PLATFORM_WIN32
#include "csutil/processor/processorspecdetection_win.h"
#elif defined(CS_PROCESSOR_POWERPC)
#include "csutil/processor/processorspecdetection_gcc_ppc.h"
#else
#include "csutil/processor/processorspecdetection_nonwin_gcc_x86.h"
#endif

        using namespace Implementation;
        template <class T>
        class ProcessorSpecDetectionBase
        {
        public:

            ProcessorSpecDetectionBase()
            {

                checked = false;
                hasAltiVec = false;
                hasMMX = false;
                hasSSE = false;
                hasSSE2 = false;
                hasSSE3 = false;
            }

            ~ProcessorSpecDetectionBase()
            {
            }

            inline bool HasMMX()
            {
                CheckSupport();
                return hasMMX;
            }

            inline bool HasSSE()
            {
                CheckSupport();
                return hasSSE;
            }

            inline bool HasSSE2()
            {
                CheckSupport();
                return hasSSE2;
            }

            inline bool HasSSE3()
            {
                CheckSupport();
                return hasSSE3;
            }

            inline bool HasAltiVec()
            {
                CheckSupport();
                return hasAltiVec;
            }

        private:

            T platform;
            uint instructionBitMask;
            bool checked;
            bool hasAltiVec;
            bool hasMMX;
            bool hasSSE;
            bool hasSSE2;
            bool hasSSE3;

            void CheckSupport()
            {
                if(checked)
                    return;

                instructionBitMask = platform.CheckSupportedInstruction();
                hasAltiVec = (instructionBitMask & ALTIVEC) != 0;
                hasMMX = (instructionBitMask & MMX) != 0;
                hasSSE = (instructionBitMask & SSE) != 0;
                hasSSE2 = (instructionBitMask & SSE2) != 0;
                hasSSE3 = (instructionBitMask & SSE3) != 0;
                checked = true;
            }
        };

        /**
         * Class performing processor specifications and feature detection.
         */
        class ProcessorSpecDetection
        {
        private:

#ifdef CS_PLATFORM_WIN32
            ProcessorSpecDetectionBase<DetectInstructionsWin> procDetect;
#elif defined(CS_PROCESSOR_POWERPC)
            ProcessorSpecDetectionBase<DetectInstructionsGCCPPC> procDetect;
#else
            ProcessorSpecDetectionBase<DetectInstructionsNonWinGCCx86> procDetect;
#endif

        public:
            /// Returns whether MMX instructions are available.
            inline bool HasMMX()
            {
                return procDetect.HasMMX();
            }

            /// Returns whether SSE instructions are available.
            inline bool HasSSE()
            {
                return procDetect.HasSSE();
            }

            /// Returns whether SSE2 instructions are available.
            inline bool HasSSE2()
            {
                return procDetect.HasSSE2();
            }

            /// Returns whether SSE3 instructions are available.
            inline bool HasSSE3()
            {
                return procDetect.HasSSE3();
            }

            /// Returns whether Altivec instructions are available.
            inline bool HasAltiVec()
            {
                return procDetect.HasAltiVec();
            }

        };
    }
}

#endif // __PROCESSORSPECDETECTION_H__
