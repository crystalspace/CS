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

#include <string.h>
#include "cssysdef.h"
#include "csutil/databuf.h"
#include "csutil/csendian.h"
#include "pvstree.h"

//-------------------------------------------------------------------------

csStaticPVSNode::csStaticPVSNode ()
{
  child1 = child2 = 0;
}

csStaticPVSNode::~csStaticPVSNode ()
{
  delete child1;
  delete child2;
}

//-------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStaticPVSTree)
  SCF_IMPLEMENTS_INTERFACE (iStaticPVSTree)
SCF_IMPLEMENT_IBASE_END

csStaticPVSTree::csStaticPVSTree ()
{
  SCF_CONSTRUCT_IBASE (0);
  root = 0;
}

csStaticPVSTree::~csStaticPVSTree ()
{
  Clear ();
  SCF_DESTRUCT_IBASE();
}

void csStaticPVSTree::Clear ()
{
  delete root;
  root = 0;
  nodes_by_id.DeleteAll ();
}

csStaticPVSNode* csStaticPVSTree::CreateNode ()
{
  csStaticPVSNode* node = new csStaticPVSNode;
  node->id = nodes_by_id.Length ();
  nodes_by_id.Push (node);
  return node;
}

csStaticPVSNode* csStaticPVSTree::CheckOrCreateNode (uint32 id)
{
  if (id < nodes_by_id.Length () && nodes_by_id[id] != 0)
    return nodes_by_id[id];

  csStaticPVSNode* node = new csStaticPVSNode;
  node->id = id;
  if (id >= nodes_by_id.Length ())
  {
    nodes_by_id.SetLength (id+1, 0);
  }
  nodes_by_id[id] = node;
  return node;
}

void* csStaticPVSTree::CreateRootNode (const csBox3& box)
{
  Clear ();
  root_box = box;
  root = CreateNode ();
  return (void*)root;
}

void csStaticPVSTree::SplitNode (void* parent, int axis, float where,
  	void*& child1, void*& child2)
{
  csStaticPVSNode* parent_node = (csStaticPVSNode*)parent;
  delete parent_node->child1;
  delete parent_node->child2;
  parent_node->child1 = CreateNode ();
  parent_node->child2 = CreateNode ();
  child1 = (void*)(parent_node->child1);
  child2 = (void*)(parent_node->child2);
  parent_node->axis = axis;
  parent_node->where = where;
}

void csStaticPVSTree::MarkInvisible (void* source, void* target)
{
  csStaticPVSNode* source_node = (csStaticPVSNode*)source;
  csStaticPVSNode* target_node = (csStaticPVSNode*)target;
  source_node->invisible_nodes.Push (target_node);
}

size_t csStaticPVSTree::CalculateSize (csStaticPVSNode* node)
{
  if (!node) return 0;
  return 4 + // id
  	 1 + // axis
  	 4 + // where
	 4 + // number of invisible nodes
	 4 * node->invisible_nodes.Length () +
	 1 + // child 1 available
	 CalculateSize (node->child1) +
	 1 + // child 2 available
	 CalculateSize (node->child2);
}

void csStaticPVSTree::WriteOut (char*& data, csStaticPVSNode* node)
{
  if (!node) return;
  csSetLittleEndianLong (data, node->id); data += 4;
  *data++ = node->axis;
  csSetLittleEndianFloat32 (data, node->where); data += 4;
  csSetLittleEndianLong (data, node->invisible_nodes.Length ()); data += 4;
  size_t i;
  for (i = 0 ; i < node->invisible_nodes.Length () ; i++)
  {
    csSetLittleEndianLong (data, node->invisible_nodes[i]->id); data += 4;
  }
  *data++ = node->child1 ? 1 : 0;
  WriteOut (data, node->child1);
  *data++ = node->child2 ? 1 : 0;
  WriteOut (data, node->child2);
}

csPtr<iDataBuffer> csStaticPVSTree::WriteOut ()
{
  size_t total_len =
  	4 +	// marker
	6 * 4 + // root_box
	CalculateSize (root);
  csDataBuffer* buf = new csDataBuffer (total_len);

  char* data = buf->GetData ();
  *data++ = 'P';
  *data++ = 'V';
  *data++ = 'S';
  *data++ = '1';
  csSetLittleEndianFloat32 (data, root_box.MinX ()); data += 4;
  csSetLittleEndianFloat32 (data, root_box.MinY ()); data += 4;
  csSetLittleEndianFloat32 (data, root_box.MinZ ()); data += 4;
  csSetLittleEndianFloat32 (data, root_box.MaxX ()); data += 4;
  csSetLittleEndianFloat32 (data, root_box.MaxY ()); data += 4;
  csSetLittleEndianFloat32 (data, root_box.MaxZ ()); data += 4;
  WriteOut (data, root);

  return buf;
}

const char* csStaticPVSTree::ReadPVS (char*& data, csStaticPVSNode*& node)
{
  uint32 id = csGetLittleEndianLong (data); data += 4;
  node = CheckOrCreateNode (id);
  node->axis = *data++;
  node->where = csGetLittleEndianFloat32 (data); data += 4;
  size_t inv_nodes = csGetLittleEndianLong (data); data += 4;
  if (inv_nodes)
  {
    node->invisible_nodes.SetLength (inv_nodes);
    size_t i;
    for (i = 0 ; i < inv_nodes ; i++)
    {
      uint32 node_id = csGetLittleEndianLong (data); data += 4;
      node->invisible_nodes[i] = CheckOrCreateNode (node_id);
    }
  }
  char child1_present = *data++;
  if (child1_present)
  {
    const char* err = ReadPVS (data, node->child1);
    if (err) return err;
  }
  char child2_present = *data++;
  if (child2_present)
  {
    const char* err = ReadPVS (data, node->child2);
    if (err) return err;
  }
  return 0;
}

const char* csStaticPVSTree::ReadPVS (iDataBuffer* buf)
{
  Clear ();
  char* data = buf->GetData ();
  if (*data++ != 'P')
    return "File marker invalid! Probably not a PVS file!";
  if (*data++ != 'V')
    return "File marker invalid! Probably not a PVS file!";
  if (*data++ != 'S')
    return "File marker invalid! Probably not a PVS file!";
  if (*data++ != '1')
    return "File marker invalid! Could be wrong version of PVS file!";

  csVector3 minv, maxv;
  minv.x = csGetLittleEndianFloat32 (data); data += 4;
  minv.y = csGetLittleEndianFloat32 (data); data += 4;
  minv.z = csGetLittleEndianFloat32 (data); data += 4;
  maxv.x = csGetLittleEndianFloat32 (data); data += 4;
  maxv.y = csGetLittleEndianFloat32 (data); data += 4;
  maxv.z = csGetLittleEndianFloat32 (data); data += 4;
  root_box.Set (minv, maxv);

  return ReadPVS (data, root);
}

