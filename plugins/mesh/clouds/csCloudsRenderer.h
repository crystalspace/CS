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
#include <ivideo/texture.h>
#include <ivideo/txtmgr.h>
#include <iutil/objreg.h>
#include <ivideo/graph3d.h>

//Cloud-Renderer class
class csCloudsRenderer : public scfImplementation1<csCloudsRenderer, iCloudsRenderer>
{
private:
  iObjectRegistry*              m_pObjectRegistry;
  csVector3                     m_vLightDir;
  csVector3                     m_vPosition;
  float                         m_fGridScale;

  //2D-Impostor texture
  csRef<iTextureHandle>         m_pImpostor;
  //Condensed water mixing ratios texture
  bool                          m_bNewOLVTexture;
  csRef<csImageVolumeMaker>     m_pQcTexture;
  //OLV texture
  UINT                          m_iOLVTexWidth;
  UINT                          m_iOLVTexHeight;
  UINT                          m_iOLVTexDepth;
  csRef<iTextureHandle>         m_pOLVTexture;

  //OLV coord system
  csVector3                     m_vXAxis;
  csVector3                     m_vYAxis;
  csVector3                     m_vZAxis;
  csMatrix3                     m_mOLVRotation;
  csMatrix3                     m_mInvOLVRotation;
  csVector3                     m_avBaseSlice[4];    //This is the slice which is farthest away from the lightsource
  csOrthoTransform              m_mOLVCameraMatrix;
  CS::Math::Matrix4             m_mOLVProjectionMatrix;

  //Impostor properties
  csVector3                     m_vImpostorDirection;
  float                         m_fImpostorValidityAngleCos;

  //=======================================================//
  //Helpermethods

  //Returns a parallelprojection matrix
  inline const CS::Math::Matrix4 ParallelProjection(const float fWidth, const float fHeight, const float fNear, const float fFar)
  {
    const float fValue = 1.f / (fFar - fNear);
    return CS::Math::Matrix4( 2.f / fWidth, 0.0f,           0.0f,             0.0f,
	                            0.0f,         2.f / fHeight,  0.0f,             0.0f,
	                            0.0f,         0.0f,           fValue,           0.0f,
                              0.0f,         0.0f,           -fValue * fNear,  1.0f);
  }

  //Returns a cameramatrix
  inline const csOrthoTransform CameraMatrix(const csVector3& vPos, const csVector3& vDir, const csVector3& vUp)
  {
    const csVector3 vRef   = vUp / vUp.Norm();
    const csVector3 vZAxis = vDir / vDir.Norm();
    const csVector3 vXAxis = vRef % vZAxis;
    const csVector3 vYAxis = vZAxis % vXAxis;

    const csMatrix3 mRotation = csMatrix3(vXAxis.x, vYAxis.x, vZAxis.x,
                                          vXAxis.y, vYAxis.y, vZAxis.y,
                                          vXAxis.z, vYAxis.z, vZAxis.z);
    return csOrthoTransform(mRotation, vPos);
  }

  /**
  Checks if the impostor is still valid. Means if the eyedirection changed too much, or if there is
  a new OLV texture
  */
  inline const bool ImpostorStillValid(const csVector3& vCameraPosition)
  {
    if(m_bNewOLVTexture)
    {
      m_bNewOLVTexture = false;
      return false;
    }
    else
    {
      csVector3 vCurrDir = m_vPosition - vCameraPosition;
      vCurrDir.Normalize();
      if(1.f - (m_vImpostorDirection * vCurrDir) < m_fImpostorValidityAngleCos) return true;
      else return false;
    }
  }

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
    SetImpostorValidityAngle(0.087266f);                    // equal to 5°
  }
public:
  csCloudsRenderer(iBase* pParent) : scfImplementationType(this, pParent), m_bNewOLVTexture(false)
  {
    SetStandardValues();
  }
  ~csCloudsRenderer()
  {
    m_pQcTexture.Invalidate();
  }

  //Own setter (not contained in interface) (called from csClouds)
  inline void SetObjectRegistry(iObjectRegistry* pObjectReg) {m_pObjectRegistry = pObjectReg;}

  //Getter
  virtual inline const UINT GetOLVSliceCount() const {return m_iOLVTexDepth;}
  virtual inline const UINT GetOLVWidth() const {return m_iOLVTexWidth;}
  virtual inline const UINT GetOLVHeight() const {return m_iOLVTexHeight;}
  virtual inline const CS::Math::Matrix4 GetOLVProjectionMatrix() const {return m_mOLVProjectionMatrix;}
  virtual inline const csOrthoTransform GetOLVCameraMatrix() const {return m_mOLVCameraMatrix;}
  virtual inline iTextureHandle* GetOLVTexture() const {return m_pOLVTexture;}

  /**
  Setter for the user, to control every major aspect of cloud rendering.
  No rotation may be set for the cloud volume. This simplifies some calculations, and
  isn't really needed as feature.
  */
  virtual inline void SetGridScale(const float dx) {m_fGridScale = dx;}
  virtual inline void SetCloudPosition(const csVector3& vPosition) {m_vPosition = vPosition;}
  virtual inline void SetLightDirection(const csVector3& vLightDir) {m_vLightDir = vLightDir;}
  virtual inline void SetImpostorValidityAngle(const float fAngle) {m_fImpostorValidityAngleCos = cosf(fAngle);}


  /**
  This method is called everytime when the dynamic part completed one timestep.
  It renders the CloudwaterMixingRatioField viewed from lightdirection --> OLV
  */
  const bool RenderOLV(const csRef<csField3<float> >& rCondWaterMixingRatios);

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
