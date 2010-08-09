/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
  Sensing Laboratory of the School of Engineering at the 
  Universite catholique de Louvain, Belgium
  http://www.tele.ucl.ac.be

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
#include "iutil/comp.h"
#include "csutil/leakguard.h"
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
  virtual void DebugTransform (csReversibleTransform& transform);
  virtual void Display (iView* view, float axisSize = 1.0f);

 private:
  iObjectRegistry* object_reg;
  csArray<csReversibleTransform> transforms;
};

}
CS_PLUGIN_NAMESPACE_END(VisualDebug)

#endif //__CS_VISUALDEBUG_H__
