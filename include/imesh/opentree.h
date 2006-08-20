/*
    Copyright (C) 2006 by Christoph "Fossi" Mewes

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

#ifndef __CS_IMESH_OPENTREE_H__
#define __CS_IMESH_OPENTREE_H__

/**\file
 * Opentree mesh object
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

class csVector3;
class csVector2;
class csColor;

struct csTriangle;
struct iMaterialWrapper;

/**
 *  Simple OTL factory state
 */
struct iOpenTreeFactoryState : public virtual iBase
{
  SCF_INTERFACE (iOpenTreeFactoryState, 0, 0, 3);


  /**
   *  Generate the tree mesh. Can be called after the parameters
   *  have been set. If this is not called explicitly, the tree 
   *  will be generated when the first instance is requested.
   */
  virtual void GenerateTree () = 0;

  /**
   *  Set the float treedata parameters.
   *  Use level = -1 for general parameters
   *  Supported:
   *   General: RatioPower, LobeDepth, BaseSize, AttractionUp,
   *            LeafScaleX, Scale, ScaleV, Ratio, LeafQuality,
   *            Flare, LeafScale, LeafBend, PruneWidth,
   *            PruneWidthPeak, PruneRatio, PruePowerLow and 
   *            PrunePowerHigh
   *   Levels:  Scale, ScaleV, BaseSplits, BranchDist, DownAngle
   *            DownAngleV, Rotate, RotateV, Length, LengthV,
   *            Taper, SegSplits, SplitAngle, SplitAngleV, Curve,
   *            CurveBack, CurveV
   */
  virtual bool SetParam (char level, csStringID name, float value) = 0;

  /**
   *  Set the int treedata parameters.
   *  Use level = -1 for general parameters
   *  Supported:
   *   General: Leaves, Shape, Lobes, Levels
   *   Levels:  LevelNumber, Branches, CurveRes
   */
  virtual bool SetParam (char level, csStringID name, int value) = 0;

  /**
   *  Set the string treedata parameters.
   *  Supported: currently none
   */
  virtual bool SetParam (char level, csStringID name, const char* value) = 0;
};

/**
 *   OTL Mesh state
 */
struct iOpenTreeState : public virtual iBase
{
  SCF_INTERFACE (iOpenTreeState, 0, 0, 1);

  /**
   *  Set the material for the given level
   */
  virtual bool SetMaterialWrapper (char level, iMaterialWrapper* mat) = 0;

  /**
   *  Get the material for the given level
   */
  virtual iMaterialWrapper* GetMaterialWrapper (char level) = 0;
};

/** @} */

#endif // __CS_IMESH_OPENTREE_H__

