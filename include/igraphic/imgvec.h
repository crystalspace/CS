/*
    Copyright (C) 2003 by Philipp Aumayr

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


#ifndef __CS_IGRAPHIC_IMGLIST_H__
#define __CS_IGRAPHIC_IMGLIST_H__

/**\file
 * Image List 
 */

#include "csutil/scf.h"

struct iImage;

/**
 * Image List 
 */
struct iImageVector : public virtual iBase
{
  SCF_INTERFACE (iImageVector, 2, 0, 0);

  /**
   * Add an Image to the Vector
   */
  virtual void AddImage(iImage* img) = 0;
  /**
   * Insert an Image into the Vector at specified index
   */
  virtual void InsertImage(iImage* img, size_t index) = 0;
  /**
  * Add an Image to the End of the Vector
  */
  virtual void operator += (iImage* img) = 0;
  /**
  * Get Image at specified index
  */
  virtual csRef<iImage> GetImage(size_t index) = 0;
  /**
  * Set Image at specified index
  */
  virtual void SetImage (size_t index, iImage* img) = 0;
  /**
  * Get Image Count
  */
  virtual size_t Length() = 0;
  /**
   * Remove a specific index
   */
  virtual void DeleteIndex(size_t index) = 0;
  /**
  * Remove All Images
  */
  virtual void RemoveAll() = 0;

};

#endif // __CS_IGRAPHIC_IMGLIST_H__
