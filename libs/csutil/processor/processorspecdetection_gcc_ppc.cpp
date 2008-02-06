/*
Copyright (C) 2008 by Michael Gist

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

#if defined(CS_PROCESSOR_POWERPC)

#include "csutil/processorspecdetection.h"

#ifdef CS_PLATFORM_MACOSX
#include <sys/sysctl.h>
#else

#include <signal.h>
static int altivec = 1;

static void handle_sigill(int sig)
{
  altivec = 0;
}

#endif

namespace CS
{
  namespace Platform
  {
    namespace Implementation
    {
      uint DetectInstructionsGCCPPC::CheckSupportedInstruction()
      {
	  uint instructionBitMask = 0U;

        if(GetAltiVecTypeAvailable() > 0)
          instructionBitMask |= ALTIVEC;

        return instructionBitMask;
      }

      int DetectInstructionsGCCPPC::GetAltiVecTypeAvailable()
      {
        int hasAltiVec = 0;
            
        // Do it the nice way if we use OSX.
#ifdef CS_PLATFORM_MACOSX
        int sels[2] = { CTL_HW, HW_VECTORUNIT };
        int altivec = 0;
        size_t length = sizeof(altivec);
        int error = sysctl(sels, 2, &altivec, &length, NULL, 0);

        if(!error)
        hasAltiVec = altivec;
#else
        // We try it this way instead :-)
        signal(SIGILL, handle_sigill);
        asm volatile("dssall");
        signal(SIGILL, SIG_DFL);
        hasAltiVec = altivec;
#endif
        return hasAltiVec;
      }
    } // namespace Implementation
  } // namespace Platform
} // namespace CS

#endif // defined(CS_PROCESSOR_POWERPC)
