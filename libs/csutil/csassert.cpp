/*
  Copyright (C) 2005 by Jorrit Tyberghein
            (C) 2005 by Frank Richter

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
#include "csutil/ansicolor.h"
#include "csutil/callstack.h"
#include "csutil/sysfunc.h"

namespace CrystalSpace
{
  namespace Debug
  {
    
    void AssertMessage (const char* expr, const char* filename, int line,
			 const char* msg)
    {
      static int assertCnt = 0;
      
      if (assertCnt == 1)
      {
	fprintf (stderr, "Whoops, assertion while reporting assertion...\n");
      }
      else if (assertCnt > 1)
      {
	fprintf (stderr, "What the... assertion while reporting assertion "
	  "while reporting assertion\n");
	fprintf (stderr, "Screw it!\n");
	CS_DEBUG_BREAK;
	return;
      }
      
      assertCnt++;
      
      csFPrintf (stderr, 
	"Assertion failed: %s\n", expr);
      csFPrintf (stderr, 
	"Location:         %s:%d\n", filename, line);
      if (msg) csFPrintf (stderr, 
	"Message:          %s\n", msg);
      fflush (stderr);
      
      csCallStack* stack = csCallStackHelper::CreateCallStack (1);
      if (stack != 0)
      {
	csFPrintf (stderr, "Call stack:\n");
	stack->Print (stderr);
	fflush (stderr);
	stack->Free();
      }

      assertCnt--;
      
      const char* ignoreEnv = getenv ("CS_ASSERT_IGNORE");
      if (!ignoreEnv || (atoi (ignoreEnv) == 0))
      {
	CS_DEBUG_BREAK;
      }
    }
    
  } // namespace Debug
} // namespace CrystalSpace
