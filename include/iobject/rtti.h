/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Martin Geisse
  
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

#ifndef __IOBJECT_RTTI_H__
#define __IOBJECT_RTTI_H__

/// Use this macro in the headers of a module that wants to use RTTI.
#define DECLARE_OBJECT_TYPE(type)					\
	extern int csObjectType_##type;

/// Use this macro in one source file of a module that wants to use RTTI.
#define ALLOCATE_OBJECT_TYPE(type)					\
	int csObjectType_##type = -1;

/**
 * Use this macro in the Initialize() method of a module that wants
 * to use RTTI.
 */
#define INTIALIZE_OBJECT_TYPE(StringServer, type);			\
	csObjectType_##type = (StringServer)->Request (#type);

/// This is the 'dynamic cast'.
#define QUERY_OBJECT_TYPE(object, type)					\
	((type*)(object)->QueryObjectType(csObjectType_##type))

/// Put this macro in the interface definition of an RTTI object.
#define DECLARE_ABSTRACT_OBJECT_INTERFACE				\
	virtual void *QueryObjectType (int typeID) = 0;

/// Put this in the class definition of an RTTI object.
#define DECLARE_OBJECT_INTERFACE					\
	virtual void *QueryObjectType (int typeID);

/// Put these in the source file for the RTTI object:
#define IMPLEMENT_OBJECT_INTERFACE(object)				\
	void *object::QueryObjectType (int Type) {

#define IMPLEMENTS_OBJECT_TYPE(tvar)					\
	if (Type == tvar) return this;

#define IMPLEMENTS_EMBEDDED_OBJECT_TYPE(object, tvar)			\
	if (Type == tvar) return (object)->QueryObjectType (Type);

#define IMPLEMENT_OBJECT_INTERFACE_END					\
	return NULL; }

#endif // __IOBJECT_RTTI_H__
