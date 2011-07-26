/*
    Copyright (C) 2011 by Jelle Hellemans

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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "csutil/event.h"
#include "csutil/eventnames.h"

#include <iengine/scenenode.h>
#include <iengine/mesh.h>
#include <iengine/movable.h>
#include <ivideo/graph2d.h>
#include <iengine/camera.h>
#include <csgeom/math3d.h>
#include <iutil/object.h>

#include "ieditor/context.h"
#include "ieditor/action.h"
#include "ieditor/actionmanager.h"

#include "moveoperator.h"



CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
bool SoftTransform(iObject* obj, csReversibleTransform transform)
{
  csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (obj);
  if (!node) return false;
  iMeshWrapper* mesh = node->QueryMesh ();
  if (!mesh) return false;
  iMovable* mov = mesh->GetMovable ();
  if (!mov) return false;
  mov->Transform(transform.GetT2O());
  mov->MovePosition(transform.GetOrigin());
  mov->UpdateMove();
  return true; 
}

MoveOperator::MoveOperator (iObjectRegistry* obj_reg)
 : scfImplementationType (this), object_reg(obj_reg)
{
}

MoveOperator::~MoveOperator ()
{
}

bool MoveOperator::Poll (iContext* ctx)
{
  if (ctx && ctx->GetActiveObject() && ctx->GetCamera())
  {
    csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (ctx->GetActiveObject());
    if (node)
    {
      iMeshWrapper* mesh = node->QueryMesh ();
      if (mesh)
      {
        iMovable* mov = mesh->GetMovable ();
        if (mov)
        {
          return true;
        }
      }
    }
  }
  return false;
}

OperatorState MoveOperator::Execute (iContext* ctx)
{
  csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (ctx->GetActiveObject());
  iMeshWrapper* mesh = node->QueryMesh ();
  worldStart = mesh->GetWorldBoundingBox ().GetCenter ();
  return OperatorRunningModal;
}

OperatorState MoveOperator::Invoke (iContext* ctx, iEvent*)
{
  return Execute(ctx);
}

OperatorState MoveOperator::Commit (iContext* ctx, iMeshWrapper* mesh)
{
  csVector3 worldEnd = mesh->GetWorldBoundingBox ().GetCenter ();
  csReversibleTransform transform = GetOperationTransform(worldEnd-worldStart);
  csRef<iActionManager> actionManager = csQueryRegistry<iActionManager> (object_reg);
  csRef<iAction> action;
  action.AttachNew(new MoveObjectAction(object_reg, ctx->GetActiveObject(), transform));
  actionManager->Do(action);
  return OperatorFinished;
}

OperatorState MoveOperator::Modal (iContext* ctx, iEvent* ev)
{
  csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (ctx->GetActiveObject());
  iMeshWrapper* mesh = node->QueryMesh ();
  iMovable* mov = mesh->GetMovable ();

    
  if (CS_IS_KEYBOARD_EVENT(object_reg, *ev)) 
  {
    csKeyEventType eventtype = csKeyEventHelper::GetEventType(ev);
    if (eventtype == csKeyEventTypeDown)
    {
      utf32_char code = csKeyEventHelper::GetCookedCode(ev);
      if (code == CSKEY_ESC)
      {
        return OperatorCanceled;
      }
      else if (code == CSKEY_ENTER)
      {
        return Commit(ctx, mesh);
      }
    }
  }
  else if (CS_IS_MOUSE_EVENT(object_reg, *ev))
  {
    int mouse_x = csMouseEventHelper::GetX(ev);
    int mouse_y = csMouseEventHelper::GetY(ev);
    
    
    csVector3 dragCenter = mesh->GetWorldBoundingBox ().GetCenter ();
    
    // Get constraint axis in world space
    csVector3 constraintAxis = csVector3(1,0,0);  //TODO
    
    csVector3 constraint = mov->GetTransform().This2Other(constraintAxis);
    
    csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (object_reg);
    int height = g3d->GetDriver2D()->GetHeight();

    csVector2 p (mouse_x, (ctx->GetCamera()->GetShiftY()*height) * 2 - mouse_y);
    csVector3 vc = ctx->GetCamera()->InvPerspective (p, 1);
    csVector3 vw = ctx->GetCamera()->GetTransform ().This2Other (vc);
    csVector3 vo = ctx->GetCamera()->GetTransform ().GetO2TTranslation ();

    // Find picking ray (from origin)
    csVector3 pickingRay = vo + (vw - vo) * (int) 100000;

    // Find normal of constraint and picking ray to form a plane containing these
    csVector3 constraintN = pickingRay % constraint;

    // Find normal of the constraint plane
    csVector3 constraintPlaneN = constraintN % constraint;

    csVector3 isect;
    float dist;
    if (csIntersect3::SegmentPlane (vo, pickingRay, csPlane3(constraintPlaneN, -constraintPlaneN*dragCenter), isect, dist))
    {
      csVector3 worldDiff = isect-dragCenter;
      csReversibleTransform transform = GetOperationTransform(worldDiff);
      SoftTransform(ctx->GetActiveObject(), transform);
    }
    
    if (csMouseEventHelper::GetButton (ev) == 0)
    {
      return Commit(ctx, mesh);
    }
  }
  return OperatorRunningModal;
}

csReversibleTransform MoveOperator::GetOperationTransform(csVector3 worldDiff)
{
  // Apply XConstraint
  worldDiff.y = worldDiff.z = 0.0;
  
  //worldDiff.Norm();
  
  csReversibleTransform transform;
  transform.SetOrigin(worldDiff);
  
  return transform;
}

MoveObjectAction::MoveObjectAction (iObjectRegistry* object_reg, iObject* obj, const csReversibleTransform& trans) 
  : scfImplementationType (this), object_reg(object_reg), object(obj), transform(trans)
{
}

bool MoveObjectAction::Do () 
{ 
  if (object)
  {
    return SoftTransform(object, transform);
  }
  return false; 
}

bool MoveObjectAction::Undo () 
{ 
  if (object)
  {
    return SoftTransform(object, transform.GetInverse());
  }
  return false; 
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
