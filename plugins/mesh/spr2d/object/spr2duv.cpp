/*
    Copyright (C) 2001 by Norman Kraemer

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

SCF_IMPLEMENT_IBASE (csSprite2DUVAnimationFrame)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DUVAnimationFrame)
SCF_IMPLEMENT_IBASE_END

csSprite2DUVAnimationFrame::csSprite2DUVAnimationFrame (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  duration = 0;
  name = 0;
}

csSprite2DUVAnimationFrame::~csSprite2DUVAnimationFrame ()
{
  delete [] name;
  SCF_DESTRUCT_IBASE ();
}

void csSprite2DUVAnimationFrame::SetName (const char *name)
{
  if (name)
  {
    delete [] this->name;
    this->name = csStrNew (name);
  }
}

const char *csSprite2DUVAnimationFrame::GetName () const
{
  return name;
}

csVector2 &csSprite2DUVAnimationFrame::GetUVCoo (int idx)
{
  return vCoo[idx];
}

const csVector2 *csSprite2DUVAnimationFrame::GetUVCoo ()
{
  return vCoo.GetArray ();
}

int csSprite2DUVAnimationFrame::GetUVCount ()
{
  return (int)vCoo.Length ();
}

void csSprite2DUVAnimationFrame::SetUV (int idx, float u, float v)
{
  csVector2 uv (u, v);

  if (idx == -1 || (size_t)idx >= vCoo.Length ())
    vCoo.Push (uv);
  else
    vCoo[MAX (0, idx)] = uv;
}

void csSprite2DUVAnimationFrame::SetFrameData (const char *name,
	int duration, int num, float *uv)
{
  SetName (name);
  SetDuration (duration);
  vCoo.SetLength (num);
  memcpy (vCoo.GetArray (), uv, 2*num*sizeof(float));
}

void csSprite2DUVAnimationFrame::RemoveUV (int idx)
{
  vCoo.DeleteIndex (idx);
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
SCF_IMPLEMENT_IBASE (csSprite2DUVAnimation)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DUVAnimation)
SCF_IMPLEMENT_IBASE_END

csSprite2DUVAnimation::csSprite2DUVAnimation (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  name = 0;
}

csSprite2DUVAnimation::~csSprite2DUVAnimation ()
{
  delete [] name;
  SCF_DESTRUCT_IBASE ();
}

void csSprite2DUVAnimation::SetName (const char *name)
{
  if (name)
  {
    delete [] this->name;
    this->name = csStrNew (name);
  }
}

const char *csSprite2DUVAnimation::GetName () const
{
  return name;
}

int csSprite2DUVAnimation::GetFrameCount ()
{
  return (int)vFrames.Length ();
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::GetFrame (int idx)
{
  return (iSprite2DUVAnimationFrame *)vFrames.Get (idx);
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::GetFrame (const char *name)
{
  int idx = (int)vFrames.FindKey (vFrames.KeyCmp(name));
  return (iSprite2DUVAnimationFrame *)(idx != -1 ? vFrames.Get (idx) : 0);
}

iSprite2DUVAnimationFrame *csSprite2DUVAnimation::CreateFrame (int idx)
{
  csSprite2DUVAnimationFrame *p = new csSprite2DUVAnimationFrame (this);
  if (idx == -1 || (size_t)idx >= vFrames.Length ())
    vFrames.Push (p);
  else
    vFrames.Insert (MAX (0, idx), p);
  return (iSprite2DUVAnimationFrame *)p;
}

void csSprite2DUVAnimation::MoveFrame (int frame, int idx)
{
  csSprite2DUVAnimationFrame* p = vFrames.Get (frame);
  if (idx == -1 || (size_t)idx >= vFrames.Length ())
    vFrames.Push (p);
  else
    vFrames.Insert (MAX (0, idx), p);

  vFrames.DeleteIndex (frame + (idx <= frame ? 1 : 0));
}

void csSprite2DUVAnimation::RemoveFrame (int idx)
{
  delete (iSprite2DUVAnimationFrame *)vFrames.Get (idx);
  vFrames.DeleteIndex (idx);
}


