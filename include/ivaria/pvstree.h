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

#ifndef __CS_IVARIA_PVSTREE_H__
#define __CS_IVARIA_PVSTREE_H__

#include "csutil/scf.h"

class csBox3;
struct iDataBuffer;
struct iStaticPVSTree;

SCF_VERSION (iPVSCuller, 0, 0, 1);

/**
 * This interface is implemented by the PVS visibility culler.
 * It allows someone to access the PVS data for writing.
 */
struct iPVSCuller : public iBase
{
  /**
   * Get PVS tree.
   */
  virtual iStaticPVSTree* GetPVSTree () = 0;
};

SCF_VERSION (iStaticPVSTree, 0, 0, 1);

/**
 * This interface allows someone to write the PVS data for
 * the PVS visibility culler.
 */
struct iStaticPVSTree : public iBase
{
  /**
   * Setup a PVS tree with the given bounding box. This is mostly
   * used in situations where you want to setup a PVS culler at
   * runtime. Normally this information is present in the XML parameters
   * for the PVS culler.
   */
  virtual void SetBoundingBox (const csBox3& bbox) = 0;

  /**
   * Get the bounding box for the PVS tree.
   */
  virtual const csBox3& GetBoundingBox () const = 0;

  /**
   * Return true if the bounding box of the PVS is set. False
   * otherwise. The PVScalc tool will use that information to know
   * if it should manually calculate the bounding box.
   */
  virtual bool IsBoundingBoxSet () const = 0;

  /**
   * After building the KDtree this function must be called to make
   * sure all node bounding boxes are ok.
   */
  virtual void UpdateBoundingBoxes () = 0;

  /**
   * Clear the PVS information. This must be called if you want
   * to recreate the PVS.
   */
  virtual void Clear () = 0;

  /**
   * Create a root node for the tree.
   * The returned void* is the representation of that root node.
   * Calling this function automatically calls Clear().
   */
  virtual void* CreateRootNode () = 0;

  /**
   * Return the root node.
   */
  virtual void* GetRootNode () = 0;

  /**
   * Split a give node along the given axis (x=0, y=1, z=2) and
   * at the specified position along that axis. Returns two new children
   * in 'child1' and 'child2'. If the node is already split the previous
   * children will be destroyed.
   */
  virtual void SplitNode (void* parent, int axis, float where,
  	void*& child1, void*& child2) = 0;

  /**
   * Return the first child of a node.
   */
  virtual void* GetFirstChild (void* parent) const = 0;

  /**
   * Return the second child of a node.
   */
  virtual void* GetSecondChild (void* parent) const = 0;

  /**
   * Get the box for the given node.
   */
  virtual const csBox3& GetNodeBBox (void* node) const = 0;

  /**
   * Return the axis and split position of a node.
   */
  virtual void GetAxisAndPosition (
  	void* node, int& axis, float& where) const = 0;

  /**
   * Mark node 'target' as being invisible from node 'source'.
   */
  virtual void MarkInvisible (void* source, void* target) = 0;

  /**
   * Write out the PVS information to the cache. Returns false
   * on failure.
   */
  virtual bool WriteOut () = 0;
};

#endif // __CS_IVARIA_PVSTREE_H__

