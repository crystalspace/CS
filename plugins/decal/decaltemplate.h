/*
    Copyright (C) 2006 by Andrew Robberts

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

#ifndef __CS_DECAL_TEMPLATE_H__
#define __CS_DECAL_TEMPLATE_H__

#include "iengine/material.h"
#include "ivaria/decal.h"
#include "csutil/scf_implementation.h"
#include "csgeom/vector2.h"
#include "csutil/cscolor.h"

// Controls the default value of hasClipping
#define CS_DECAL_CLIP_DECAL

class csDecalTemplate : public scfImplementation1<csDecalTemplate,
  iDecalTemplate>
{
private:
  float timeToLive;
  csRef<iMaterialWrapper> material;
  CS::Graphics::RenderPriority renderPriority;
  csZBufMode zBufMode;
  float polygonNormalThreshold;
  float decalOffset;
  bool hasClipping;
  bool hasTopClip;
  float topClipScale;
  bool hasBottomClip;
  float bottomClipScale;
  csVector2 minTexCoord;
  csVector2 maxTexCoord;
  uint mixMode;
  float perpendicularFaceThreshold;
  float perpendicularFaceOffset;
  csColor4 mainColor;
  csColor4 topColor;
  csColor4 bottomColor;

public:

  csDecalTemplate ();
  csDecalTemplate (iBase* parent);
  virtual ~csDecalTemplate ();

  virtual float GetTimeToLive () const;
  virtual iMaterialWrapper* GetMaterialWrapper ();
  virtual CS::Graphics::RenderPriority GetRenderPriority () const;
  virtual csZBufMode GetZBufMode () const;
  virtual float GetPolygonNormalThreshold () const;
  virtual float GetDecalOffset () const;
  virtual bool HasClipping () const;
  virtual bool HasTopClipping () const;
  virtual float GetTopClippingScale () const;
  virtual bool HasBottomClipping () const;
  virtual float GetBottomClippingScale () const;
  virtual const csVector2& GetMinTexCoord () const;
  virtual const csVector2& GetMaxTexCoord() const;
  virtual const uint GetMixMode () const;
  virtual float GetPerpendicularFaceThreshold () const;
  virtual float GetPerpendicularFaceOffset () const;
  virtual const csColor4& GetMainColor () const;
  virtual const csColor4& GetTopColor () const;
  virtual const csColor4& GetBottomColor () const;

  virtual void SetTimeToLive (float timeToLive);
  virtual void SetMaterialWrapper (iMaterialWrapper* material);
  virtual void SetRenderPriority (CS::Graphics::RenderPriority renderPriority);
  virtual void SetZBufMode (csZBufMode mode);
  virtual void SetPolygonNormalThreshold (float polygonNormalThreshold);
  virtual void SetDecalOffset (float decalOffset);
  virtual void SetClipping (bool enabled);
  virtual void SetTopClipping (bool enabled, float topPlaneScale);
  virtual void SetBottomClipping (bool enabled, float bottomPlaneScale);
  virtual void SetTexCoords (const csVector2& min, const csVector2& max);
  virtual void SetMixMode (uint mixMode);
  virtual void SetPerpendicularFaceThreshold (float threshold);
  virtual void SetPerpendicularFaceOffset (float offset);
  virtual void SetMainColor (const csColor4& color);
  virtual void SetTopColor (const csColor4& color);
  virtual void SetBottomColor (const csColor4& color);
};

#endif // __CS_DECAL_TEMPLATE_H__
