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

#include "bfdsymbols.h"

namespace CrystalSpace
{
  namespace Debug
  {
    
    BfdSymbols::BfdSymbols (const char* filename, uintptr_t addrOffs) : 
      syms(0), addrOffs(addrOffs)
    {
      this->filename = filename;
      abfd = bfd_openr (filename, 0);
      if (!CheckValid()
	|| !GrabSymbols())
      {
	bfd_close (abfd);
	abfd = 0;
      }
    }
    
    BfdSymbols::~BfdSymbols ()
    {
      if (abfd != 0) bfd_close (abfd);
      if (syms != 0) free (syms);
    }
    
    bool BfdSymbols::CheckValid()
    {
      return (abfd
	&& bfd_check_format (abfd, bfd_object)
	&& (bfd_get_file_flags (abfd) & HAS_SYMS));
    }

    bool BfdSymbols::GrabSymbols()
    {
      int symsize = bfd_get_symtab_upper_bound (abfd);
      if (symsize < 0) return false;
      syms = (asymbol**)malloc (symsize);
      
      int numSyms = bfd_canonicalize_symtab (abfd, syms);
      return numSyms > 0;
    }
    
    bool BfdSymbols::FindSymbol (uintptr_t addr, const char*& filename, 
				  const char*& function, uint& line)
    {
      addr += addrOffs;
      
      asection* sect;
      for (sect = abfd->sections; sect != 0; sect = sect->next)
      {
	if (!(bfd_get_section_flags (abfd, sect) & SEC_ALLOC)) continue;
	bfd_vma vma = bfd_get_section_vma (abfd, sect);
	if (addr < vma) continue;
	bfd_size_type size = bfd_section_size (abfd, sect);
	if (addr > vma + size) continue;
	  
	return (bfd_find_nearest_line (abfd, sect, syms, addr - vma, 
	  &filename, &function, &line));
      }
      return false;
    }
  } // namespace Debug
} // namespace CrystalSpace
