/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include <string.h>
#include "csutil/sysfunc.h"
#include "csgeom/matrix3.h"
#include "csgeom/box.h"
#include "csgeom/sphere.h"
#include "csgeom/math3d.h"
#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "csutil/randomgen.h"
#include "csutil/weakref.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivaria/sequence.h"
#include "ivaria/reporter.h"
#include "iengine/light.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/camera.h"
#include "iengine/rview.h"
#include "iengine/material.h"
#include "iengine/sharevar.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "engseq.h"

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

/**
 * The superclass of all sequence operations.
 */
class OpStandard : public iSequenceOperation
{
protected:
  virtual ~OpStandard() { SCF_DESTRUCT_IBASE(); }

public:
  SCF_DECLARE_IBASE;
  OpStandard () { SCF_CONSTRUCT_IBASE (0); }
  virtual void CleanupSequences () { }
};

SCF_IMPLEMENT_IBASE (OpStandard)
  SCF_IMPLEMENTS_INTERFACE (iSequenceOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * The superclass of all sequence conditions.
 */
class CondStandard : public iSequenceCondition
{
protected:
  virtual ~CondStandard() { SCF_DESTRUCT_IBASE(); }

public:
  SCF_DECLARE_IBASE;
  CondStandard () { SCF_CONSTRUCT_IBASE (0); }
};

SCF_IMPLEMENT_IBASE (CondStandard)
  SCF_IMPLEMENTS_INTERFACE (iSequenceCondition)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

/**
 * Set fog operation.
 */
class OpSetFog : public OpStandard
{
private:
  csRef<iParameterESM> sectorpar;
  csRef<iSector> sector;
  csColor color;
  float density;

public:
  OpSetFog (iParameterESM* sectorpar, const csColor& color, float density)
  {
    if (sectorpar->IsConstant ())
      sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (), iSector);
    else
      OpSetFog::sectorpar = sectorpar;
    OpSetFog::color = color;
    OpSetFog::density = density;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (sectorpar)
      sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (params), iSector);
    if (density < 0.001)
      sector->DisableFog ();
    else
      sector->SetFog (density, color);
    if (sectorpar)
      sector = 0;
  }
};

class FadeFogInfo : public iSequenceTimedOperation
{
public:
  csRef<iSector> sector;
  csColor start_col, end_col;
  float start_density, end_density;

  SCF_DECLARE_IBASE;
  FadeFogInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~FadeFogInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    float density = (1-time) * start_density + time * end_density;
    if (density < 0.001)
      sector->DisableFog ();
    else
    {
      csColor color;
      color.red = (1-time) * start_col.red + time * end_col.red;
      color.green = (1-time) * start_col.green + time * end_col.green;
      color.blue = (1-time) * start_col.blue + time * end_col.blue;
      sector->SetFog (density, color);
    }
  }
};

SCF_IMPLEMENT_IBASE (FadeFogInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade fog operation.
 */
class OpFadeFog : public OpStandard
{
private:
  csRef<iParameterESM> sectorpar;
  csRef<iSector> sector;
  csColor end_col;
  float end_density;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeFog (iParameterESM* sectorpar, const csColor& color, float density,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    if (sectorpar->IsConstant ())
      sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (), iSector);
    else
      OpFadeFog::sectorpar = sectorpar;
    OpFadeFog::end_col = color;
    OpFadeFog::end_density = density;
    OpFadeFog::duration = duration;
    OpFadeFog::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    if (sectorpar)
      sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (params), iSector);
    csFog* fog = sector->GetFog ();

    FadeFogInfo* fi = new FadeFogInfo ();
    fi->start_col.red = fog->red;
    fi->start_col.green = fog->green;
    fi->start_col.blue = fog->blue;
    fi->start_density = fog->density;
    fi->end_col = end_col;
    fi->end_density = end_density;
    fi->sector = sector;
    eseqmgr->FireTimedOperation (dt, duration, fi);
    fi->DecRef ();
    if (sectorpar)
      sector = 0;
  }
};

//---------------------------------------------------------------------------

/**
 * Set variable operation.
 */
class OpSetVariable : public OpStandard
{
private:
# define OP_SET_VAR_VALUE 0
# define OP_SET_VAR_DVALUE 1
# define OP_SET_VALUE 2
# define OP_SET_DVALUE 3
# define OP_SET_VECTOR 4
# define OP_SET_COLOR 5
  int op;
  iSharedVariable* var;
  iSharedVariable* value_var;
  float value;
  csVector3 vector;
  csColor color;

public:
  OpSetVariable (iSharedVariable* var, float value, float dvalue)
  {
    OpSetVariable::var = var;
    if (dvalue != 0)
    {
      op = OP_SET_DVALUE;
      OpSetVariable::value = dvalue;
    }
    else
    {
      op = OP_SET_VALUE;
      OpSetVariable::value = value;
    }
  }
  OpSetVariable (iSharedVariable* var, iSharedVariable* value,
  	iSharedVariable* dvalue)
  {
    OpSetVariable::var = var;
    if (dvalue != 0)
    {
      op = OP_SET_VAR_DVALUE;
      OpSetVariable::value_var = dvalue;
    }
    else
    {
      op = OP_SET_VAR_VALUE;
      OpSetVariable::value_var = value;
    }
  }
  OpSetVariable (iSharedVariable* var, const csVector3& v)
  {
    OpSetVariable::var = var;
    op = OP_SET_VECTOR;
    vector = v;
  }
  OpSetVariable (iSharedVariable* var, const csColor& c)
  {
    OpSetVariable::var = var;
    op = OP_SET_COLOR;
    color = c;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    switch (op)
    {
      case OP_SET_VAR_VALUE:
        switch (value_var->GetType ())
	{
          case iSharedVariable::SV_FLOAT:
            var->Set (value_var->Get ());
	    break;
          case iSharedVariable::SV_COLOR:
            var->SetColor (value_var->GetColor ());
	    break;
          case iSharedVariable::SV_VECTOR:
            var->SetVector (value_var->GetVector ());
	    break;
	}
        break;
      case OP_SET_VAR_DVALUE:
        var->Set (var->Get () + value_var->Get ());
        break;
      case OP_SET_VALUE:
        var->Set (value);
        break;
      case OP_SET_DVALUE:
        var->Set (var->Get () + value);
        break;
      case OP_SET_VECTOR:
        var->SetVector (vector);
        break;
      case OP_SET_COLOR:
        var->SetColor (color);
        break;
    }
  }
};

//---------------------------------------------------------------------------

/**
 * Set material operation.
 */
class OpSetMaterial : public OpStandard
{
private:
  csRef<iParameterESM> meshpar;
  csRef<iParameterESM> polygonpar;
  csRef<iParameterESM> materialpar;
  csRef<iMeshWrapper> mesh;
  csRef<iPolygonHandle> polygon;
  csRef<iMaterialWrapper> material;

public:
  OpSetMaterial (iParameterESM* meshpar, iParameterESM* polygonpar,
  	iParameterESM* materialpar)
  {
    if (meshpar)
    {
      if (meshpar->IsConstant ())
        mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (), iMeshWrapper);
      else
        OpSetMaterial::meshpar = meshpar;
    }
    if (polygonpar)
    {
      if (polygonpar->IsConstant ())
        polygon = SCF_QUERY_INTERFACE (polygonpar->GetValue (), iPolygonHandle);
      else
        OpSetMaterial::polygonpar = polygonpar;
    }
    if (materialpar->IsConstant ())
      material = SCF_QUERY_INTERFACE (
      	materialpar->GetValue (), iMaterialWrapper);
    else
      OpSetMaterial::materialpar = materialpar;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (materialpar)
      material = SCF_QUERY_INTERFACE (
      	materialpar->GetValue (params), iMaterialWrapper);
    if (polygon || polygonpar)
    {
      if (polygonpar)
        polygon = SCF_QUERY_INTERFACE (polygonpar->GetValue (params),
		iPolygonHandle);
      int poly_idx = polygon->GetIndex ();
      iThingFactoryState* tfs = polygon->GetThingFactoryState ();
      if (tfs)
      {
	tfs->SetPolygonMaterial (CS_POLYRANGE_SINGLE (poly_idx), material);
      }
      if (polygonpar)
        polygon = 0;
    }
    else
    {
      if (meshpar)
        mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iMeshWrapper);
      mesh->GetMeshObject ()->SetMaterialWrapper (material);
      if (meshpar)
        mesh = 0;
    }
    if (materialpar)
      material = 0;
  }
};

//---------------------------------------------------------------------------

/**
 * Set light operation.
 */
class OpSetLight : public OpStandard
{
private:
  csRef<iParameterESM> lightpar;
  csRef<iLight> light;
  csColor color;

public:
  OpSetLight (iParameterESM* lightpar, const csColor& color)
  {
    if (lightpar->IsConstant ())
      light = SCF_QUERY_INTERFACE (lightpar->GetValue (), iLight);
    else
      OpSetLight::lightpar = lightpar;
    OpSetLight::color = color;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (lightpar)
      light = SCF_QUERY_INTERFACE (lightpar->GetValue (params), iLight);
    light->SetColor (color);
    if (lightpar)
      light = 0;
  }
};

class FadeLightInfo : public iSequenceTimedOperation
{
public:
  csRef<iLight> light;
  csColor start_col, end_col;

  SCF_DECLARE_IBASE;
  FadeLightInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~FadeLightInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csColor color;
    color.red = (1-time) * start_col.red + time * end_col.red;
    color.green = (1-time) * start_col.green + time * end_col.green;
    color.blue = (1-time) * start_col.blue + time * end_col.blue;
    light->SetColor (color);
  }
};

SCF_IMPLEMENT_IBASE (FadeLightInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade light operation.
 */
class OpFadeLight : public OpStandard
{
private:
  csRef<iParameterESM> lightpar;
  csRef<iLight> light;
  csColor end_col;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeLight (iParameterESM* lightpar, const csColor& color,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    if (lightpar->IsConstant ())
      light = SCF_QUERY_INTERFACE (lightpar->GetValue (), iLight);
    else
      OpFadeLight::lightpar = lightpar;
    OpFadeLight::end_col = color;
    OpFadeLight::duration = duration;
    OpFadeLight::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    if (lightpar)
      light = SCF_QUERY_INTERFACE (lightpar->GetValue (params), iLight);
    FadeLightInfo* fl = new FadeLightInfo ();
    fl->light = light;
    fl->start_col = light->GetColor ();
    fl->end_col = end_col;
    eseqmgr->FireTimedOperation (dt, duration, fl);
    fl->DecRef ();
    if (lightpar)
      light = 0;
  }
};

//---------------------------------------------------------------------------

static iEngineSequenceManager* debug_eseqmgr;//@@@@@@@@@@@@@@@@@@@@@@@@@@

/**
 * Set ambient light operation.
 */
class OpSetAmbientLight : public OpStandard
{
private:
  csRef<iSector> sector;
  csColor color;
  iSharedVariable *colorvar;

public:
  OpSetAmbientLight (iParameterESM* sectorpar, const csColor& color,
		     iSharedVariable *varcolor)
  {
    sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (), iSector);
    if (varcolor)
      colorvar = varcolor;
    else
    {
      colorvar = 0;
      OpSetAmbientLight::color = color;
    }
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    sector->SetDynamicAmbientLight (colorvar?colorvar->GetColor():color);
  }
};

class FadeAmbientLightInfo : public iSequenceTimedOperation
{
public:
  csRef<iSector> sector;
  csColor start_col, end_col;

  SCF_DECLARE_IBASE;
  FadeAmbientLightInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~FadeAmbientLightInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csColor color;
    color.red = (1-time) * start_col.red + time * end_col.red;
    color.green = (1-time) * start_col.green + time * end_col.green;
    color.blue = (1-time) * start_col.blue + time * end_col.blue;
    sector->SetDynamicAmbientLight (color);
  }
};

SCF_IMPLEMENT_IBASE (FadeAmbientLightInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade light operation.
 */
class OpFadeAmbientLight : public OpStandard
{
private:
  csRef<iSector> sector;
  csColor end_col;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeAmbientLight (iParameterESM* sectorpar, const csColor& color,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    sector = SCF_QUERY_INTERFACE (sectorpar->GetValue (), iSector);
    OpFadeAmbientLight::end_col = color;
    OpFadeAmbientLight::duration = duration;
    OpFadeAmbientLight::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    FadeAmbientLightInfo* fl = new FadeAmbientLightInfo ();
    fl->sector = sector;
    fl->start_col = sector->GetDynamicAmbientLight ();
    fl->end_col = end_col;
    eseqmgr->FireTimedOperation (dt, duration, fl);
    fl->DecRef ();
  }
};

//---------------------------------------------------------------------------

/**
 * Set mesh color operation.
 */
class OpSetMeshColor : public OpStandard
{
private:
  csRef<iParameterESM> meshpar;
  csRef<iMeshWrapper> mesh;
  csColor color;

public:
  OpSetMeshColor (iParameterESM* meshpar, const csColor& color)
  {
    if (meshpar->IsConstant ())
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (), iMeshWrapper);
    else
      OpSetMeshColor::meshpar = meshpar;
    OpSetMeshColor::color = color;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (meshpar)
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iMeshWrapper);
    mesh->GetMeshObject ()->SetColor (color);
    if (meshpar)
      mesh = 0;
  }
};

class FadeMeshColorInfo : public iSequenceTimedOperation
{
public:
  csRef<iMeshWrapper> mesh;
  csColor start_col, end_col;
  float start_density, end_density;

  SCF_DECLARE_IBASE;
  FadeMeshColorInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~FadeMeshColorInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csColor color;
    color.red = (1-time) * start_col.red + time * end_col.red;
    color.green = (1-time) * start_col.green + time * end_col.green;
    color.blue = (1-time) * start_col.blue + time * end_col.blue;
    mesh->GetMeshObject ()->SetColor (color);
  }
};

SCF_IMPLEMENT_IBASE (FadeMeshColorInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade mesh operation.
 */
class OpFadeMeshColor : public OpStandard
{
private:
  csRef<iParameterESM> meshpar;
  csRef<iMeshWrapper> mesh;
  csColor end_col;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeMeshColor (iParameterESM* meshpar, const csColor& color,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    if (meshpar->IsConstant ())
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (), iMeshWrapper);
    else
      OpFadeMeshColor::meshpar = meshpar;
    OpFadeMeshColor::end_col = color;
    OpFadeMeshColor::duration = duration;
    OpFadeMeshColor::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    if (meshpar)
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iMeshWrapper);
    FadeMeshColorInfo* fm = new FadeMeshColorInfo ();
    fm->mesh = mesh;
    mesh->GetMeshObject ()->GetColor (fm->start_col);
    fm->end_col = end_col;
    eseqmgr->FireTimedOperation (dt, duration, fm);
    fm->DecRef ();
    if (meshpar)
      mesh = 0;
  }
};

//---------------------------------------------------------------------------

class RotateInfo : public iSequenceTimedOperation
{
public:
  csRef<iMeshWrapper> mesh;
  int axis1, axis2, axis3;
  float tot_angle1, tot_angle2, tot_angle3;
  csVector3 offset;
  csReversibleTransform start_transform;

  SCF_DECLARE_IBASE;
  RotateInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~RotateInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csReversibleTransform trans = start_transform;
    trans.Translate (-offset);
    csVector3 o (0);
    switch (axis1)
    {
      case -1:
        break;
      case 0:
        trans = trans * csTransform (csXRotMatrix3 (tot_angle1*time), o);
	break;
      case 1:
        trans = trans * csTransform (csYRotMatrix3 (tot_angle1*time), o);
	break;
      case 2:
        trans = trans * csTransform (csZRotMatrix3 (tot_angle1*time), o);
	break;
    }
    switch (axis2)
    {
      case -1:
        break;
      case 0:
        trans = trans * csTransform (csXRotMatrix3 (tot_angle2*time), o);
	break;
      case 1:
        trans = trans * csTransform (csYRotMatrix3 (tot_angle2*time), o);
	break;
      case 2:
        trans = trans * csTransform (csZRotMatrix3 (tot_angle2*time), o);
	break;
    }
    switch (axis3)
    {
      case -1:
        break;
      case 0:
        trans = trans * csTransform (csXRotMatrix3 (tot_angle3*time), o);
	break;
      case 1:
        trans = trans * csTransform (csYRotMatrix3 (tot_angle3*time), o);
	break;
      case 2:
        trans = trans * csTransform (csZRotMatrix3 (tot_angle3*time), o);
	break;
    }
    trans.Translate (offset);
    mesh->GetMovable ()->SetTransform (trans);
    mesh->GetMovable ()->UpdateMove ();
  }
};

SCF_IMPLEMENT_IBASE (RotateInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Rotate operation.
 */
class OpRotate : public OpStandard
{
private:
  csRef<iParameterESM> meshpar;
  csRef<iMeshWrapper> mesh;
  int axis1, axis2, axis3;
  float tot_angle1, tot_angle2, tot_angle3;
  csVector3 offset;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpRotate (iParameterESM* meshpar,
  	int axis1, float tot_angle1,
  	int axis2, float tot_angle2,
  	int axis3, float tot_angle3,
	const csVector3& offset,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    if (meshpar->IsConstant ())
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (), iMeshWrapper);
    else
      OpRotate::meshpar = meshpar;
    OpRotate::axis1 = axis1;
    OpRotate::tot_angle1 = tot_angle1;
    OpRotate::axis2 = axis2;
    OpRotate::tot_angle2 = tot_angle2;
    OpRotate::axis3 = axis3;
    OpRotate::tot_angle3 = tot_angle3;
    OpRotate::offset = offset;
    OpRotate::duration = duration;
    OpRotate::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    if (meshpar)
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iMeshWrapper);
    iMovable* movable = mesh->GetMovable ();
    RotateInfo* ri = new RotateInfo ();
    ri->mesh = mesh;
    ri->start_transform = movable->GetTransform ();
    ri->axis1 = axis1;
    ri->axis2 = axis2;
    ri->axis3 = axis3;
    ri->tot_angle1 = tot_angle1;
    ri->tot_angle2 = tot_angle2;
    ri->tot_angle3 = tot_angle3;
    ri->offset = offset;
    eseqmgr->FireTimedOperation (dt, duration, ri);
    ri->DecRef ();
    if (meshpar)
      mesh = 0;
  }
};

//---------------------------------------------------------------------------

class MoveLightInfo : public iSequenceTimedOperation
{
public:
  csRef<iLight> light;
  csVector3 start_pos;
  csVector3 offset;

  SCF_DECLARE_IBASE;
  MoveLightInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~MoveLightInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csVector3 new_pos = start_pos + time * offset;
    light->SetCenter (new_pos);
  }
};

SCF_IMPLEMENT_IBASE (MoveLightInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

class MoveInfo : public iSequenceTimedOperation
{
public:
  csRef<iMeshWrapper> mesh;
  csVector3 start_pos;
  csVector3 offset;

  SCF_DECLARE_IBASE;
  MoveInfo ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~MoveInfo ()
  { SCF_DESTRUCT_IBASE(); }

  virtual void Do (float time, iBase*)
  {
    csVector3 new_pos = start_pos + time * offset;
    mesh->GetMovable ()->GetTransform ().SetOrigin (new_pos);
    mesh->GetMovable ()->UpdateMove ();
  }
};

SCF_IMPLEMENT_IBASE (MoveInfo)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Move operation.
 */
class OpMove : public OpStandard
{
private:
  csRef<iParameterESM> meshpar;
  csRef<iMeshWrapper> mesh;
  csRef<iLight> light;
  csVector3 offset;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpMove (iParameterESM* meshpar,
	const csVector3& offset,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    if (meshpar->IsConstant ())
    {
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (), iMeshWrapper);
      if (!mesh)
        light = SCF_QUERY_INTERFACE (meshpar->GetValue (), iLight);
    }
    else
    {
      OpMove::meshpar = meshpar;
    }
    OpMove::offset = offset;
    OpMove::duration = duration;
    OpMove::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt, iBase* params)
  {
    if (meshpar)
    {
      mesh = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iMeshWrapper);
      if (!mesh)
        light = SCF_QUERY_INTERFACE (meshpar->GetValue (params), iLight);
    }
    if (mesh)
    {
      iMovable* movable = mesh->GetMovable ();
      MoveInfo* mi = new MoveInfo ();
      mi->mesh = mesh;
      mi->start_pos = movable->GetTransform ().GetOrigin ();
      mi->offset = offset;
      eseqmgr->FireTimedOperation (dt, duration, mi);
      mi->DecRef ();
    }
    else if (light)
    {
      MoveLightInfo* mi = new MoveLightInfo ();
      mi->light = light;
      mi->start_pos = light->GetCenter ();
      mi->offset = offset;
      eseqmgr->FireTimedOperation (dt, duration, mi);
      mi->DecRef ();
    }
    if (meshpar)
    {
      mesh = 0;
      light = 0;
    }
  }
};

/**
 * Random Delay operation.
 */
class OpRandomDelay : public OpStandard
{
private:
  iEngineSequenceManager* eseqmgr;
  int min,max;
  csRandomGen *rg;
  csSequenceWrapper *sequence;

public:
  OpRandomDelay (int min_int, int max_int,
  	csSequenceWrapper *seq,
	iEngineSequenceManager* seqmgr)
  {
    min = min_int;
    max = max_int;
    sequence = seq;
    eseqmgr  = seqmgr;
    rg = new csRandomGen(csGetTicks()+(intptr_t)this);  // seed rng
  }
  virtual ~OpRandomDelay ()
  {
    delete rg;
  }
  virtual void Do (csTicks dt, iBase* params)
  {
    int delay = rg->Get(max-min) + min;
    sequence->OverrideTimings(this, delay);
  }
};

//---------------------------------------------------------------------------

/**
 * Set trigger state.
 */
class OpTriggerState : public OpStandard
{
private:
  csRef<iParameterESM> triggerpar;
  csRef<iSequenceTrigger> trigger;
  bool en;

public:
  OpTriggerState (iParameterESM* triggerpar, bool en)
  {
    if (triggerpar->IsConstant ())
      trigger = SCF_QUERY_INTERFACE (triggerpar->GetValue (), iSequenceTrigger);
    else
      OpTriggerState::triggerpar = triggerpar;
    OpTriggerState::en = en;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (triggerpar)
      trigger = SCF_QUERY_INTERFACE (
      	triggerpar->GetValue (params), iSequenceTrigger);
    trigger->SetEnabled (en);
    if (triggerpar)
      trigger = 0;
  }
};

//---------------------------------------------------------------------------

/**
 * Check trigger.
 */
class OpCheckTrigger : public OpStandard
{
private:
  csRef<iParameterESM> triggerpar;
  csRef<iSequenceTrigger> trigger;
  csTicks delay;

public:
  OpCheckTrigger (iParameterESM* triggerpar, csTicks delay)
  {
    if (triggerpar->IsConstant ())
      trigger = SCF_QUERY_INTERFACE (triggerpar->GetValue (), iSequenceTrigger);
    else
      OpCheckTrigger::triggerpar = triggerpar;
    OpCheckTrigger::delay = delay;
  }

  virtual void Do (csTicks /*dt*/, iBase* params)
  {
    if (triggerpar)
      trigger = SCF_QUERY_INTERFACE (
      	triggerpar->GetValue (params), iSequenceTrigger);
    trigger->TestConditions (delay);
    if (triggerpar)
      trigger = 0;
  }
};

//---------------------------------------------------------------------------

/**
 * Condition to test a trigger.
 */
class CondTestTrigger : public CondStandard
{
private:
  csRef<iParameterESM> triggerpar;
  csRef<iSequenceTrigger> trigger;

public:
  CondTestTrigger (iParameterESM* triggerpar)
  {
    if (triggerpar->IsConstant ())
      trigger = SCF_QUERY_INTERFACE (triggerpar->GetValue (), iSequenceTrigger);
    else
      CondTestTrigger::triggerpar = triggerpar;
  }

  virtual bool Condition (csTicks /*dt*/, iBase* params)
  {
    if (triggerpar)
      trigger = SCF_QUERY_INTERFACE (
      	triggerpar->GetValue (params), iSequenceTrigger);
    bool rc = trigger->CheckState ();
    if (triggerpar)
      trigger = 0;
    return rc;
  }
};

//---------------------------------------------------------------------------

class esmPar : public iParameterESM
{
private:
  size_t idx;

public:
  SCF_DECLARE_IBASE;
  esmPar (size_t idx)
  {
    SCF_CONSTRUCT_IBASE (0);
    esmPar::idx = idx;
  }
  virtual ~esmPar ()
  {
    SCF_DESTRUCT_IBASE();
  }
  virtual iBase* GetValue (iBase* params = 0) const
  {
    // The following cast is in theory unsafe but in this case it is
    // ok since there is no way some other 'params' instance could get
    // here.
    iEngineSequenceParameters* par = (iEngineSequenceParameters*)params;
    return par->GetParameter (idx);
  }
  virtual bool IsConstant () const
  {
    return false;
  }
};

SCF_IMPLEMENT_IBASE (esmPar)
  SCF_IMPLEMENTS_INTERFACE (iParameterESM)
SCF_IMPLEMENT_IBASE_END

class constantPar : public iParameterESM
{
private:
  csRef<iBase> value;

public:
  SCF_DECLARE_IBASE;
  constantPar (iBase* value)
  {
    SCF_CONSTRUCT_IBASE (0);
    constantPar::value = value;
  }
  virtual ~constantPar ()
  {
    SCF_DESTRUCT_IBASE();
  }
  virtual iBase* GetValue (iBase* params = 0) const
  {
    (void)params;
    return value;
  }
  virtual bool IsConstant () const
  {
    return true;
  }
};

SCF_IMPLEMENT_IBASE (constantPar)
  SCF_IMPLEMENTS_INTERFACE (iParameterESM)
SCF_IMPLEMENT_IBASE_END

class sharedvarPar : public iParameterESM
{
private:
  csRef<iBase> value;

public:
  SCF_DECLARE_IBASE;
  sharedvarPar (iBase* value)
  {
    SCF_CONSTRUCT_IBASE (0);
    sharedvarPar::value = value;
  }
  virtual ~sharedvarPar ()
  {
    SCF_DESTRUCT_IBASE();
  }
  virtual iBase* GetValue (iBase* params = 0) const
  {
    (void)params;
    return value;
  }
  virtual bool IsConstant () const
  {
    return false;
  }
};

SCF_IMPLEMENT_IBASE (sharedvarPar)
  SCF_IMPLEMENTS_INTERFACE (iParameterESM)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csEngineSequenceParameters)
  SCF_IMPLEMENTS_INTERFACE (iEngineSequenceParameters)
SCF_IMPLEMENT_IBASE_END

csPtr<iParameterESM> csEngineSequenceParameters::CreateParameterESM (
	const char* name)
{
  size_t idx = GetParameterIdx (name);
  if (idx == csArrayItemNotFound) return 0;
  csRef<iParameterESM> par = csPtr<iParameterESM> (new esmPar (idx));
  return csPtr<iParameterESM>(par);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csSequenceWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSequenceWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSequenceWrapper::SequenceWrapper)
  SCF_IMPLEMENTS_INTERFACE (iSequenceWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSequenceWrapper::csSequenceWrapper (csEngineSequenceManager* eseqmgr,
	iSequence* sequence)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSequenceWrapper);
  csSequenceWrapper::eseqmgr = eseqmgr;
  csSequenceWrapper::sequence = sequence;
}

csSequenceWrapper::~csSequenceWrapper ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSequenceWrapper);
}

iEngineSequenceParameters* csSequenceWrapper::CreateBaseParameterBlock ()
{
  params = csPtr<csEngineSequenceParameters> (
  	new csEngineSequenceParameters ());
  return params;
}

iEngineSequenceParameters* csSequenceWrapper::GetBaseParameterBlock ()
{
  return params;
}

csPtr<iEngineSequenceParameters> csSequenceWrapper::CreateParameterBlock ()
{
  if (!params) return 0;
  csEngineSequenceParameters* copyparams = new csEngineSequenceParameters ();

  size_t i;
  for (i = 0 ; i < params->GetParameterCount () ; i++)
  {
    const char* name = params->GetParameterName (i);
    iBase* value = params->GetParameter (i);
    copyparams->AddParameter (name, value);
  }

  return csPtr<iEngineSequenceParameters> (copyparams);
}

void csSequenceWrapper::AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, float value, float dvalue)
{
  OpSetVariable* op = new OpSetVariable (var, value, dvalue);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, iSharedVariable* value,
		iSharedVariable* dvalue)
{
  OpSetVariable* op = new OpSetVariable (var, value, dvalue);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csVector3& v)
{
  OpSetVariable* op = new OpSetVariable (var, v);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csColor& c)
{
  OpSetVariable* op = new OpSetVariable (var, c);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetPolygonMaterial (csTicks time,
	iParameterESM* polygon, iParameterESM* material)
{
  OpSetMaterial* op = new OpSetMaterial (0, polygon, material);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetMaterial (csTicks time,
	iParameterESM* mesh, iParameterESM* material)
{
  OpSetMaterial* op = new OpSetMaterial (mesh, 0, material);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetLight (csTicks time,
	iParameterESM* light, const csColor& color)
{
  OpSetLight* op = new OpSetLight (light, color);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeLight (csTicks time,
	iParameterESM* light, const csColor& color, csTicks duration)
{
  OpFadeLight* op = new OpFadeLight (light, color, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetAmbient (csTicks time,
	iParameterESM* sector, const csColor& color,iSharedVariable *var)
{
  OpSetAmbientLight* op = new OpSetAmbientLight (sector, color, var);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeAmbient (csTicks time,
	iParameterESM* sector, const csColor& color, csTicks duration)
{
  OpFadeAmbientLight* op = new OpFadeAmbientLight (sector, color, duration,
  	eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationRandomDelay(csTicks time,int min, int max)
{
  OpRandomDelay* op = new OpRandomDelay (min,max,this,eseqmgr);
  sequence->AddOperation (time,op);
  op->DecRef();
}

void csSequenceWrapper::AddOperationSetMeshColor (csTicks time,
	iParameterESM* mesh, const csColor& color)
{
  OpSetMeshColor* op = new OpSetMeshColor (mesh, color);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeMeshColor (csTicks time,
	iParameterESM* mesh, const csColor& color, csTicks duration)
{
  OpFadeMeshColor* op = new OpFadeMeshColor (mesh, color, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetFog (csTicks time,
	iParameterESM* sector, const csColor& color, float density)
{
  OpSetFog* op = new OpSetFog (sector, color, density);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeFog (csTicks time,
	iParameterESM* sector, const csColor& color, float density,
	csTicks duration)
{
  OpFadeFog* op = new OpFadeFog (sector, color, density, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationRotateDuration (csTicks time,
	iParameterESM* mesh,
	int axis1, float tot_angle1,
	int axis2, float tot_angle2,
	int axis3, float tot_angle3,
	const csVector3& offset, csTicks duration)
{
  OpRotate* op = new OpRotate (mesh,
  	axis1, tot_angle1, axis2, tot_angle2, axis3, tot_angle3,
	offset, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationMoveDuration (csTicks time,
	iParameterESM* mesh, const csVector3& offset,
	csTicks duration)
{
  OpMove* op = new OpMove (mesh, offset,
	duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationTriggerState (csTicks time,
	iParameterESM* trigger, bool en)
{
  OpTriggerState* op = new OpTriggerState (trigger, en);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationCheckTrigger (csTicks time,
	iParameterESM* trigger, csTicks delay)
{
  OpCheckTrigger* op = new OpCheckTrigger (trigger, delay);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationTestTrigger (csTicks time,
	iParameterESM* trigger,
	iSequence* trueSequence,
	iSequence* falseSequence)
{
  CondTestTrigger* cond = new CondTestTrigger (trigger);
  sequence->AddCondition (time, cond, trueSequence, falseSequence);
  cond->DecRef ();
}

void csSequenceWrapper::OverrideTimings (OpStandard *afterop, int ticks)
{
  csSequenceOp *curr;
  int time_diff=0;

  for (curr = sequence->GetFirstSequence(); curr; curr = curr->next)
  {
    if (curr->operation == afterop)
    {
      // calculate delta relative to time which is already set there
      csSequenceOp *nextop = curr->next;
      if (nextop)
        time_diff = curr->time + ticks - nextop->time;
    }
    else if (time_diff)
    {
      // adjust time for op by diff amount
      curr->time += time_diff;
    }
  }
}

//---------------------------------------------------------------------------

/**
 * Callback that will activate trigger when sector is visible.
 */
class csTriggerSectorCallback : public iSectorCallback
{
private:
  csSequenceTrigger* trigger;
  bool insideonly;
  bool do_box;
  csBox3 box;
  bool do_sphere;
  csSphere sphere;
  uint32 framenr;

public:
  SCF_DECLARE_IBASE;

  csTriggerSectorCallback (csSequenceTrigger* trigger,
	bool insideonly, const csBox3* box, const csSphere* sphere)
  {
    SCF_CONSTRUCT_IBASE (0);
    csTriggerSectorCallback::trigger = trigger;
    csTriggerSectorCallback::insideonly = insideonly;
    if (box)
    {
      do_box = true;
      csTriggerSectorCallback::box = *box;
    }
    else
    {
      do_box = false;
    }
    if (sphere)
    {
      do_sphere = true;
      csTriggerSectorCallback::sphere = *sphere;
    }
    else
    {
      do_sphere = false;
    }
    framenr = 0;
  }
  virtual ~csTriggerSectorCallback ()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual void Traverse (iSector* /*sector*/, iBase* context)
  {
    csRef<iRenderView> rview (SCF_QUERY_INTERFACE (context, iRenderView));
    if (rview)
    {
      uint32 global_framenr = trigger->GetEngineSequenceManager ()
      	->GetGlobalFrameNr ();
      if (framenr != global_framenr)
      {
	// It is potentially useful to fire. So we try to see if
	// all conditions are met.
	if (insideonly && rview->GetPreviousSector () != 0)
	  return;
	if (do_sphere)
	{
	  const csVector3& pos = rview->GetCamera ()
		  ->GetTransform ().GetOrigin ();
	  float sqd = csSquaredDist::PointPoint (pos, sphere.GetCenter ());
	  if (sqd > sphere.GetRadius () * sphere.GetRadius ())
	    return;
	}
	if (do_box)
	{
	  const csVector3& pos = rview->GetCamera ()
		  ->GetTransform ().GetOrigin ();
	  if (!box.In (pos))
	    return;
	}

        framenr = global_framenr;
	trigger->Fire ();
      }
    }
  }
};

SCF_IMPLEMENT_IBASE (csTriggerSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iSectorCallback)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

/**
 * Callback that will activate trigger when light crosses threshold value.
 */
class csTriggerLightCallback : public iLightCallback
{
private:
  csSequenceTrigger* trigger;
  int operation;  /* 0=always, 1= <color, 2= >color */
  csColor trigger_color,last_color;
  unsigned int framenr;

public:
  SCF_DECLARE_IBASE;

  csTriggerLightCallback (csSequenceTrigger* trigger,
	int oper, const csColor& col)
  {
    SCF_CONSTRUCT_IBASE (0);
    csTriggerLightCallback::trigger = trigger;
    operation = oper;
    trigger_color = col;
    framenr = 0;
  }
  virtual ~csTriggerLightCallback ()
  {
    SCF_DESTRUCT_IBASE();
  }

  float AverageColor (const csColor& col)
  {
    return (col.red + col.blue + col.green)/3;
  }

  virtual void OnColorChange (iLight*, const csColor& col)
  {
    uint32 global_framenr = trigger->GetEngineSequenceManager ()
      	->GetGlobalFrameNr ();
    if (framenr != global_framenr)
    {
      if (operation == CS_SEQUENCE_LIGHTCHANGE_LESS) // new color less than trigger color
      {
	if ( AverageColor (col) >= AverageColor (trigger_color) )
	  return;
      }
      else if (operation == CS_SEQUENCE_LIGHTCHANGE_GREATER)
      {
	if ( AverageColor (col) <= AverageColor (trigger_color) )
	  return;
      }
      framenr = global_framenr;
      trigger->Fire ();
    }
  }

  virtual void OnPositionChange (iLight*, const csVector3&) { }
  virtual void OnSectorChange (iLight*, iSector*) { }
  virtual void OnRadiusChange (iLight*, float) { }
  virtual void OnDestroy (iLight*) { }
  virtual void OnAttenuationChange (iLight* light, int newatt) { }
};

SCF_IMPLEMENT_IBASE (csTriggerLightCallback)
  SCF_IMPLEMENTS_INTERFACE (iLightCallback)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

/**
 * Cleanup a sector callback.
 */
class csConditionCleanupSectorCB : public csConditionCleanup
{
private:
  csWeakRef<iSector> sector;
  csRef<iSectorCallback> cb;

public:
  csConditionCleanupSectorCB (iSector* sect, iSectorCallback* cb)
  {
    sector = sect;
    csConditionCleanupSectorCB::cb = cb;
  }
  virtual void Cleanup ()
  {
    if (sector && cb)
    {
      sector->RemoveSectorCallback (cb);
    }
  }
};

//---------------------------------------------------------------------------

/**
 * Cleanup a light callback.
 */
class csConditionCleanupLightCB : public csConditionCleanup
{
private:
  csWeakRef<iLight> mylight;
  csRef<iLightCallback> cb;

public:
  csConditionCleanupLightCB (iLight* light, iLightCallback* cb)
  {
    mylight = light;
    csConditionCleanupLightCB::cb = cb;
  }
  virtual void Cleanup ()
  {
    if (mylight && cb)
    {
      mylight->RemoveLightCallback (cb);
    }
  }
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csSequenceTrigger)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSequenceTrigger)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSequenceTrigger::SequenceTrigger)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTrigger)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSequenceTrigger::csSequenceTrigger (csEngineSequenceManager* eseqmgr)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSequenceTrigger);
  enabled = true;
  enable_onetest = false;
  onetest_framenr = 0;
  fire_delay = 0;
  csSequenceTrigger::eseqmgr = eseqmgr;
  framenr = 0;
  total_conditions = 0;
  last_trigger_state = false;
  condtest_delay = 0;
}

csSequenceTrigger::~csSequenceTrigger ()
{
  ClearConditions ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSequenceTrigger);
}

void csSequenceTrigger::AddConditionInSector (iSector* sector,
	bool insideonly, const csBox3* box, const csSphere* sphere)
{
  csTriggerSectorCallback* trig = new csTriggerSectorCallback (this,
		  insideonly, box, sphere);
  sector->SetSectorCallback (trig);

  csConditionCleanupSectorCB* cleanup = new csConditionCleanupSectorCB (
  	sector, trig);
  condition_cleanups.Push (cleanup);
 
  cleanup->DecRef ();
  trig->DecRef ();

  total_conditions++;
}

void csSequenceTrigger::AddConditionMeshClick (iMeshWrapper* mesh)
{
  eseqmgr->RegisterMeshTrigger (this);
  click_mesh = mesh;
  total_conditions++;
}

void csSequenceTrigger::AddConditionLightChange (iLight *whichlight, 
						 int oper, const csColor& col)
{
  csTriggerLightCallback* trig = new csTriggerLightCallback (this,
							     oper, col);
  whichlight->SetLightCallback (trig);

  csConditionCleanupLightCB* cleanup = new csConditionCleanupLightCB (
  	whichlight, trig);
  condition_cleanups.Push (cleanup);
 
  cleanup->DecRef ();
  trig->DecRef ();

  total_conditions++;
}

void csSequenceTrigger::AddConditionManual ()
{
}

void csSequenceTrigger::ClearConditions ()
{
  total_conditions = 0;
  fired_conditions = 0;
  framenr = 0;
  condition_cleanups.DeleteAll ();
  click_mesh = 0;
}

void csSequenceTrigger::Trigger ()
{
}

void csSequenceTrigger::FireSequence (csTicks delay, iSequenceWrapper* seq)
{
  fire_sequence = seq;
  fire_delay = delay;
}

void csSequenceTrigger::EnableOneTest ()
{
  if (enable_onetest && onetest_framenr == 0)
  {
    // Since we last enabled the test nothing has happened.
    // That means that we can consider the test to have
    // failed since nothing fired (i.e. Fire() wasn't called).
    last_trigger_state = false;
    return;
  }
  enable_onetest = true;
  onetest_framenr = 0;	// We don't know the frame yet.
}

bool csSequenceTrigger::CheckState ()
{
  return last_trigger_state;
}

void csSequenceTrigger::Fire ()
{
  if (enabled)
  {
    enable_onetest = false;
    uint32 global_framenr = eseqmgr->GetGlobalFrameNr ();
    if (framenr != global_framenr)
    {
      framenr = global_framenr;
      fired_conditions = 0;
    }

    last_trigger_state = false;
    fired_conditions++;
    if (fired_conditions >= total_conditions)
    {
      last_trigger_state = true;
      // Only fire if trigger is enabled. Otherwise we are only
      // doing the test.
      eseqmgr->GetSequenceManager ()->RunSequence (fire_delay,
    	  fire_sequence->GetSequence (), params);
      enabled = false;
      fired_conditions = 0;
    }
  }
  else if (enable_onetest)
  {
    uint32 global_framenr = eseqmgr->GetGlobalFrameNr ();
    if (framenr != global_framenr)
    {
      // We start a new frame.
      if (onetest_framenr != 0)
      {
        // In this case we already did our test last frame.
	// Since we come here we know the trigger failed.
	enable_onetest = false;
	last_trigger_state = false;
	return;
      }
      framenr = global_framenr;
      onetest_framenr = global_framenr;
      fired_conditions = 0;
    }
    if (onetest_framenr == 0) return;	// Not busy testing yet.

    fired_conditions++;
    if (fired_conditions >= total_conditions)
    {
      last_trigger_state = true;
      fired_conditions = 0;
      enable_onetest = false;
    }
  }
}

void csSequenceTrigger::ForceFire (bool now)
{
  eseqmgr->GetSequenceManager ()->RunSequence (now ? 0 : fire_delay,
    	  fire_sequence->GetSequence (), params);
}

/**
 * Condition to loop for TestConditions().
 */
class CondTestConditions : public CondStandard
{
private:
  csSequenceTrigger* trigger;
  csTicks delay;

public:
  CondTestConditions (csSequenceTrigger* trigger, csTicks delay)
  {
    CondTestConditions::trigger = trigger;
    CondTestConditions::delay = delay;
  }

  virtual bool Condition (csTicks /*dt*/, iBase*)
  {
    // The sequence which loops this condition will end
    // automatically when the delay in the trigger is
    // different from this one.
    if (delay != trigger->GetConditionTestDelay ())
      return false;
    trigger->EnableOneTest ();
    return true;
  }
};

void csSequenceTrigger::TestConditions (csTicks delay)
{
  if (condtest_delay == delay) return;

  // By setting the condtest_delay to a different value we will
  // end the condition test sequence that is already running as soon
  // as it fires again.
  condtest_delay = delay;
  if (delay > 0)
  {
    // Here we already start a new sequence with the new delay.
    interval_seq = eseqmgr->GetSequenceManager ()->NewSequence ();
    CondTestConditions* cond = new CondTestConditions (this, delay);
    interval_seq->AddCondition (delay, cond, interval_seq, 0);
    cond->DecRef ();
    eseqmgr->GetSequenceManager ()->RunSequence (0, interval_seq, params);
  }
  else
  {
    interval_seq = 0;
  }
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csEngineSequenceManager)


SCF_IMPLEMENT_IBASE (csEngineSequenceManager)
  SCF_IMPLEMENTS_INTERFACE (iEngineSequenceManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csEngineSequenceManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csEngineSequenceManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csEngineSequenceManager::csEngineSequenceManager (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
  object_reg = 0;
  global_framenr = 1;
  debug_eseqmgr = this;//@@@@@@@@@@@@@
}

csEngineSequenceManager::~csEngineSequenceManager ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csEngineSequenceManager::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing | CSMASK_MouseDown);

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  seqmgr = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.sequence",
  	iSequenceManager);
  if (!seqmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.utilities.sequence.engine",
	"Couldn't load sequence manager plugin!");
    return false;
  }
  seqmgr->Resume ();

  return true;
}

bool csEngineSequenceManager::HandleEvent (iEvent &event)
{
  // Engine sequence manager must be post because frame must
  // be rendered and this must be fired BEFORE sequence manager. @@@ HACKY
  if (event.Type == csevBroadcast
	&& event.Command.Code == cscmdPostProcess)
  {
    global_framenr++;

    csTicks curtime = seqmgr->GetMainTime () + seqmgr->GetDeltaTime ();
    size_t i = timed_operations.Length ();
    while (i-- > 0)
    {
      csTimedOperation* op = timed_operations[i];
      if (curtime >= op->end)
      {
        op->op->Do (1.0, op->GetParams ());
        timed_operations.DeleteIndex (i);
      }
      else
      {
	float time = float (curtime-op->start) / float (op->end-op->start);
        op->op->Do (time, op->GetParams ());
      }
    }

    return true;
  }
  else if (event.Type == csevMouseDown)
  {
    int mouse_x = event.Mouse.x;
    int mouse_y = event.Mouse.y;
    if (camera != 0 && mesh_triggers.Length () > 0)
    {
      csVector3 v;
      // Setup perspective vertex, invert mouse Y axis.
      csVector2 p (mouse_x, camera->GetShiftY() * 2 - mouse_y);

      camera->InvPerspective (p, 1, v);
      csVector3 vw = camera->GetTransform ().This2Other (v);

      iSector* sector = camera->GetSector ();
      csVector3 origin = camera->GetTransform ().GetO2TTranslation ();
      csVector3 isect, end = origin + (vw - origin) * 120;

      int polyidx = -1;
      iMeshWrapper* sel = sector->HitBeam (origin, end, isect, &polyidx);

      size_t i;
      for (i = 0 ; i < mesh_triggers.Length () ; i++)
      {
	if (mesh_triggers[i]->GetClickMesh () == sel)
	  mesh_triggers[i]->Fire ();
      }

      //vw = isect;
      //v = camera->GetTransform ().Other2This (vw);
    }
  }
  return false;
}

void csEngineSequenceManager::RegisterMeshTrigger (
	csSequenceTrigger* trigger)
{
  if (mesh_triggers.Find (trigger) == csArrayItemNotFound)
    mesh_triggers.Push (trigger);
}

void csEngineSequenceManager::UnregisterMeshTrigger (
	csSequenceTrigger* trigger)
{
  mesh_triggers.Delete (trigger);
}

csPtr<iSequenceTrigger> csEngineSequenceManager::CreateTrigger (
	const char* name)
{
  csSequenceTrigger* trig = new csSequenceTrigger (this);
  trig->SetName (name);
  triggers.Push (&(trig->scfiSequenceTrigger));
  return &(trig->scfiSequenceTrigger);
}

csPtr<iParameterESM> csEngineSequenceManager::CreateParameterESM (iBase* value)
{
  csRef<iParameterESM> par = csPtr<iParameterESM> (new constantPar (value));
  return csPtr<iParameterESM>(par);
}

void csEngineSequenceManager::RemoveTrigger (iSequenceTrigger* trigger)
{
  triggers.Delete (trigger);
}

void csEngineSequenceManager::RemoveTriggers ()
{
  triggers.DeleteAll ();
}

size_t csEngineSequenceManager::GetTriggerCount () const
{
  return triggers.Length ();
}

iSequenceTrigger* csEngineSequenceManager::GetTrigger (size_t idx) const
{
  return triggers[idx];
}

iSequenceTrigger* csEngineSequenceManager::FindTriggerByName (
	const char* name) const
{
  size_t i;
  for (i = 0 ; i < triggers.Length () ; i++)
  {
    if (!strcmp (name, triggers[i]->QueryObject ()->GetName ()))
      return triggers[i];
  }
  return 0;
}

bool csEngineSequenceManager::FireTriggerByName (const char *name,
	bool now) const
{
  iSequenceTrigger *trig = FindTriggerByName(name);
  if (trig)
  {
    trig->ForceFire (now);
    return true;
  }
  return false;
}

csPtr<iSequenceWrapper> csEngineSequenceManager::CreateSequence (
	const char* name)
{
  csRef<iSequence> seq = seqmgr->NewSequence ();
  csSequenceWrapper* seqwrap = new csSequenceWrapper (this, seq);
  seqwrap->SetName (name);
  sequences.Push (&(seqwrap->scfiSequenceWrapper));
  return &(seqwrap->scfiSequenceWrapper);
}

void csEngineSequenceManager::RemoveSequence (iSequenceWrapper* seq)
{
  sequences.Delete (seq);
}

void csEngineSequenceManager::RemoveSequences ()
{
  sequences.DeleteAll ();
}

size_t csEngineSequenceManager::GetSequenceCount () const
{
  return sequences.Length ();
}

iSequenceWrapper* csEngineSequenceManager::GetSequence (size_t idx) const
{
  return sequences[idx];
}

iSequenceWrapper* csEngineSequenceManager::FindSequenceByName (
	const char* name) const
{
  size_t i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    if (!strcmp (name, sequences[i]->QueryObject ()->GetName ()))
      return sequences[i];
  }
  return 0;
}

bool csEngineSequenceManager::RunSequenceByName (
        const char *name,int delay) const
{
  iSequenceWrapper *seq = FindSequenceByName(name);
  if (seq)
  {
    seqmgr->RunSequence (delay, seq->GetSequence (), 0);
    return true;
  }
  return false;
}

void csEngineSequenceManager::FireTimedOperation (csTicks delta,
	csTicks duration, iSequenceTimedOperation* op, iBase* params)
{
  csTicks curtime = seqmgr->GetMainTime ();
  if (delta >= duration)
  {
    op->Do (1.0, params);
    return;	// Already done.
  }

  csTimedOperation* top = new csTimedOperation (op, params);
  top->start = curtime-delta;
  top->end = top->start + duration;

  timed_operations.Push (top);
  top->DecRef ();
}


