/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_FOLIAGEMESH_H__
#define __CS_IMESH_FOLIAGEMESH_H__

#include "csutil/scf.h"

class csVector3;
class csVector2;
class csColor;

struct csTriangle;
struct iMaterialWrapper;

SCF_VERSION (iFoliageFactoryState, 0, 0, 1);

/**
 * The foliage mesh can be used to make foliage (plants, boulders,
 * ...) that fits nicely with a terrain.
 * The general API for the foliage factory. Here you define the
 * actual geometry which is shared between all foliage mesh instances.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Foliage mesh object plugin (crystalspace.mesh.object.foliage)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Foliage Factory Loader plugin
        (crystalspace.mesh.loader.factory.foliage)
 *   </ul>
 */
struct iFoliageFactoryState : public iBase
{
};

SCF_VERSION (iFoliageMeshState, 0, 0, 1);

/**
 * This interface describes the API for the foliage mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Foliage mesh object plugin (crystalspace.mesh.object.foliage)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Foliage Loader plugin (crystalspace.mesh.loader.foliage)
 *   </ul>
 */
struct iFoliageMeshState : public iBase
{
};

#endif // __CS_IMESH_FOLIAGEMESH_H__

