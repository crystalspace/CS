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

#ifndef __CONTAIN_H__
#define __CONTAIN_H__

#include "csutil/array.h"

// Central declaration of all required containers to avoid
// multiple definition of classes.

class CMapBrush;
class CMapEntity;
class CMapTexturedPlane;
class CMapPolygon;
class CMapPolygonSet;
class CMapKeyValuePair;
class CdVector3;
class CIPortal;
class CIThing;
class CISector;
class CTextureArchive;
class CTextureFile;
class CMapCurve;
class CMapCurvePoint;

#undef CS_DECLARE_TYPED_VECTOR
#define CS_DECLARE_TYPED_VECTOR(NAME,TYPE)			\
  typedef csArray<TYPE*> NAME

CS_DECLARE_TYPED_VECTOR(CMapBrushVector,         CMapBrush);
CS_DECLARE_TYPED_VECTOR(CMapEntityVector,        CMapEntity);
CS_DECLARE_TYPED_VECTOR(CMapTexturedPlaneVector, CMapTexturedPlane);
CS_DECLARE_TYPED_VECTOR(CMapPolygonVector,       CMapPolygon);
CS_DECLARE_TYPED_VECTOR(CMapPolygonSetVector,    CMapPolygonSet);
CS_DECLARE_TYPED_VECTOR(CMapKeyValuePairVector,  CMapKeyValuePair);
CS_DECLARE_TYPED_VECTOR(CIPortalVector,          CIPortal);
CS_DECLARE_TYPED_VECTOR(CIThingVector,           CIThing);
CS_DECLARE_TYPED_VECTOR(CISectorVector,          CISector);
CS_DECLARE_TYPED_VECTOR(CVector3Vector,          CdVector3);
CS_DECLARE_TYPED_VECTOR(CTextureArchiveVector,   CTextureArchive);
CS_DECLARE_TYPED_VECTOR(CTextureFileVector,      CTextureFile);
CS_DECLARE_TYPED_VECTOR(CMapCurvePointVector,    CMapCurvePoint);
CS_DECLARE_TYPED_VECTOR(CMapCurveVector,         CMapCurve);
CS_DECLARE_TYPED_VECTOR(CCharVector,             char);

#define DELETE_VECTOR_MEMBERS(VECT) \
  {size_t Num=(VECT).Length(); size_t i; \
   for (i=0;i<Num;i++) delete (VECT)[i]; \
   (VECT).DeleteAll();}

#define COPY_VECTOR_MEMBERS(DEST, SRC, TYPE) \
  {size_t Num=(SRC).Length(); \
  size_t i;  \
   for (i=0;i<Num;i++) (DEST).Push(new TYPE(*((SRC)[i])));}

#endif // __CONTAIN_H__
