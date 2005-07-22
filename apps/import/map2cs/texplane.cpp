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

#include "cssysdef.h"
#include "mapstd.h"
#include "map.h"
#include "texplane.h"

CMapTexturedPlane::CMapTexturedPlane(CMapFile*,
                                     CdVector3 v0, CdVector3 v1, CdVector3 v2,
                                     CTextureFile* pTexture,
                                     double x_off, double y_off, double rot_angle,
                                     double x_scale, double y_scale,
				     CdVector3 v_tx_right, CdVector3 v_tx_up,
                                     bool QuarkModeTexture, bool QuarkMirrored,
				     bool HLTexture)
{
  assert(pTexture);

  m_Red    = 0;
  m_Green  = 0;
  m_Blue   = 0;

  //Calculate the planes equation from the given vectors.
  CdMath3::CalcPlane(v0, v1, v2, norm, DD);
  double len = norm.Norm();
  A() = A() / len;
  B() = B() / len;
  C() = C() / len;
  D() = D() / len;

  //Now we assign all the texture information.
  m_pTexture = pTexture;

  if (x_scale==0) x_scale = 1;
  if (y_scale==0) y_scale = 1;

  m_PlaneName = "";

  if (QuarkModeTexture)
  {
    //In QuArK style maps, texturing is really simple.
    //The three given vertices are:
    //v0: the bottom left corner of the texture (origin)
    //v1: a point, 128 pixel upwards from the origin
    //v2: a point, 128 pixel right from the origin
    //If QuarkMirrored is true. (//TX2), then you need to
    //swap v1 and v2.

    //Now the texture needs to be aligned, so that v0, v1 and v2
    //get the proper texture coordinates. This is pretty simple,
    //because Crystal Space uses a pretty similar texture
    //specification system. The main difference are the reference
    //points required:
    //m_tx[0]: the coordinates of the top left corner of the texture
    //m_tx[1]: the coordinates of the top right corner of the texture
    //m_tx[2]: the coordinates of the bottom left corner of the texture

    //With a little vector arithmetics this is no problem.

    double scalex = (double)m_pTexture->GetOriginalWidth() /128.0;
    double scaley = (double)m_pTexture->GetOriginalHeight()/128.0;

    if (QuarkMirrored)
    {
      m_tx[0] = v0 + scaley*(v2-v0);                  //CS Origin
      m_tx[1] = v0 + scaley*(v2-v0) + scalex*(v1-v0); //CS First
      m_tx[2] = v0;                                   //CS Second
    }
    else
    {
      m_tx[0] = v0 + scaley*(v1-v0);                  //CS Origin
      m_tx[1] = v0 + scaley*(v1-v0) + scalex*(v2-v0); //CS First
      m_tx[2] = v0;                                   //CS Second
    }
  }
  else
  {
    //Quake style texture mapping is _really_ complex:

    //We need to know texture size, because in Quake higher resulution
    //Textures will show up bigger. (default scale: one pixel is one
    //Quake world unit)

    //Width and Height of the texture. (after scaling)
    double sx    = m_pTexture->GetOriginalWidth()  * x_scale;
    double sy    = m_pTexture->GetOriginalHeight() * y_scale;

    //Rotation of the texture. (converted from degrees to rad)
    double angle = rot_angle*-0.017453293;

    //The Origin of the texture is rotated around (0,0) and scaled.
    double xo=-( cos(angle)*x_off*x_scale +
                 sin(angle)*y_off*y_scale );
    double yo=-(-sin(angle)*x_off*x_scale +
                 cos(angle)*y_off*y_scale );
    
    //The first and second point are relative to the origin and of course
    //rotated too.
    double x1=xo+sx*cos(angle);
    double y1=yo-sx*sin(angle);
    double x2=xo+sy*sin(angle);
    double y2=yo+sy*cos(angle);

    //Now we get the default plane for the current orientation of this
    //plane.
    CdVector3 norm;
    CdVector3 xv;
    CdVector3 yv;
    if (HLTexture)
    {
      xv = v_tx_right;
      yv = v_tx_up;
      norm = xv % yv;
    }
    else
      CalcTextureAxis(norm, xv, yv);

    //Now we calculate points on on this plane to represent out texture
    //coordinates
    CdVector3 tv0 = xv*xo + yv*yo;
    CdVector3 tv1 = xv*x1 + yv*y1;
    CdVector3 tv2 = xv*x2 + yv*y2;

    //Now we project these points onto our own plane along the default
    //planes normal vector.
    ProjectPoint(m_tx[0], tv0, norm); //Origin
    ProjectPoint(m_tx[1], tv1, norm); //First
    ProjectPoint(m_tx[2], tv2, norm); //Second
  }

  //No mirror plane for this plane yet.
  m_pMirrorPlane = 0;
}

CMapTexturedPlane::CMapTexturedPlane(CMapTexturedPlane* pPlane, bool mirrored)
{
  //Depending on the setting of mirrored, we will copy the original geometry,
  //or we will set the mirrored plane
  double factor = mirrored ? -1.0 : 1.0;
  A() = factor * pPlane->A();
  B() = factor * pPlane->B();
  C() = factor * pPlane->C();
  D() = factor * pPlane->D();

  //Initialise plane name adnd texture name
  m_PlaneName = "";
  m_pTexture  = pPlane->m_pTexture;

  //texture coordinates stay the same
  int i;
  for (i=0; i<3; i++)
  {
    m_tx[i] = pPlane->m_tx[i];
  }

  //No mirror plane for this plane yet.
  m_pMirrorPlane = 0;
}

CMapTexturedPlane::CMapTexturedPlane(CdVector3 v0, CdVector3 v1, CdVector3 v2,
                                     int r, int g, int b)
{
  m_Red    = r;
  m_Green  = g;
  m_Blue   = b;

  //Calculate the planes equation from the given vectors.
  CdMath3::CalcPlane(v0, v1, v2, norm, DD);
  double len = norm.Norm();
  A() = A() / len;
  B() = B() / len;
  C() = C() / len;
  D() = D() / len;

  //This plane is flatshaded
  m_pTexture  = 0;
  m_PlaneName = "";

  m_tx[0] = CdVector3(0,0,0);
  m_tx[1] = CdVector3(0,0,0);
  m_tx[2] = CdVector3(0,0,0);

  //No mirror plane for this plane yet.
  m_pMirrorPlane = 0;
}


CMapTexturedPlane::~CMapTexturedPlane()
{
}

bool CMapTexturedPlane::IsEqual(CMapTexturedPlane* pPlane)
{
  //Compare textures
  if (pPlane->m_pTexture != m_pTexture) return false;

  //Check if textures are aligned equal
  if (m_tx[0] != pPlane->m_tx[0]) return false;
  if (m_tx[1] != pPlane->m_tx[1]) return false;
  if (m_tx[2] != pPlane->m_tx[2]) return false;

  if (m_Red   != pPlane->m_Red)   return false;
  if (m_Green != pPlane->m_Green) return false;
  if (m_Blue  != pPlane->m_Blue)  return false;

  //If the textures are equal we let csMath3 do the comparison of the
  //planes.
  return CdMath3::PlanesEqual(*this, *pPlane);
}

bool CMapTexturedPlane::IsSameGeometry(CMapTexturedPlane* pPlane)
{
  return CdMath3::PlanesEqual(*this, *pPlane);
}

void CMapTexturedPlane::SetName(const char* name)
{
  m_PlaneName = name;
}

const char* CMapTexturedPlane::GetName() const
{
  return m_PlaneName;
}

void CMapTexturedPlane::GetColor(int& r, int& g, int& b) const
{
  r = m_Red;
  g = m_Green;
  b = m_Blue;
}

void CMapTexturedPlane::CalcTextureAxis(CdVector3& no,
                                        CdVector3& xv,
                                        CdVector3& yv)
{
  //This is a table of all possible baseplanes. The order of these
  //planes is important, because this will decide, which plane
  //will be used, if the give plane is aligned at 45 degrees between
  //two axis.
  struct
  {
    double x;
    double y;
    double z;
  }
  baseaxis[18] =
  {
    { 0, 0, 1}, { 1, 0, 0}, { 0,-1, 0}, // floor      left-handed
    { 0, 0,-1}, { 1, 0, 0}, { 0,-1, 0}, // ceiling    right-handed
    { 1, 0, 0}, { 0, 1, 0}, { 0, 0,-1}, // west wall  left-handed
    {-1, 0, 0}, { 0, 1, 0}, { 0, 0,-1}, // east wall  right-handed
    { 0, 1, 0}, { 1, 0, 0}, { 0, 0,-1}, // south wall right-handed
    { 0,-1, 0}, { 1, 0, 0}, { 0, 0,-1}  // north wall left-handed
  };

  double best     = 0;
  int   bestaxis = 0;

  //Search for the best axis by comparing all normals
  int i;
  for (i=0 ; i<6 ; i++)
  {
    double dot = Normal() * CdVector3(baseaxis[i*3].x,
                                     baseaxis[i*3].y,
                                     baseaxis[i*3].z);
    if (dot > best)
    {
      best     = dot;
      bestaxis = i;
    }
  }

  //return normal and two axis that define the plane.
  no.x=baseaxis[bestaxis*3+0].x;
  no.y=baseaxis[bestaxis*3+0].y;
  no.z=baseaxis[bestaxis*3+0].z;

  xv.x=baseaxis[bestaxis*3+1].x;
  xv.y=baseaxis[bestaxis*3+1].y;
  xv.z=baseaxis[bestaxis*3+1].z;

  yv.x=baseaxis[bestaxis*3+2].x;
  yv.y=baseaxis[bestaxis*3+2].y;
  yv.z=baseaxis[bestaxis*3+2].z;
}

void CMapTexturedPlane::ProjectPoint(CdVector3& ProjectedPoint,
                                     const CdVector3& OriginalPoint,
                                     const CdVector3& Direction)
{
  double denom = norm * Direction;
  assert(ABS (denom) > SMALL_EPSILON);

  double dist = -(norm*OriginalPoint + DD) / denom;

  ProjectedPoint = OriginalPoint + dist*Direction;
}
