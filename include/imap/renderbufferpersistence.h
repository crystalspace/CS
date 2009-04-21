/*
  Copyright (C) 2007 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_IMAP_RENDERBUFFERPERSISTENCE_H__
#define __CS_IMAP_RENDERBUFFERPERSISTENCE_H__

/**\file
 */

/**
 * Render buffer persistence information.
 * If a render buffer exhibits this interface it can return information on the
 * preferred way a render buffer should be made persistent (ie stored on 
 * disk).
 */
struct iRenderBufferPersistence : public virtual iBase
{
  SCF_INTERFACE (iRenderBufferPersistence, 0, 0, 1);
  
  /**
   * Query the filename into which the render buffer should preferably be
   * saved.
   */
  virtual const char* GetFileName () = 0;
};

#endif // __CS_IMAP_RENDERBUFFERPERSISTENCE_H__
