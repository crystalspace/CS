/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_REGISTRY_H__
#define __AWS_REGISTRY_H__

#include <map>
#include "object.h"
#include "csutil/scfstr.h"

/** Namespace which includes aws specific code. */
namespace aws
{
  /** Maintains a name to value mapping. */
  class registry : public csRefCount
  {
    /** The type of map that we use here. */
    typedef std::map<scfString, autom::keeper> value_type;

    /** The registry. */
    value_type reg;

    /** The type for the list of registries in the child category map. */
    typedef std::vector<csRef<registry> > child_list_type;

    /** The type for the list of child registries that we may use. */
    typedef std::map<scfString, child_list_type> child_map_type;

    /** The list of children. */
    child_map_type children;

    /** The parent, if there is one. */
    csRef<registry> parent;

    /** The name of this registry. */
    scfString rname;

  public:
    registry (const scfString &_name) : rname(_name) {}
    virtual ~registry() {}

    /** Gets the name of this registry. */
    const scfString &Name() { return rname; }

    /** Sets the parent of this registry. */
    void setParent (csRef<registry> _parent) { parent=_parent; }
    
    /** Adds a child registry under a certain category.  
     * We can lookup children given those categories. 
     */
    void addChild (const scfString &category, csRef<registry> _child);

    /** Finds the child registry with the given name in the given category. */
    csRef<registry> findChild (const scfString &category, const scfString &name);
    
    /** Finds a named value and returns a keeper to it. */
    bool findValue (const scfString &name, autom::keeper &k);

    /** Inserts a new value into this registry. */
    void insert (const scfString &name, const autom::keeper &k)
    {
      reg.insert(std::make_pair (name, k));
    }

    /** Clears out this registry. */
    void clear()
    {
      reg.clear();
    }
  };


} // end namespace

#endif
