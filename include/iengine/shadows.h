/*
    Crystal Space 3D engine
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

#ifndef __I_SHADOWS_H__
#define __I_SHADOWS_H__

#include "csutil/scf.h"

struct iShadowBlock;
struct iSector;
class csFrustum;

SCF_VERSION (iShadowIterator, 0, 0, 1);

/**
 * A shadow iterator allows someone to iterate over all shadows
 * in a iShadowBlock or iShadowBlockList.
 */

struct iShadowIterator : public iBase
{
  /// Reset the iterator to start again.
  virtual void Reset () = 0;
  /// Is there still an element in this iterator?
  virtual bool HasNext () = 0;
  /// Get the next shadow.
  virtual csFrustum* Next () = 0;
  /// Get the user data for the last shadow.
  virtual void* GetUserData () = 0;
  /// Return if the last shadow is relevant or not.
  virtual bool IsRelevant () = 0;
  /// Mark the last shadow as relevant.
  virtual void MarkRelevant (bool rel) = 0;
  /// Delete the last returned shadow.
  virtual void DeleteCurrent () = 0;
  /// Return the shadow list for the current element.
  virtual iShadowBlock* GetCurrentShadowBlock () = 0;
  /// Return the shadow list for the next element.
  virtual iShadowBlock* GetNextShadowBlock () = 0;
};

SCF_VERSION (iShadowBlock, 0, 0, 1);

/**
 * A block of shadows represent the shadows that are casted by
 * one iShadowCaster object.
 */
struct iShadowBlock : public iBase
{
  /// Get an iterator to iterate over all shadows in this block.
  virtual iShadowIterator* GetShadowIterator (bool reverse = false) = 0;
  /// Get a pointer to the sector in which this block was generated.
  virtual iSector* GetSector () = 0;
  /// Get the recursion level of this sector in our frustum check.
  virtual int GetRecLevel () = 0;
};

SCF_VERSION (iShadowBlockList, 0, 0, 1);

/**
 * This is a list of shadow blocks. An iShadowReceiver will get
 * such a list.
 */
struct iShadowBlockList : public iBase
{
  /// Get an iterator to iterate over all shadows in this list.
  virtual iShadowIterator* GetShadowIterator (bool reverse = false) = 0;
};

#endif // __I_SHADOWS_H__

