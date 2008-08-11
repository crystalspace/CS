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

#include <csgeom/matrix3.h>
#include <csgeom/vector3.h>
#include <csgfx/imagevolumemaker.h>
#include <csgfx/imagememory.h>

//Cloud-Renderer class
class csCloudsRenderer : public scfImplementation1<csCloudsRenderer, iCloudsRenderer>
{
private:
  csVector3                     m_vLightDir;
  csVector3                     m_vPosition;
  float                         m_fGridScale;

  //OLV 3D-Texture
  csImageVolumeMaker*           m_OLVTexture;

  //OLV coord system
  csVector3                     m_vXAxis;
  csVector3                     m_vYAxis;
  csVector3                     m_vZAxis;
  csMatrix3                     m_mOLVRotation;
  csMatrix3                     m_mInvOLVRotation;
  csVector3                     m_avBaseSlice[4];    //This is the slice which is farthest away from the lightsource


  //=======================================================//
  //Helpermethods

  inline const csVector3 VectorMin(const csVector3& a, const csVector3& b)
  {
    return csVector3(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);
  }
  inline const csVector3 VectorMax(const csVector3& a, const csVector3& b)
  {
    return csVector3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
  }

  void SetStandardValues()
  {
    SetGridScale(1.f);
    SetCloudPosition(csVector3(0.f, 0.f, 0.f));
    SetLightDirection(csVector3(0.f, -1.f, 0.f));
  }
public:
  csCloudsRenderer(iBase* pParent) : scfImplementationType(this, pParent), m_OLVTexture(NULL)
  {
    SetStandardValues();
  }
  ~csCloudsRenderer()
  {
  }

  /**
  Setter for the user, to control every major aspect of cloud rendering.
  No rotation may be set for the cloud volume. This simplifies some calculations, and
  isn't really needed as feature.
  */
  virtual inline void SetGridScale(const float dx) {m_fGridScale = dx;}
  virtual inline void SetCloudPosition(const csVector3& vPosition) {m_vPosition = vPosition;}
  virtual inline void SetLightDirection(const csVector3& vLightDir) {m_vLightDir = vLightDir;}


  /**
  This method is called everytime when the dynamic part completed one timestep.
  It renders the CloudwaterMixingRatioField viewed from lightdirection --> OLV
  */
  const bool RenderOLV(const csRef<csField3<float>>& rCondWaterMixingRatios);

  /**
  This Method is called every frame. It simply renders the clouds impostor from view direction
  */
  const bool Render(const csVector3& vCameraPosition);

  /**
  This Method is called every time, when the impostor is needed to be updated. Means, everytime
  when the looking-at-angle has changed too much, or when a new OLV was made
  */
  const bool CreateImpostor(const csVector3& vCameraPosition);
};

#endif // __CSCLOUDRENDERER_PLUGIN_H__