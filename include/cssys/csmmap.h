/*
    Copyright (C) 2002 by Jorrit Tyberghein
    	      (C) 2002 Frank Richter	

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

#ifndef __CS_CSSYS_CSMMAP_H__
#define __CS_CSSYS_CSMMAP_H__

/**\file
 * Memory mapping interface.
 * BE AWARE that the functions here are very platform-dependent, they
 * even might not be available at all. 
 * For platform-independence don't use the routines here, 
 * use the csMemoryMappedIO class.
 */

#ifdef CS_HAS_MEMORY_MAPPED_IO
/**
 * Map a file to a memory area. 
 * Fills in the mmioInfo struct by mapping in \c filename.
 * \c filename is a platform-dependent path.
 * Returns true on success, false otherwise.
 */
extern bool MemoryMapFile(mmioInfo* info, char const* filename);
/// Unmap a file from a memory area.
extern void UnMemoryMapFile(mmioInfo* info);
#endif

#endif // __CS_CSSYS_CSMMAP_H__
