/*
    Copyright (C) 2001 by Norman Krämer
  
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

#ifndef _SPR2D_UV_ANIMATION_
#define _SPR2D_UV_ANIMATION_

#include "imesh/sprite2d.h"
#include "csutil/csvector.h"
#include "csgeom/vector2.h"

class csSprite2DUVAnimationFrame : public iSprite2DUVAnimationFrame
{
 protected:
  char *name;
  int duration;
  TYPEDEF_GROWING_ARRAY(UVArray, csVector2) vCoo;

 public:
  DECLARE_IBASE;


  csSprite2DUVAnimationFrame (iBase*);
  virtual ~csSprite2DUVAnimationFrame ();

  /**
   * Give this frame a name.
   */
  virtual void SetName (const char *name);

  /**
   * Return the name of this frame.
   */
  virtual const char *GetName ();

  /**
   * Get the u,v coordinates of the <idx>-th vertex
   */
  virtual csVector2 &GetUVCoo (int idx);

  /**
   * Get all u,v coordinates
   */
  virtual const csVector2 *GetUVCoo ();

  /**
   * Get the number of (u,v) coordinates
   */
  virtual int GetUVCount ();

  /**
   * Set the (u,v) coordinate of the <idx>-th coo. Set idx to -1 to append it
   */
  virtual void SetUV (int idx, float u, float v);

  /**
   * Set all (u,v) coordinates and the name and duration
   */
  virtual void SetFrameData (const char *name, int duration, int num, float *uv);

  /**
   * Remove the <idx>-th coordinate.
   */
  virtual void RemoveUV (int idx);

  /**
   * Return the duration of this frame.
   */
  virtual int GetDuration ();

  /**
   * Set the duration of this frame.
   */
  virtual void SetDuration (int duration);
};

class csSprite2DUVAnimation : public iSprite2DUVAnimation
{
 protected:
  char *name;

  class frameVector : public csVector
  {
  public:
    frameVector () : csVector (8, 16){}
    virtual int CompareKey (csSome Item1, csConstSome Item2, int Mode) const
    { 
      (void)Mode;
      csSprite2DUVAnimationFrame *f1 = (csSprite2DUVAnimationFrame *)Item1;
      const char *f2 = (const char *)Item2;
      return strcmp (f1->GetName (), f2);
    }
  };

  frameVector vFrames;

 public:
  DECLARE_IBASE;

  csSprite2DUVAnimation (iBase*);
  virtual ~csSprite2DUVAnimation ();

  virtual void SetName (const char *name);

  /**
   * return the name of this sequence.
   */
  virtual const char *GetName ();

  /**
   * Retrieve the number of frames in this animation.
   */
  virtual int GetFrameCount ();

  /**
   * Get the <idx>-th frame in the animation.
   * Set idx to -1 to get the current to be played.
   */
  virtual iSprite2DUVAnimationFrame *GetFrame (int idx);

  /**
   * Get the frame <name> in the animation.
   */
  virtual iSprite2DUVAnimationFrame *GetFrame (const char *name);

  /**
   * Create a new frame that will be inserted before the <idx> frame.
   * Set <idx> to -1 to append the frame to the sequence.
   */
  virtual iSprite2DUVAnimationFrame *CreateFrame (int idx);

  /**
   * Move the <frame>-th frame before the <idx>-th frame. Set idx to -1
   * to move the frame to the end of the sequence.
   */
  virtual void MoveFrame (int frame, int idx);

  /**
   * Remove the <idx>-th from the animation
   */
  virtual void RemoveFrame (int idx);

};

#endif
