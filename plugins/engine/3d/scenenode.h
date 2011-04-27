/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_SCENENODE_H__
#define __CS_SCENENODE_H__

#include "plugins/engine/3d/movable.h"
#include "iengine/scenenode.h"


/// Helper class for iSceneNode.
class csSceneNode
{
public:
  static void SetParent (iSceneNode* this_node, iSceneNode* parent,
  	CS_PLUGIN_NAMESPACE_NAME(Engine)::csMovable* this_movable)
  {
    CS_PLUGIN_NAMESPACE_NAME(Engine)::csMovable* parent_mov = this_movable->GetParent ();
    if (!parent_mov && !parent) return;
    if (parent_mov && parent_mov->GetSceneNode () == parent) return;

    if (parent_mov)
    {
      csRefArray<iSceneNode>& parent_children = parent_mov->GetChildren ();
      size_t idx = parent_children.Find (this_node);
      CS_ASSERT (idx != csArrayItemNotFound);
      parent_children.DeleteIndex (idx);
    }

    if (parent)
      this_movable->SetParent ((CS_PLUGIN_NAMESPACE_NAME(Engine)::csMovable*)(parent->GetMovable ()));
    else
      this_movable->SetParent (0);

    if (parent)
    {
      parent_mov = (CS_PLUGIN_NAMESPACE_NAME(Engine)::csMovable*)(parent->GetMovable ());
      csRefArray<iSceneNode>& parent_children = parent_mov->GetChildren ();
#ifdef CS_DEBUG
      size_t idx = parent_children.Find (parent);
      CS_ASSERT (idx == csArrayItemNotFound);
#endif
      parent_children.Push (this_node);
    }
  }
};

#endif // __CS_SCENENODE_H__
