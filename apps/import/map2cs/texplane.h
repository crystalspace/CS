/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __TEXPLANE_H__
#define __TEXPLANE_H__

#include "texfile.h"

class CMapFile;

/**
  * This class takes care of a single textures plane. It can be
  * used in multiple places at the same time. It is possible (and
  * desirable), that no other plane with the same orientation and
  * texturing exists, because this will allow the final model
  * to use consitent texturing for all polygons on the same plane,
  * and will also enable potential optimisations in the engine.
  * <p>
  * Maybe it is no good idea, to derive this class von CdPlane,
  * because all math in the csXxx classes is done in float.
  * This may be good for speed, but it is bad for precision.
  * We don't really care for speed in out convertor, but we care
  * for precision, so this may become an issue in the future.
  */
class CMapTexturedPlane : public CdPlane
{
public:
  /**
    * Constructs a Plane from the data found in the original
    * map file. If QuakeModeTexture is true, then x_off,
    * y_off, rot_angle, x_scale, y_scale will be ignored and
    * v0, v1 and v2 will be used to define the texture.
    */
  CMapTexturedPlane(CMapFile* pMap,
                    CdVector3 v0, CdVector3 v1, CdVector3 v2,
                    CTextureFile* pTexture,
                    double x_off, double y_off, double rot_angle,
                    double x_scale, double y_scale,
		    CdVector3 v_tx_right, CdVector3 v_tx_u,
                    bool QuarkModeTexture, bool QuarkMirrored,
		    bool HLTexture);

  /**
    * Constructs a Plane from the given color
    */
  CMapTexturedPlane(CdVector3 v1, CdVector3 v2, CdVector3 v3,
                    int r, int g, int b);

  /**
    * Some sort of Copy Constructor. Used mainly, to create the mirror
    * plane, we will need for every plane in the map
    */
  CMapTexturedPlane(CMapTexturedPlane* pPlane, bool mirrored);

  /// The destructor, will do some cleanup, as usual.
  ~CMapTexturedPlane();

  /**
    * Checks if the given plane info matches this plane or not.
    * To match, the planes need to be the same, and the
    * textures orientation (Offset, Rotation and Scale)
    * must match. This method is used to avoid the same plane
    * is added multiple times to the map.
    */
  bool IsEqual(CMapTexturedPlane* pPlane);

  /**
    * Returns true, if both planes are (almost) the same geometry,
    * which means they occupy the same space. Texture information
    * is ignored for this test.
    */
  bool IsSameGeometry(CMapTexturedPlane* pPlane);

  /**
    * Assign a name to this plane. The plane will allocate memory
    * for that name and free this memory on destruction.
    * This name should be used to describe the plane in the world
    * file.
    */
  void SetName(const char* name);

  /// Get the name of the plane.
  const char* GetName() const;

  /// Get the name of the original texture of the plane
  const char* GetTextureName() const
  {return m_pTexture ? m_pTexture->GetTexturename() : "";}

  /// Get the name of the original texture of the plane
  CTextureFile* GetTexture() const {return m_pTexture;}

  /// Get the color of the texture
  void GetColor(int& r, int& g, int& b) const;

  /**
    * Return Texture coordinates for this plane.
    * 0=origin, 1=first, 2=second
    */
  CdVector3 GetTextureCoordinates(int num) const {return m_tx[num];}

  /**
    * returns a pointer to the according mirrored plane.
    */
  CMapTexturedPlane* GetMirror() const {return m_pMirrorPlane;}

  /**
    * sets a pointer to the mirror plane.
    */
  void SetMirror(CMapTexturedPlane* pMirror) {m_pMirrorPlane = pMirror;}

  /**
    * Returns vectors for the texture baseplane, to be used with
    * traditional Quake texturing. no: normal, xv: vector to be used
    * for the x coordinated of the texture, yv same for y.
    */
  void CalcTextureAxis(CdVector3& no,
                       CdVector3& xv,
                       CdVector3& yv);

  /**
    * Projects "OriginalPoint" along "Direction" onto this plane.
    * No errorchecking is done for the case, when Direction is parallel
    * to this plane!
    */
  void ProjectPoint(CdVector3& ProjectedPoint,
                    const CdVector3& OriginalPoint,
                    const CdVector3& Direction);

protected:

  /**
    * The name of this plane.
    */
  csString m_PlaneName;

  /**
    * The associated Texture.
    */
  CTextureFile* m_pTexture;

  /**
    * Define the texture coordinates
    */
  CdVector3 m_tx[3];

  /// Flatshaded color Red
  int m_Red;

  /// Flatshaded color Green
  int m_Green;

  /// Flatshaded color Blue
  int m_Blue;

  /**
    * A pointer the a plane with the same texture, but the opposite orientation
    */
  CMapTexturedPlane* m_pMirrorPlane;
};

#endif // __TEXPLANE_H__

