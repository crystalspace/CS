/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
  and Communication Technologies, Electronics and Applied Mathematics
  at Universite catholique de Louvain, Belgium
  http://www.uclouvain.be/en-icteam.html

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
#ifndef __CS_VISUALDEBUG_H__
#define __CS_VISUALDEBUG_H__

#include "csutil/scf_implementation.h"
#include "csutil/leakguard.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "iutil/comp.h"
#include "iutil/visualdebug.h"

CS_PLUGIN_NAMESPACE_BEGIN(VisualDebug)
{

class VisualDebugger : public scfImplementation2<VisualDebugger,
    CS::Debug::iVisualDebugger, iComponent>
{
 public:
  CS_LEAKGUARD_DECLARE(VisualDebugger);

  VisualDebugger (iBase* parent);

  //-- iComponent
  virtual bool Initialize (iObjectRegistry*);

  //-- CS::Debug::iVisualDebugger
  virtual void DebugTransform (const csReversibleTransform& transform,
			       bool persist,
			       float size);
  virtual void DebugPosition (const csVector3& position,
			      bool persist,
			      csColor color,
			      size_t size);
  virtual void DebugVector (const csReversibleTransform& transform,
			    const csVector3& vector,
			    bool persist,
			    csColor color);
  virtual void Display (iView* view);

 private:
  iObjectRegistry* object_reg;

  struct TransformData
  {
    csReversibleTransform transform;
    bool persist;
    float size;
  };
  csArray<TransformData> transforms;

  struct PositionData
  {
    csVector3 position;
    bool persist;
    csColor color;
    size_t size;
  };
  csArray<PositionData> positions;

  struct VectorData
  {
    csReversibleTransform transform;
    csVector3 vector;
    bool persist;
    csColor color;
  };
  csArray<VectorData> vectors;
};

}
CS_PLUGIN_NAMESPACE_END(VisualDebug)

#endif //__CS_VISUALDEBUG_H__
