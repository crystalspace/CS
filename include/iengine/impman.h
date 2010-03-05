/*
  Copyright (C) 2009 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_IENGINE_IMPMAN_H__
#define __CS_IENGINE_IMPMAN_H__

/**\file
 * Imposter Manager interfact
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

struct iImposterMesh;
struct iRenderView;

struct iImposterManager : public virtual iBase
{
  SCF_INTERFACE(iImposterManager, 1, 0, 0);

  virtual void Register(iImposterMesh* mesh) = 0;

  virtual bool Update(iImposterMesh* mesh) = 0;

  virtual void Unregister(iImposterMesh* mesh) = 0;
};

/** @} */

#endif // __CS_IENGINE_IMPMAN_H__
