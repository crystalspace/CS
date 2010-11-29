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

#include "cssysdef.h"
#include "decaltemplate.h"

SCF_IMPLEMENT_FACTORY (csDecalTemplate)

csDecalTemplate::csDecalTemplate ()
  : scfImplementationType (this),
    timeToLive (-1.0f),
    material (0),
    zBufMode (CS_ZBUF_TEST),
    polygonNormalThreshold (0.01f),
    decalOffset (0.05f),
    hasTopClip (true), 
    topClipScale (0.5f),
    hasBottomClip (true), 
    bottomClipScale (0.5f),
    minTexCoord (0, 0),
    maxTexCoord (1, 1),
    mixMode (CS_FX_COPY),
    perpendicularFaceThreshold (0.05f),
    perpendicularFaceOffset (0.01f),
    mainColor (1, 1, 1, 1),
    topColor (1, 1, 1, 1),
    bottomColor (1, 1, 1, 1)
{
#ifdef CS_DECAL_CLIP_DECAL
  hasClipping = true;
#else
  hasClipping = false;
#endif
}

csDecalTemplate::csDecalTemplate (iBase* parent)
  : scfImplementationType (this, parent),
    timeToLive (-1.0f),
    material (0),
    zBufMode (CS_ZBUF_TEST),
    polygonNormalThreshold (0.01f),
    decalOffset (0.05f),
    hasTopClip (true), 
    topClipScale (0.5f),
    hasBottomClip (true), 
    bottomClipScale (0.5f),
    minTexCoord (0, 0),
    maxTexCoord (1, 1),
    mixMode (CS_FX_COPY),
    perpendicularFaceThreshold (0.05f),
    perpendicularFaceOffset (0.01f),
    mainColor (1, 1, 1, 1),
    topColor (1, 1, 1, 1),
    bottomColor (1, 1, 1, 1)
{
#ifdef CS_DECAL_CLIP_DECAL
  hasClipping = true;
#else
  hasClipping = false;
#endif
}

csDecalTemplate::~csDecalTemplate ()
{
}

float csDecalTemplate::GetTimeToLive () const
{
  return timeToLive;
}

iMaterialWrapper* csDecalTemplate::GetMaterialWrapper ()
{
  return material;
}

CS::Graphics::RenderPriority csDecalTemplate::GetRenderPriority () const
{
    return renderPriority;
}

csZBufMode csDecalTemplate::GetZBufMode() const
{
  return zBufMode;
}

float csDecalTemplate::GetPolygonNormalThreshold () const
{
  return polygonNormalThreshold;
}

float csDecalTemplate::GetDecalOffset () const
{
  return decalOffset;
}

bool csDecalTemplate::HasClipping () const
{
  return hasClipping;
}

bool csDecalTemplate::HasTopClipping () const
{
  return hasTopClip;
}

float csDecalTemplate::GetTopClippingScale () const
{
  return topClipScale;
}

bool csDecalTemplate::HasBottomClipping () const
{
  return hasBottomClip;
}

float csDecalTemplate::GetBottomClippingScale () const
{
  return bottomClipScale;
}

const csVector2& csDecalTemplate::GetMinTexCoord () const
{
  return minTexCoord;
}

const csVector2& csDecalTemplate::GetMaxTexCoord () const
{
  return maxTexCoord;
}

const uint csDecalTemplate::GetMixMode () const
{
  return mixMode;
}

float csDecalTemplate::GetPerpendicularFaceThreshold () const
{
  return perpendicularFaceThreshold;
}

float csDecalTemplate::GetPerpendicularFaceOffset () const
{
  return perpendicularFaceOffset;
}

const csColor4& csDecalTemplate::GetMainColor () const
{
  return mainColor;
}

const csColor4& csDecalTemplate::GetTopColor () const
{
  return topColor;
}

const csColor4& csDecalTemplate::GetBottomColor () const
{
  return bottomColor;
}

void csDecalTemplate::SetTimeToLive (float timeToLive)
{
  this->timeToLive = timeToLive;
}

void csDecalTemplate::SetMaterialWrapper (iMaterialWrapper* material)
{
  this->material = material;
}

void csDecalTemplate::SetRenderPriority (CS::Graphics::RenderPriority renderPriority)
{
  this->renderPriority = renderPriority;
}

void csDecalTemplate::SetZBufMode (csZBufMode mode)
{
  this->zBufMode = mode;
}

void csDecalTemplate::SetPolygonNormalThreshold (float polygonNormalThreshold)
{
  this->polygonNormalThreshold = polygonNormalThreshold;
}

void csDecalTemplate::SetDecalOffset (float decalOffset)
{
  this->decalOffset = decalOffset;
}

void csDecalTemplate::SetClipping (bool enabled)
{
  hasClipping = enabled;
}

void csDecalTemplate::SetTopClipping (bool enabled, float topPlaneScale)
{
  hasTopClip = enabled;
  topClipScale = topPlaneScale;
}

void csDecalTemplate::SetBottomClipping (bool enabled, float bottomPlaneScale)
{
  hasBottomClip = enabled;
  bottomClipScale = bottomPlaneScale;
}

void csDecalTemplate::SetTexCoords (const csVector2& min, 
  const csVector2 & max)
{
  this->minTexCoord = min;
  this->maxTexCoord = max;
}

void csDecalTemplate::SetMixMode (uint mixMode)
{
  this->mixMode = mixMode;
}

void csDecalTemplate::SetPerpendicularFaceThreshold (float threshold)
{
  perpendicularFaceThreshold = threshold;
}

void csDecalTemplate::SetPerpendicularFaceOffset (float offset)
{
  perpendicularFaceOffset = offset;
}

void csDecalTemplate::SetMainColor (const csColor4& color)
{
  mainColor = color;
}

void csDecalTemplate::SetTopColor (const csColor4& color)
{
  topColor = color;
}

void csDecalTemplate::SetBottomColor (const csColor4& color)
{
  bottomColor = color;
}
