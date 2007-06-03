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

#include "csextern.h"

/*
*  Include the correct version of CheckSupportedInstruction().
*/
#ifdef CS_PLATFORM_WIN32
#include "processorspecdetection_win.h"
#elif defined(CS_PLATFORM_POWERPC)
#include "processorspecdetection_gcc_ppc.h"
#else
#include "processorspecdetection_nonwin_gcc_x86.h"
#endif

namespace CS
{
    namespace Platform
    {
        using namespace Implementation;
        template <class T>
        class CS_CRYSTALSPACE_EXPORT ProcessorSpecDetectionBase
        {
        public:

            ProcessorSpecDetectionBase()
            {
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
                checked = true;
                hasAltiVec = (instructionBitMask == 1);
                hasMMX = (instructionBitMask > 1);
                hasSSE = (instructionBitMask > 2);
                hasSSE2 = (instructionBitMask > 3);
                hasSSE3 = (instructionBitMask > 4);
            }
        };

        class CS_CRYSTALSPACE_EXPORT ProcessorSpecDetection
        {
        private:

#ifdef CS_PLATFORM_WIN32
            ProcessorSpecDetectionBase<DetectInstructionsWin> procDetect;
#elif defined(CS_PLATFORM_POWERPC)
            ProcessorSpecDetectionBase<DetectInstructionsGCCPPC> procDetect;
#else
            ProcessorSpecDetectionBase<DetectInstructionsNonWinGCCx86> procDetect;
#endif

        public:
            inline bool HasMMX()
            {
                return procDetect.HasMMX();
            }

            inline bool HasSSE()
            {
                return procDetect.HasSSE();
            }

            inline bool HasSSE2()
            {
                return procDetect.HasSSE2();
            }

            inline bool HasSSE3()
            {
                return procDetect.HasSSE3();
            }

            inline bool HasAltiVec()
            {
                return procDetect.HasAltiVec();
            }
        };
    }
}
