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

#ifndef CONTAIN_H
#define CONTAIN_H

#include "csutil/csvector.h"

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

/*
 * Unfortunately, map2cs was written at the time when CS_DECLARE_TYPED_VECTOR
 * was in a very bad shape (thanks, Brandon). Thus it relies on some incorrect
 * features such as inability to cleanup upon destruction and so on. Thus we
 * redefine the CS_DECLARE_TYPED_VECTOR macro so that it implements the original
 * expected behaviour.
 */
#undef CS_DECLARE_TYPED_VECTOR
#define CS_DECLARE_TYPED_VECTOR(NAME,TYPE)				\
  class NAME : public csVector					\
  {								\
  public:							\
    NAME (int ilimit = 16, int ithreshold = 16) :		\
      csVector (ilimit, ithreshold) {}				\
    virtual ~NAME ()						\
    { DeleteAll (); }						\
    inline TYPE*& operator [] (int n)				\
    { return (TYPE*&)csVector::operator [] (n); }		\
    inline TYPE*& operator [] (int n) const			\
    { return (TYPE*&)csVector::Get (n); }			\
    inline TYPE*& Get (int n) const				\
    { return (TYPE*&)csVector::Get (n); }			\
    int Find (TYPE* which) const				\
    { return csVector::Find ((csSome)which); }			\
    int FindKey (const TYPE* value) const			\
    { return csVector::FindKey ((csSome)value); }		\
    inline void Push (TYPE* what)				\
    { csVector::Push ((csSome)what); }				\
    inline TYPE* Pop ()						\
    { return (TYPE*)csVector::Pop (); }				\
    bool Insert (int n, TYPE* Item)				\
    { return csVector::Insert (n, (csSome)Item); }		\
  };

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
  {int Num=(VECT).Length(); int i; \
   for (i=0;i<Num;i++) delete (VECT)[i]; \
   (VECT).DeleteAll();}

#define COPY_VECTOR_MEMBERS(DEST, SRC, TYPE) \
  {int Num=(SRC).Length(); \
  int i;  \
   for (i=0;i<Num;i++) (DEST).Push(new TYPE(*((SRC)[i])));}

#endif
