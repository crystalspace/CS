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

#ifndef __LITMESHSEL_H__
#define __LITMESHSEL_H__

#include <stdarg.h>
#include <csutil/util.h>
#include <csutil/ref.h>
#include <csutil/refcount.h>
#include <csutil/flags.h>
#include "csutil/regexp.h"
#include <iutil/object.h>
#include <iengine/mesh.h>
#include <imesh/object.h>

/**
 * This abstract class indicates when a mesh is selected or not
 * (selected means that we will use this mesh to either cast shadows
 * or receive shadows). Subclasses of this class will control how
 * exactly the mesh is selected.
 */
class litMeshSelect : public csRefCount
{
public:
  virtual ~litMeshSelect () { }

  /**
   * Return true if this mesh should be selected.
   */
  virtual bool SelectMesh (iMeshWrapper*) = 0;
};

/**
 * Select a mesh based on name.
 */
class litMeshSelectByName
{
private:
  char* name;

public:
  litMeshSelectByName (const char* name)
  {
    litMeshSelectByName::name = csStrNew (name);
  }
  virtual ~litMeshSelectByName ()
  {
    delete[] name;
  }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    return strcmp (name, mesh->QueryObject ()->GetName ()) == 0;
  }
};

/**
 * Select a mesh based on regexp on name.
 */
class litMeshSelectByNameRE
{
private:
  csRegExpMatcher matcher;

public:
  litMeshSelectByNameRE (const char* name_re) : matcher (name_re) { }
  virtual ~litMeshSelectByNameRE () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    return matcher.Match (mesh->QueryObject ()->GetName ()) == NoError;
  }
};

/**
 * Select a mesh based on mesh wrapper flags.
 */
class litMeshSelectByMWFlags
{
private:
  uint32 value, mask;

public:
  litMeshSelectByMWFlags (uint32 value, uint32 mask)
  {
    litMeshSelectByMWFlags::value = value;
    litMeshSelectByMWFlags::mask = mask;
  }
  virtual ~litMeshSelectByMWFlags () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    return (mesh->GetFlags ().Get () & mask) == value;
  }
};

/**
 * Select a mesh based on mesh object flags.
 */
class litMeshSelectByMOFlags
{
private:
  uint32 value, mask;

public:
  litMeshSelectByMOFlags (uint32 value, uint32 mask)
  {
    litMeshSelectByMOFlags::value = value;
    litMeshSelectByMOFlags::mask = mask;
  }
  virtual ~litMeshSelectByMOFlags () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    return (mesh->GetMeshObject ()->GetFlags ().Get () & mask) == value;
  }
};

/**
 * Select a mesh based on mesh type.
 */
class litMeshSelectByType
{
private:
  char* type;

public:
  litMeshSelectByType (const char* type)
  {
    litMeshSelectByType::type = csStrNew (type);
  }
  virtual ~litMeshSelectByType ()
  {
    delete[] type;
  }
  virtual bool SelectMesh (iMeshWrapper* mesh);
};

/**
 * Logical and of two other mesh selectors.
 */
class litMeshSelectAnd
{
private:
  csRef<litMeshSelect> a;
  csRef<litMeshSelect> b;

public:
  litMeshSelectAnd (litMeshSelect* a, litMeshSelect* b)
  {
    litMeshSelectAnd::a = a;
    litMeshSelectAnd::b = b;
  }
  virtual ~litMeshSelectAnd () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    bool rc = a->SelectMesh (mesh);
    if (!rc) return false;
    return b->SelectMesh (mesh);
  }
};

/**
 * Logical or of two other mesh selectors.
 */
class litMeshSelectOr
{
private:
  csRef<litMeshSelect> a;
  csRef<litMeshSelect> b;

public:
  litMeshSelectOr (litMeshSelect* a, litMeshSelect* b)
  {
    litMeshSelectOr::a = a;
    litMeshSelectOr::b = b;
  }
  virtual ~litMeshSelectOr () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    bool rc = a->SelectMesh (mesh);
    if (rc) return true;
    return b->SelectMesh (mesh);
  }
};

/**
 * Logical not of other mesh selector.
 */
class litMeshSelectNot
{
private:
  csRef<litMeshSelect> a;

public:
  litMeshSelectNot (litMeshSelect* a)
  {
    litMeshSelectNot::a = a;
  }
  virtual ~litMeshSelectNot () { }
  virtual bool SelectMesh (iMeshWrapper* mesh)
  {
    return !a->SelectMesh (mesh);
  }
};

/**
 * Select everything.
 */
class litMeshSelectAll
{
public:
  litMeshSelectAll () { }
  virtual ~litMeshSelectAll () { }
  virtual bool SelectMesh (iMeshWrapper*) { return true; }
};

#endif // __LITMESHSEL_H__

