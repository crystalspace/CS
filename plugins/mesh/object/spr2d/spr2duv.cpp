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

#include "cssysdef.h"
#include "spr2duv.h"
#include "csutil/util.h"

IMPLEMENT_IBASE (csSprite2DUVAnimationFrame)
  IMPLEMENTS_INTERFACE (iSprite2DUVAnimationFrame)
IMPLEMENT_IBASE_END

csSprite2DUVAnimationFrame::csSprite2DUVAnimationFrame (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
  duration = 0;
  name = NULL;
}

csSprite2DUVAnimationFrame::~csSprite2DUVAnimationFrame ()
{
  if (name) delete [] name;
}

void csSprite2DUVAnimationFrame::SetName (const char *name)
{
  if (name)
  {
    if (this->name) delete [] this->name;
    this->name = strnew (name);
  }
}

const char *csSprite2DUVAnimationFrame::GetName ()
{
  return name;
}

csVector2 &csSprite2DUVAnimationFrame::GetUVCoo (int idx)
{
  return vCoo.Get (idx);
}

const csVector2 *csSprite2DUVAnimationFrame::GetUVCoo ()
{
  return vCoo.GetArray ();
}

int csSprite2DUVAnimationFrame::GetUVCount ()
{
  return vCoo.Length ();
}

void csSprite2DUVAnimationFrame::SetUV (int idx, float u, float v)
{
  csVector2 uv (u, v);
  
  if (idx == -1 || idx >= vCoo.Length ())
    vCoo.Push (uv);
  else
    vCoo[MAX (0, idx)] = uv;
}

void csSprite2DUVAnimationFrame::SetFrameData (const char *name, int duration, int num, float *uv)
{
  SetName (name);
  SetDuration (duration);
  vCoo.SetLength (num);
  memcpy (vCoo.GetArray (), uv, 2*num*sizeof(float));
}

void csSprite2DUVAnimationFrame::RemoveUV (int idx)
{
  vCoo.Delete (idx);
}

int csSprite2DUVAnimationFrame::GetDuration ()
{
  return duration;
}

void csSprite2DUVAnimationFrame::SetDuration (int duration)
{
  this->duration = duration;
}

// ------------------- csSprite2DUVAnimation ---------
IMPLEMENT_IBASE (csSprite2DUVAnimation)
  IMPLEMENTS_INTERFACE (iSprite2DUVAnimation)
IMPLEMENT_IBASE_END

csSprite2DUVAnimation::csSprite2DUVAnimation (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
  name = NULL;
}

csSprite2DUVAnimation::~csSprite2DUVAnimation ()
{
  if (name) delete [] name;
}

void csSprite2DUVAnimation::SetName (const char *name)
{
  if (name)
  {
    if (this->name) delete [] this->name;
    this->name = strnew (name);
  }
}

const char *csSprite2DUVAnimation::GetName ()
{
  return name;
}

int csSprite2DUVAnimation::GetFrameCount ()
{
  return vFrames.Length ();
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::GetFrame (int idx)
{
  return (iSprite2DUVAnimationFrame *)vFrames.Get (idx);
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::GetFrame (const char *name)
{
  int idx = vFrames.FindKey ((csSome)name);
  return (iSprite2DUVAnimationFrame *)(idx != -1 ? vFrames.Get (idx) : NULL);
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::CreateFrame (int idx)
{
  csSprite2DUVAnimationFrame *p = new csSprite2DUVAnimationFrame (this);
  if (idx == -1 || idx >= vFrames.Length ())
    vFrames.Push (p);
  else
    vFrames.Insert (MAX (0, idx), p);
  return (iSprite2DUVAnimationFrame *)p;
}

void csSprite2DUVAnimation::MoveFrame (int frame, int idx)
{
  csSome p = vFrames.Get (frame);
  if (idx == -1 || idx >= vFrames.Length ())
    vFrames.Push (p);
  else
    vFrames.Insert (MAX (0, idx), p);
    
  vFrames.Delete (frame + (idx <= frame ? 1 : 0));
}

void csSprite2DUVAnimation::RemoveFrame (int idx)
{
  delete (iSprite2DUVAnimationFrame *)vFrames.Get (idx);  
  vFrames.Delete (idx);
}


