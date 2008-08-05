/*
Copyright (C) 2008 by Julian Mautner

This application is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This application is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this application; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSCLOUDRENDERER_PLUGIN_H__
#define __CSCLOUDRENDERER_PLUGIN_H__

#include "imesh/clouds.h"
#include "csCloudsUtils.h"

//#include <csgeom/matrix4.h>

//Cloud-Renderer class
class csCloudsRenderer : public scfImplementation1<csCloudsRenderer, iCloudsRenderer>
{
private:
  csVector3                     m_vLightDir;
  CS::Math::Matrix4             m_mTransform;
  float                         m_fGridScale;

  void SetStandardValues()
  {
    SetGridScale(1.f);
    //Identity!p
    SetTransformationMatrix(CS::Math::Matrix4(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f));
    SetLightDirection(csVector3(0.f, -1.f, 0.f));
  }
public:
  csCloudsRenderer(iBase* pParent) : scfImplementationType(this, pParent)
  {
    SetStandardValues();
  }
  ~csCloudsRenderer()
  {
  }

  /**
  Setter for the user, to control every major aspect of cloud rendering
  */
  virtual inline void SetGridScale(const float dx) {m_fGridScale = dx;}
  virtual inline void SetTransformationMatrix(const CS::Math::Matrix4& mTransform) {m_mTransform = mTransform;}
  virtual inline void SetLightDirection(const csVector3& vLightDir) {m_vLightDir = vLightDir;}


  /**
  This method is called everytime when the dynamic part completed one timestep.
  It renders the CloudwaterMixingRatioField viewed from lightdirection --> OLV
  */
  const bool RenderOLV(const csRef<csField3<float>>& rCondWaterMixingRatios);

  /**
  This Method is called every frame. It simply renders the clouds from view direction
  using the OLV as base for computing the illumination
  */
  const bool Render();
};

#endif // __CSCLOUDRENDERER_PLUGIN_H__