/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cssysdef.h>

#include "furmesh.h"
#include "furdata.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  csHairData
  ********************/

  csHairData::~csHairData ()
  {
//     if (controlPointsCount)
//       delete controlPoints;
  }

  /********************
  *  csHairStrand
  ********************/

  csVector2 csHairStrand::GetUV( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD ) const
  {
    csVector2 strandUV(0);

    for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
      if (guideHairsRef[j].index < guideHairs.GetSize() )
        strandUV += guideHairsRef[j].distance * 
        guideHairs.Get(guideHairsRef[j].index).uv;
      else
        strandUV += guideHairsRef[j].distance * 
        guideHairsLOD.Get(guideHairsRef[j].index - guideHairs.GetSize()).uv;

    return strandUV;
  }

  void csHairStrand::Generate( size_t controlPointsCount,
    const csArray<csGuideHair> &guideHairs, 
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    // generate control points
    this -> controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++ )
    {
      controlPoints[i] = csVector3(0);

      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairs.Get(guideHairsRef[j].index).controlPoints[i];
        else
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairsLOD.Get(guideHairsRef[j].index - 
          guideHairs.GetSize()).controlPoints[i];
    }
  }

  void csHairStrand::Update( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance * 
          (guideHairs.Get(guideHairsRef[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairsRef[j].distance * (guideHairsLOD.Get
          (guideHairsRef[j].index - guideHairs.GetSize()).controlPoints[i]);
    }
  }

  /********************
  *  csGuideHair
  ********************/

  void csGuideHair::Generate (size_t controlPointsCount, float distance,
    const csVector3& pos, const csVector3& direction)
  {
    this->controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t j = 0 ; j < controlPointsCount ; j ++ )
      controlPoints[j] = pos + j * distance * direction;
  }

  /********************
  *  csGuideHairLOD
  ********************/

  void csGuideHairLOD::Generate( size_t controlPointsCount,
    const csArray<csGuideHair> &guideHairs, 
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    // generate control points
    this -> controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++ )
    {
      controlPoints[i] = csVector3(0);
      
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairs.Get(guideHairsRef[j].index).controlPoints[i];
        else
          controlPoints[i] += guideHairsRef[j].distance *
          guideHairsLOD.Get(guideHairsRef[j].index - 
          guideHairs.GetSize()).controlPoints[i];
    }
  }

  void csGuideHairLOD::Update( const csArray<csGuideHair> &guideHairs,
    const csArray<csGuideHairLOD> &guideHairsLOD )
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance * 
          (guideHairs.Get(guideHairsRef[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairsRef[j].distance * (guideHairsLOD.Get
          (guideHairsRef[j].index - guideHairs.GetSize()).controlPoints[i]);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

