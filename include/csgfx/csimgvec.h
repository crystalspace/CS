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

#ifndef __CS_CSGRAPHIC_IMGLIST_H__
#define __CS_CSGRAPHIC_IMGLIST_H__

#include "csextern.h"

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"

#include "igraphic/image.h"
#include "igraphic/imgvec.h"

struct CS_CRYSTALSPACE_EXPORT csImageVector :
  public scfImplementation1<csImageVector, iImageVector>
{
private:
  csRefArray<iImage> image;
public:
  CS_LEAKGUARD_DECLARE (csImageVector);

  csImageVector();
  virtual ~csImageVector();

  /**
  * Add an Image to the Vector
  */
  virtual void AddImage (iImage* img);
  /**
  * Insert an Image into the Vector at specified index
  */
  virtual void InsertImage (iImage* img, size_t index);
  /**
  * Add an Image to the End of the Vector
  */
  virtual void operator += (iImage* img);
  /**
  * Get Image at specified index
  */
  virtual csRef<iImage> GetImage(size_t index);
  /**
  * Get Image operator[]
  */
  virtual void SetImage (size_t index, iImage* img);
  /**
  * Get Image Count
  */
  virtual size_t Length();
  /**
   * Remove a specific index
   */
  virtual void DeleteIndex(size_t index);
  /**
  * Remove All Images
  */
  virtual void RemoveAll();

};

#endif // __CS_IGRAPHIC_IMGLIST_H__
