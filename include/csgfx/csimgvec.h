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

/**\file
 * Implementation of iImageVector.
 */

/**
 * Implementation of iImageVector.
 */
struct CS_CRYSTALSPACE_EXPORT csImageVector :
  public scfImplementation1<csImageVector, iImageVector>
{
private:
  csRefArray<iImage> image;
public:
  CS_LEAKGUARD_DECLARE (csImageVector);

  csImageVector();
  virtual ~csImageVector();

  virtual void AddImage (iImage* img);
  virtual void InsertImage (iImage* img, size_t index);
  virtual void operator += (iImage* img);
  virtual csRef<iImage> GetImage(size_t index);
  virtual void SetImage (size_t index, iImage* img);
  virtual size_t Length();
  virtual void DeleteIndex(size_t index);
  virtual void RemoveAll();
};

#endif // __CS_IGRAPHIC_IMGLIST_H__
