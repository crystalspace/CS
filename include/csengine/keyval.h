/*
    Copyright (C) 2000 by Thomas Hieber
    
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

#ifndef __KEYVAL_H__
#define __KEYVAL_H__

#include "csgeom/vector3.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "iengine/keyval.h"

class csSector;
struct iSector;

/**
 * A Key Value Pair.
 */
class csKeyValuePair : public csObject
{
public:
  /// The constructor. Requires both key and value. Data is being copied!
  csKeyValuePair (const char* Key, const char* Value);
  /// The destructor as usual
  virtual ~csKeyValuePair ();

  /// Get the key string of the pair.
  const char *GetKey () const { return GetName (); }

  /// Get the value string of the pair
  const char *GetValue () const { return m_Value; }

  /// Set the value of a key in an object.
  void SetValue (const char* value);

  DECLARE_IBASE_EXT (csObject);
  //----------------------- iKeyValuePair --------------------------
  struct KeyValuePair : public iKeyValuePair
  {
    DECLARE_EMBEDDED_IBASE (csKeyValuePair);
    virtual iObject *QueryObject() { return scfParent; }
    virtual const char *GetKey () const { return scfParent->GetKey (); }
    virtual const char *GetValue () const { return scfParent->GetValue (); }
    virtual void SetValue (const char* value) { scfParent->SetValue (value); }
  } scfiKeyValuePair;

private:
  char *m_Value;
};

/**
 * A node.
 */
class csMapNode : public csObject
{
public:
  /// The constructor. Requires the Nodes name!
  csMapNode (const char *Name);
  /// The destructor as usual
  virtual ~csMapNode ();

  ///
  void SetPosition (const csVector3& pos) { m_Position = pos; }
  ///
  const csVector3& GetPosition () const { return m_Position; }

  ///
  void SetSector (csSector *pSector) { m_pSector = pSector; }
  ///
  csSector *GetSector () const { return m_pSector; }

  /// Get a node with the given name and a given classname. (shortcut)
  static iMapNode *GetNode (iSector *pSector, const char *name,
    const char *classname = NULL);

  DECLARE_IBASE_EXT (csObject);
  //----------------------- iMapNode --------------------------
  struct MapNode : public iMapNode
  {
    DECLARE_EMBEDDED_IBASE (csMapNode);
    virtual iObject *QueryObject() { return scfParent; }
    virtual void SetPosition (const csVector3& pos)
    {
      scfParent->SetPosition (pos);
    }
    virtual const csVector3& GetPosition () const
    {
      return scfParent->GetPosition ();
    }
    virtual void SetSector (iSector *pSector);
    virtual iSector *GetSector () const;
  } scfiMapNode;

private:
  csSector *m_pSector;
  csVector3 m_Position;
};

/**
 * A node iterator.
 */
class csNodeIterator
{
public:
  /**
   * The constructor. Theorectially, we could handle any csObject, but
   * that doesn't make sense for the current implementation, so we 
   * restrict it to csSector to avoid some pitfalls.<p>
   *
   * If a classname is given, search is restricted to nodes, in which
   * the key "classname" has the same value as the given classname.
   * the classname string is _not_ duplicated, so the caller is
   * responsible to take care, that the string is available while
   * the Iterator is alive.
   */
  csNodeIterator (iSector *pSector, const char *classname = NULL);

  /// The destructor as usual
  ~csNodeIterator ();

  /// Reuse the iterator for an other search
  void Reset (iSector *pSector, const char *classname = NULL);
  /// Get the object we are pointing at
  iMapNode *GetNode ();
  /// Move forward
  void Next ();
  /// Check if there are other nodes
  bool IsFinished () const;

protected:
  /// Skip all nodes with wrong classname
  void SkipWrongClassname ();
  /// Step to the next node in the sector, ignoring its classname
  void NextNode ();

  iObjectIterator *Iterator;
  const char *Classname;
  iMapNode *CurrentNode;
};

#endif // __KEYVAL_H__
