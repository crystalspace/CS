/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_MESHLOD_H__
#define __CS_MESHLOD_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refcount.h"
#include "iengine/lod.h"
#include "iengine/mesh.h"
#include "iengine/sharevar.h"

struct iMeshWrapper;
struct iMeshFactoryWrapper;

/**
 * A listener to listen to the variables.
 */
class csLODListener : public iSharedVariableListener
{
private:
  float* variable;
public:
  SCF_DECLARE_IBASE;
  csLODListener (float* variable)
  {
    SCF_CONSTRUCT_IBASE (0);
    csLODListener::variable = variable;
  }

  virtual ~csLODListener() { SCF_DESTRUCT_IBASE(); }

  virtual void VariableChanged (iSharedVariable* var)
  {
    *variable = var->Get ();
  }
};

/**
 * This class is used to represent the static lod levels of a
 * hierarchical mesh.
 */
class csStaticLODMesh : public iLODControl
{
private:
  /// All static lod levels.
  csArray<csArray<iMeshWrapper*> > meshes_for_lod;

  /// Function for lod.
  float lod_m, lod_a;
  /// Or using variables.
  csRef<iSharedVariable> lod_varm;
  csRef<iSharedVariable> lod_vara;
  csRef<csLODListener> lod_varm_listener;
  csRef<csLODListener> lod_vara_listener;

  void ClearLODListeners ();

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csStaticLODMesh ();
  virtual ~csStaticLODMesh ();

  virtual void SetLOD (float m, float a);
  virtual void GetLOD (float& m, float& a) const;
  virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara);
  virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara) const
  {
    varm = lod_varm;
    vara = lod_vara;
  }
  virtual int GetLODPolygonCount (float lod) const;

  float GetLODValue (float distance) const
  {
    return lod_m * distance + lod_a;
  }

  /// Get the mesh array for the numerical lod.
  csArray<iMeshWrapper*>& GetMeshesForLOD (int lod)
  {
    if (lod >= (int)meshes_for_lod.Length ())
    {
      meshes_for_lod.SetLength (lod+1);
    }
    return meshes_for_lod[lod];
  }

  /// Get the mesh array for a lod between 0 and 1.
  csArray<iMeshWrapper*>& GetMeshesForLOD (float lod)
  {
    int l = (int)meshes_for_lod.Length ();
    int idx = int (lod * l);
    if (idx < 0) idx = 0;
    else if (idx >= l) idx = l-1;
    return meshes_for_lod[idx];
  }

  /// Get number of lod levels we have.
  int GetLODCount ()
  {
    return (int)meshes_for_lod.Length ();
  }
};

/**
 * This class is used to represent the static lod levels of a
 * hierarchical mesh factory. It is used as a template to create
 * a csStaticLODMesh instance.
 */
class csStaticLODFactoryMesh : public iLODControl
{
private:
  /// All static lod levels.
  csArray<csArray<iMeshFactoryWrapper*> > meshes_for_lod;

  /// Function for lod.
  float lod_m, lod_a;
  /// Or using variables.
  csRef<iSharedVariable> lod_varm;
  csRef<iSharedVariable> lod_vara;
  csRef<csLODListener> lod_varm_listener;
  csRef<csLODListener> lod_vara_listener;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csStaticLODFactoryMesh ();
  virtual ~csStaticLODFactoryMesh ();

  virtual void SetLOD (float m, float a);
  virtual void GetLOD (float& m, float& a) const;
  virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara);
  virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara) const
  {
    varm = lod_varm;
    vara = lod_vara;
  }
  virtual int GetLODPolygonCount (float lod) const { return 0; }

  /// Get the mesh array for the numerical lod.
  csArray<iMeshFactoryWrapper*>& GetMeshesForLOD (int lod)
  {
    if (lod >= (int)meshes_for_lod.Length ())
    {
      meshes_for_lod.SetLength (lod+1);
    }
    return meshes_for_lod[lod];
  }

  /// Get number of lod levels we have.
  int GetLODCount ()
  {
    return (int)meshes_for_lod.Length ();
  }
};

#endif // __CS_MESHLOD_H__

