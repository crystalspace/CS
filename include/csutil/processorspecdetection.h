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

/*
 * 
 */
#define MMX 0
#define SSE 1
#define SSE2 2
#define SSE3 3
#define ALTIVEC 4

namespace CS
{
    namespace Platform
    {
        class CS_CRYSTALSPACE_EXPORT ProcessorSpecDetection
        {
        public:

            ProcessorSpecDetection()
            {
            }

            ~ProcessorSpecDetection()
            {
            }

            inline bool HasMMX()
            {
#if defined(CS_PLATFORM_WIN32) && (CS_PROCESSOR_SIZE == 64)
                    return true;
#endif
                checkSupport(MMX);
                return supportsMMX;
            }

            inline bool HasSSE()
            {
#if defined(CS_PLATFORM_WIN32) && (CS_PROCESSOR_SIZE == 64)
                    return true;
#endif
                checkSupport(SSE);
                return supportsSSE;
            }

            inline bool HasSSE2()
            {
#if defined(CS_PLATFORM_WIN32) && (CS_PROCESSOR_SIZE == 64)
                    return true;
#endif
                checkSupport(SSE2);
                return supportsSSE2;
            }

            inline bool HasSSE3()
            {
                checkSupport(SSE3);
                return supportsSSE3;
            }

            inline bool HasAltiVec()
            {
#ifndef CS_PLATFORM_POWERPC
                    return false;
#endif
                checkSupport(ALTIVEC);
                return supportsAltiVec;
            }

        private:

            bool supportsMMX;
            bool supportsSSE;
            bool supportsSSE2;
            bool supportsSSE3;
            bool supportsAltiVec;
            bool checked[5];

            void checkSupport(int iSet)
            {
                if(checked[iSet])
                    return;

                bool result = CheckSupportedInstruction(iSet);

                switch(iSet)
                {
                case 0:
                    {
                        supportsMMX = result;
                    }
                case 1:
                    {
                        supportsSSE = result;
                    }
                case 2:
                    {
                        supportsSSE2 = result;
                    }
                case 3:
                    {
                        supportsSSE3 = result;
                    }
                case 4:
                    {
                        supportsAltiVec = result;
                    }
                }
                checked[iSet] = true;
            }
        };
    }
}
