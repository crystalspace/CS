/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __IMESH_CROSSBLD_H__
#define __IMESH_CROSSBLD_H__

#include "csutil/scf.h"

struct iModelDataObject;
struct iThingState;
struct iSprite3DFactoryState;
struct iMaterialWrapper;


SCF_VERSION (iCrossBuilder, 0, 1, 0);

/**
 * The crossbuilder can be used to build things and sprite factories from
 * single objects of imported model files (iModelDataObject).
 */
struct iCrossBuilder : public iBase
{
  /**
   * Build a thing from a model file. The model data must have its materials
   * stored as material wrappers, otherwise the default material will be used.
   */
  virtual bool BuildThing (iModelDataObject *Data, iThingState *tgt,
	iMaterialWrapper *DefaultMaterial = NULL) const = 0;

  /// Build a sprite factory from a model file
  virtual bool BuildSpriteFactory (iModelDataObject *Data,
	iSprite3DFactoryState *tgt) const = 0;
};

#endif // __IMESH_CROSSBLD_H__
