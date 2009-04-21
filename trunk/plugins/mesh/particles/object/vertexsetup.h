/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_MESH_VERTEXSETUP_H__
#define __CS_MESH_VERTEXSETUP_H__

#include "imesh/particles.h"

/**\file
 * Helper functors for vertex buffer setup
 */

class csVector3;

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  struct iVertexSetup
  {
    virtual ~iVertexSetup() {}
    virtual void Init (const csReversibleTransform& o2c, const csVector3& commonDir,
      const csVector2& particleSize) = 0;

    virtual void SetupVertices (const csParticleBuffer particleBuffer,
      csVector3* vertexBuffer) = 0;
  };

  // Function to get a pointer to a vertex setup
  iVertexSetup* GetVertexSetupFunc (csParticleRotationMode rotMode, 
    csParticleRenderOrientation orient, bool individualSize);

}
CS_PLUGIN_NAMESPACE_END(Particles)

#endif
