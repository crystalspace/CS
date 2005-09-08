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

#ifndef __CS_LIBS_CSUTIL_BFDSYMBOLS_H__
#define __CS_LIBS_CSUTIL_BFDSYMBOLS_H__

#include "csutil/csstring.h"

#include <bfd.h>

namespace CrystalSpace
{
  namespace Debug
  {
    
    /// Class to obtain symbols from an object file via libbfd.
    class BfdSymbols
    {
      bfd* abfd;
      asymbol** syms;
      csString filename;
      uintptr_t addrOffs;
      
      /// Check if everything is sane
      bool CheckValid();
      /// Read canonicalized bfd symbol
      bool GrabSymbols();
    public:
      /**
       * Initialize. Try to obtain symbols from \a filename. \a addrOffs is 
       * is an offset added to the sections in the object to adjust for e.g.
       * relocation.
       */
      BfdSymbols (const char* filename, uintptr_t addrOffs = 0);
      ~BfdSymbols ();
      /**
       * Returns whether the file could be read and contains symbols.
       * No further operations(save deletion) should be done on the instance
       * if this returns false.
       */
      bool Ok() { return abfd != 0; }
      
      /// Find function, file name and line for an instruction address
      bool FindSymbol (uintptr_t addr, const char*& filename, 
	const char*& function, uint& line);
      /// Get file name of object file
      const char* GetFileName () { return filename; }
    };
    
  } // namespace Debug
} // namespace CrystalSpace

#endif // __CS_LIBS_CSUTIL_BFDSYMBOLS_H__
