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
                // Init everything to false.
                for(int i=0; i<INSTRUCTIONCOUNT; i++)
                {
                    instruction[i] = false;
                    checked[i] = false;
                }
            }

            ~ProcessorSpecDetectionBase()
            {
            }

            inline bool HasMMX()
            {
                CheckSupport(MMX);
                return instruction[MMX];
            }

            inline bool HasSSE()
            {
                CheckSupport(SSE);
                return instruction[SSE];
            }

            inline bool HasSSE2()
            {
                CheckSupport(SSE2);
                return instruction[SSE2];
            }

            inline bool HasSSE3()
            {
                CheckSupport(SSE3);
                return instruction[SSE3];
            }

            inline bool HasAltiVec()
            {
                CheckSupport(ALTIVEC);
                return instruction[ALTIVEC];
            }

        private:

            enum InstructionList { MMX, SSE, SSE2, SSE3, ALTIVEC, INSTRUCTIONCOUNT };
            bool instruction[INSTRUCTIONCOUNT];
            bool checked[INSTRUCTIONCOUNT];

            void CheckSupport(int iSet)
            {
                if(checked[iSet])
                    return;

                instruction[iSet] = T::CheckSupportedInstruction(iSet);
                checked[iSet] = true;
            }
        };

        class CS_CRYSTALSPACE_EXPORT ProcessorSpecDetection
        {
        private:

#ifdef CS_PLATFORM_WIN32
            ProcessorSpecDetectionBase<DetectInstructionsWin> procDetect;
            // Define static bool plat64bit.
            bool DetectInstructionsWin::plat64bit = false;
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
