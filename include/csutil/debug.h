/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_UTIL_DEBUG_H__
#define __CS_UTIL_DEBUG_H__

#include "csextern.h"

// Enable the following define to have the DG_... macros.
//#define CS_USE_GRAPHDEBUG

struct iBase;
struct iObjectRegistry;

// The following versions of the defines are only available in
// debug mode. In release mode they do nothing. They expect SetupGraph()
// to be called with a valid object registry.
#if defined(CS_DEBUG) && defined(CS_USE_GRAPHDEBUG)
#define DG_ADD(obj,desc) \
  csDebuggingGraph::AddObject(0,(void*)(obj),false,__FILE__,__LINE__,desc)
#define DG_ADDI(obj,desc) \
  csDebuggingGraph::AddObject(0,(void*)(obj),true,__FILE__,__LINE__,desc)
#define DG_TYPE(obj,type) \
  csDebuggingGraph::AttachType(0,(void*)(obj),type)
#define DG_DESCRIBE0(obj,desc) \
  csDebuggingGraph::AttachDescription(0,(void*)(obj),desc)
#define DG_DESCRIBE1(obj,desc,a) \
  csDebuggingGraph::AttachDescription(0,(void*)(obj),desc,a)
#define DG_DESCRIBE2(obj,desc,a,b) \
  csDebuggingGraph::AttachDescription(0,(void*)(obj),desc,a,b)
#define DG_REM(obj) \
  csDebuggingGraph::RemoveObject(0,(void*)(obj),__FILE__,__LINE__)
#define DG_ADDCHILD(parent,child) \
  csDebuggingGraph::AddChild(0,(void*)(parent),(void*)(child))
#define DG_ADDPARENT(child,parent) \
  csDebuggingGraph::AddParent(0,(void*)(child),(void*)(parent))
#define DG_REMCHILD(parent,child) \
  csDebuggingGraph::RemoveChild(0,(void*)(parent),(void*)(child))
#define DG_REMPARENT(child,parent) \
  csDebuggingGraph::RemoveParent(0,(void*)(child),(void*)(parent))
#define DG_LINK(parent,child) \
  csDebuggingGraph::AddChild(0,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::AddParent(0,(void*)(child),(void*)(parent))
#define DG_UNLINK(parent,child) \
  csDebuggingGraph::RemoveChild(0,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::RemoveParent(0,(void*)(child),(void*)(parent))
#else
#define DG_ADD(obj,desc)
#define DG_ADDI(obj,desc)
#define DG_TYPE(obj,type)
#define DG_DESCRIBE0(obj,desc)
#define DG_DESCRIBE1(obj,desc,a)
#define DG_DESCRIBE2(obj,desc,a,b)
#define DG_REM(obj)
#define DG_ADDCHILD(parent,child)
#define DG_ADDPARENT(child,parent)
#define DG_REMCHILD(parent,child)
#define DG_REMPARENT(child,parent)
#define DG_LINK(parent,child)
#define DG_UNLINK(parent,child)
#endif

/**
 * This is a static class that helps with debugging.
 * It will register an object in the object registry that keeps
 * track of allocations in a graph. Later on you can add/remove
 * allocations from that graph.
 */
class CS_CRYSTALSPACE_EXPORT csDebuggingGraph
{
public:
  /**
   * Initialize the debugging graph.
   * Special note! In debug mode (CS_DEBUG) this function will put the
   * pointer to the object registry in iSCF::object_reg. That way we
   * can use this debugging functionality in places where the object
   * registry is not available.
   */
  static void SetupGraph (iObjectRegistry* object_reg);

  /**
   * Add a new object to the debug graph and link to its parent.
   * If 'scf' is true 'object' is an iBase.
   */
  static void AddObject (iObjectRegistry* object_reg,
  	void* object, bool scf, char* file, int linenr,
  	char* description, ...);

  /**
   * Attach a new description to an object in the graph.
   */
  static void AttachDescription (iObjectRegistry* object_reg,
  	void* object, char* description, ...) CS_GNUC_PRINTF (3, 4);

  /**
   * Attach a type to an object in the graph.
   */
  static void AttachType (iObjectRegistry* object_reg,
  	void* object, char* type);

  /**
   * Remove an object from the debug tree.
   */
  static void RemoveObject (iObjectRegistry* object_reg,
  	void* object, char* file, int linenr);

  /**
   * Add a child to an object.
   */
  static void AddChild (iObjectRegistry* object_reg,
  	void* parent, void* child);

  /**
   * Add a parent to an object.
   */
  static void AddParent (iObjectRegistry* object_reg,
  	void* child, void* parent);

  /**
   * Unlink a child from its parent.
   */
  static void RemoveChild (iObjectRegistry* object_reg,
  	void* parent, void* child);

  /**
   * Unlink a parent from its child.
   */
  static void RemoveParent (iObjectRegistry* object_reg,
  	void* child, void* parent);

  /**
   * Completely clear everything in the debug graph.
   */
  static void Clear (iObjectRegistry* object_reg);

  /**
   * Dump all the resulting graphs.
   */
  static void Dump (iObjectRegistry* object_reg);

  /**
   * Dump the graph containing the given object. You should
   * usually leave reset_mark alone. That's for internal use.
   */
  static void Dump (iObjectRegistry* object_reg, void* object,
  	bool reset_mark = true);
};

#endif //__CS_UTIL_DEBUG_H__

