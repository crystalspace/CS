/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_SPRBUILD_H__
#define __CS_SPRBUILD_H__

/**\file
 * Generic sprite builder interface. 
 */

#include "csextern.h"

#include "csutil/csstring.h"
#include "csutil/ref.h"

class csVector3;
class csVector2;
struct iModelDataMaterial;
struct iModelDataObject;
struct iSprite3DFactoryState;
struct iSpriteFrame;
struct iSpriteAction;
struct iDataBuffer;

/**
 * This is a generic sprite builder interface. It takes a model data object
 * and creates a 3d sprite from it. However, this class does not contain
 * the part that actually creates the sprite. This is done in subclasses.
 */
class CS_CRYSTALSPACE_EXPORT csSpriteBuilder
{
public:
  virtual ~csSpriteBuilder () { }

  // begin the build process
  virtual void Begin () = 0;
  // finish the build process
  virtual void Finish () = 0;
  // store a triangle
  virtual void StoreTriangle (int a, int b, int c) = 0;
  // store the material
  virtual void StoreMaterial (iModelDataMaterial *mat) = 0;
  // store the frame information. Returns the number of existing frames.
  virtual int StoreFrameInfo (int FrameCount, int VertexCount) = 0;
  // set mixing mode to tiling
  virtual void EnableTiling () = 0;

  // begin a frame
  virtual void BeginFrame (int Num) = 0;
  // finish a frame
  virtual void FinishFrame () = 0;
  // add a vertex
  virtual void AddVertex (const csVector3 &pos, const csVector3 &nrm,
	const csVector2 &tex) = 0;

  // begin an action
  virtual void BeginAction (const char *Name) = 0;
  // finish an action
  virtual void FinishAction () = 0;
  // store an action frame
  virtual void StoreActionFrame (int Frame, csTicks Delay,
  	float displacement) = 0;

  // Build a sprite
  bool Build (iModelDataObject *Input);
};

/// This sprite builder writes the data to a buffer
class CS_CRYSTALSPACE_EXPORT csSpriteBuilderFile : private csSpriteBuilder
{
private:
  csString Out;

  virtual void Begin ();
  virtual void Finish ();
  virtual void StoreTriangle (int a, int b, int c);
  virtual void StoreMaterial (iModelDataMaterial *mat);
  virtual int StoreFrameInfo (int FrameCount, int VertexCount);
  virtual void EnableTiling ();
  virtual void BeginFrame (int Num);
  virtual void FinishFrame ();
  virtual void AddVertex (const csVector3 &pos, const csVector3 &nrm,
	const csVector2 &tex);
  virtual void BeginAction (const char *Name);
  virtual void FinishAction ();
  virtual void StoreActionFrame (int Frame, csTicks Delay, float displacement);

public:
  csPtr<iDataBuffer> Build (iModelDataObject *Input);
};

/// This sprite builder takes a sprite factory and adds the input information
class CS_CRYSTALSPACE_EXPORT csSpriteBuilderMesh : private csSpriteBuilder
{
private:
  iSprite3DFactoryState *Out;
  iSpriteFrame *CurrentFrame;
  int CurrentFrameNum, CurrentVertexNum;
  iSpriteAction *CurrentAction;

  virtual void Begin ();
  virtual void Finish ();
  virtual void StoreTriangle (int a, int b, int c);
  virtual void StoreMaterial (iModelDataMaterial *mat);
  virtual int StoreFrameInfo (int FrameCount, int VertexCount);
  virtual void EnableTiling ();
  virtual void BeginFrame (int Num);
  virtual void FinishFrame ();
  virtual void AddVertex (const csVector3 &pos, const csVector3 &nrm,
	const csVector2 &tex);
  virtual void BeginAction (const char *Name);
  virtual void FinishAction ();
  virtual void StoreActionFrame (int Frame, csTicks Delay, float displacement);

public:
  bool Build (iModelDataObject *Input, iSprite3DFactoryState *Output);
};

#endif // __CS_SPRBUILD_H__
