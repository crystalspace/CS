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

#include <cssysdef.h>
#include "csCloudsRenderer.h"

SCF_IMPLEMENT_FACTORY(csCloudsRenderer)

//-------------------------------------------------------//

const bool csCloudsRenderer::RenderOLV(const csRef<csField3<float>>& rCondWaterMixingRatios)
{
  //First save the mixingratios into a 3D-Texture
  m_pQcTexture.Invalidate();
  m_pQcTexture.AttachNew(new csImageVolumeMaker);
  unsigned long* pdwBuffer = new unsigned long[rCondWaterMixingRatios->GetSizeX() * rCondWaterMixingRatios->GetSizeY()];
  //Now add slice per slice
  for(UINT z = 0; z < rCondWaterMixingRatios->GetSizeZ(); ++z)
  {
    //convert mixing ratios into a color and save it into a buffer
    UINT iIndex = 0;
    for(UINT x = 0; x < rCondWaterMixingRatios->GetSizeX(); ++x)
    {
      for(UINT y = 0; y < rCondWaterMixingRatios->GetSizeY(); ++y)
      {
        const float fValue = rCondWaterMixingRatios->GetValue(x, y, z);
        const unsigned char ucA = static_cast<unsigned char>(fValue < 0.f ? 0.f : fValue > 1.f ? 1.f : fValue * 255.f);
        const unsigned char ucR = 0xFF;
        const unsigned char ucG = 0xFF;
        const unsigned char ucB = 0xFF;
        pdwBuffer[iIndex++] = static_cast<unsigned long>(ucR) | 
                              static_cast<unsigned long>(ucG << 8) | 
                              static_cast<unsigned long>(ucB << 16) | 
                              static_cast<unsigned long>(ucA << 24);
      }
    }
    //Create current slice and add it to the 3D-Texture
    csImageMemory TempImage(rCondWaterMixingRatios->GetSizeX(), rCondWaterMixingRatios->GetSizeY(), pdwBuffer, false);
    m_pQcTexture->AddImage(&TempImage);
  }
  delete[] pdwBuffer;

  //Then fit bounding-box in Lightdirection. Compute Rotationsmatrix such that vLightDir equals Z-Axis
  //Used Coord system: left-hand
  m_vZAxis = -m_vLightDir;
  m_vZAxis.Normalize();
  csVector3 vLinIndepVec;
  if(m_vZAxis != csVector3(0.f, 1.f, 0.f) && m_vZAxis != csVector3(0.f, -1.f, 0.f)) vLinIndepVec = csVector3(0.f, 1.f, 0.f);
  else vLinIndepVec = csVector3(-1.f, 0.f, 0.f);
  m_vYAxis.Cross(vLinIndepVec, m_vZAxis);
  m_vXAxis.Cross(m_vZAxis, m_vYAxis);
  //Rotationmatrix
  m_mOLVRotation  = csMatrix3(m_vXAxis.x, m_vYAxis.x, m_vZAxis.x,
                              m_vXAxis.y, m_vYAxis.y, m_vZAxis.y,
                              m_vXAxis.z, m_vYAxis.z, m_vZAxis.z);
  //For rotationmatrices is Inversion == Transpose valid!
  m_mInvOLVRotation = m_mOLVRotation;
  m_mInvOLVRotation.Transpose();

  //Compute Boundingbox for qc-volume
  const float fSizeXHalf = rCondWaterMixingRatios->GetSizeX() * m_fGridScale * 0.5f;
  const float fSizeYHalf = rCondWaterMixingRatios->GetSizeY() * m_fGridScale * 0.5f;
  const float fSizeZHalf = rCondWaterMixingRatios->GetSizeZ() * m_fGridScale * 0.5f;
  //Center of the box is the Origin
  csVector3 avQcBox[8]   = {csVector3(fSizeXHalf, -fSizeYHalf, -fSizeZHalf),    // front, bottom, right
                            csVector3(fSizeXHalf, fSizeYHalf, -fSizeZHalf),     // front, top, right
                            csVector3(-fSizeXHalf, fSizeYHalf, -fSizeZHalf),    // front, top, left
                            csVector3(-fSizeXHalf, -fSizeYHalf, -fSizeZHalf),   // front, bottom, left
                            csVector3(fSizeXHalf, -fSizeYHalf, fSizeZHalf),     // back, bottom, right
                            csVector3(fSizeXHalf, fSizeYHalf, fSizeZHalf),      // back, top, right
                            csVector3(-fSizeXHalf, fSizeYHalf, fSizeZHalf),     // back, top, left
                            csVector3(-fSizeXHalf, -fSizeYHalf, fSizeZHalf)};   // back, bottom, left
  //Transform Box into OLV space
  csVector3 vBBMin;
  csVector3 vBBMax;
  csVector3 avTransBox[8];
  for(UINT i = 0; i < 8; ++i)
  {
    avTransBox[i] = m_mInvOLVRotation * avQcBox[i];
    //Searching for transformed OBB
    if(i == 0) vBBMax = vBBMin = avTransBox[i];
    else
    {
      vBBMin = VectorMin(avTransBox[i], vBBMin);
      vBBMax = VectorMax(avTransBox[i], vBBMax);
    }
  }
  //Transform back the front xyPlane of the BB
  //Sorted CW, and CCW from light position
  m_avBaseSlice[0] = m_mOLVRotation * csVector3(vBBMin.x, vBBMin.y, vBBMin.z);    //Bottom, Left
  m_avBaseSlice[1] = m_mOLVRotation * csVector3(vBBMin.x, vBBMax.y, vBBMin.z);    //Top, Left
  m_avBaseSlice[2] = m_mOLVRotation * csVector3(vBBMax.x, vBBMax.y, vBBMin.z);    //Top, Right
  m_avBaseSlice[3] = m_mOLVRotation * csVector3(vBBMax.x, vBBMin.y, vBBMin.z);    //Bottom, Right

  //Spacing Vector between each slice (from Back to Front!)
  m_iOLVTexWidth  = rCondWaterMixingRatios->GetSizeX() / 2;
  m_iOLVTexHeight = rCondWaterMixingRatios->GetSizeY() / 2;
  m_iOLVTexDepth  = rCondWaterMixingRatios->GetSizeZ() / 2;
  const csVector3 vDelta = ((vBBMax.z - vBBMin.z) / static_cast<float>(m_iOLVTexDepth)) * m_vZAxis;

  //Create empty OLV texture
  m_pOLVTexture.Invalidate();
  //CreateTexture

  //Create OLV-transformation matrices
  m_mOLVProjectionMatrix = ParallelProjection(vBBMax.x - vBBMin.x, vBBMax.y - vBBMin.y, 0.f, vBBMax.z - vBBMin.z);
  const csVector3 vCameraPos = m_mOLVRotation * csVector3((vBBMin.x + vBBMax.x) * 0.5f, (vBBMin.y + vBBMax.y) * 0.5f, vBBMax.z);
  m_mOLVCameraMatrix = CameraMatrix(vCameraPos, m_vLightDir, vLinIndepVec);

  //Resultion == Slice count
  //Rendering from front to back (reference: lightdirection)
  for(int i = m_iOLVTexDepth; i >= 0; --i)
  {
    //m_avBaseSlice[0] + vDelta * i;

  }

  m_bNewOLVTexture = true;
  return true;
}

//-------------------------------------------------------//

const bool csCloudsRenderer::CreateImpostor(const csVector3& vCameraPosition)
{
  m_vImpostorDirection = m_vPosition - vCameraPosition;
  m_vImpostorDirection.Normalize();

  return true;
}

//-------------------------------------------------------//

const bool csCloudsRenderer::Render(const csVector3& vCameraPosition)
{
  if(!ImpostorStillValid(vCameraPosition)) CreateImpostor(vCameraPosition);

  return true;
}

//-------------------------------------------------------//

//-------------------------------------------------------//