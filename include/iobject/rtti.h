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

/**
 * Pseudo-RTTI system: These macros allow to quickly and safely cast
 * objects to other types. They work similar to SCF's QUERY_INTERFACE,
 * but are less flexible and faster. They do not test for version
 * compatibility, and the requested type is given as an integer number,
 * not a string. This system requires that the ID numbers are allocated
 * from a global plug-in, the string server. <p>
 */

/**
 * Use this macro in the headers of a module that wants to use RTTI. It
 * allows to use the type information in the whole module.
 */
#define DECLARE_OBJECT_TYPE(type)					\
	extern int csObjectType_##type;

/**
 * Use this macro in one source file of a module that wants to use RTTI. It
 * defines a global variable in the module that contains the type information.
 */
#define ALLOCATE_OBJECT_TYPE(type)					\
	int csObjectType_##type = -1;

/**
 * Use this macro in the Initialize() method of a module that wants
 * to use RTTI.
 */
#define INITIALIZE_OBJECT_TYPE(StringServer, type);			\
	csObjectType_##type = (StringServer)->Request (#type);

/// This is the 'dynamic cast'.
#define QUERY_OBJECT_TYPE(object, type)					\
	((type*)(object)->QueryObjectType(csObjectType_##type))

/**
 * Put this macro in the interface definition of an RTTI object. It adds the
 * (abstract) method that is required for the 'dynamic cast'.
 */
#define DECLARE_ABSTRACT_OBJECT_INTERFACE				\
	virtual void *QueryObjectType (int typeID) = 0;

/**
 * Put this macro in the cass definition of an RTTI object. It adds the
 * method that is required for the 'dynamic cast'.
 */
#define DECLARE_OBJECT_INTERFACE					\
	virtual void *QueryObjectType (int typeID);

/**
 * Put these macros in the source file for the RTTI object. They implement the
 * 'dynamic cast' method. In the simple case you just write:
 *
 *   IMPLEMENT_OBJECT_INTERFACE (myclass)
 *   IMPLEMENT_OBJECT_INTERFACE_END
 *
 * This allows casting to 'myclass'. You can add any number of
 * IMPLEMENTS_OBJECT_TYPE or IMPLEMENTS_EMBEDDED_OBJECT_TYPE between these two
 * macros to support casting to other types than the class itself. This is
 * usually used to allow casting to SCF interfaces, for example
 *
 *   IMPLEMENT_OBJECT_INTERFACE (csSector)
 *     IMPLEMENTS_OBJECT_TYPE (iSector)
 *   IMPLEMENT_OBJECT_INTERFACE_END
 *
 * You can use the 'EXT' version of the macros if your RTTI-able class is a
 * subclass of another RTTI-able class to extend the class list of the
 * parent class.
 */

#define IMPLEMENT_OBJECT_INTERFACE(object)				\
	void *object::QueryObjectType (int Type) {			\
	  if (Type == csObjectType_##object) return this;

#define IMPLEMENT_OBJECT_INTERFACE_EXT(object,parentclass)		\
	void *object::QueryObjectType (int Type) {			\
	  void *obj = parentclass::QueryObjectType (Type);		\
	  if (obj) return obj;						\
	  if (Type == csObjectType_##object) return this;		\

#define IMPLEMENTS_OBJECT_TYPE(type)					\
	if (Type == csObjectType_##type) return this;

#define IMPLEMENTS_EMBEDDED_OBJECT_TYPE(type)				\
	if (Type == csObjectType_##type) return &scf##type;

#define IMPLEMENT_OBJECT_INTERFACE_END					\
	return NULL; }

#define IMPLEMENT_OBJECT_INTERFACE_EXT_END				\
	return NULL; }

/**
 * Sometimes functions want you to pass a type ID number. You can get the
 * number with this macro.
 */
#define OBJECT_TYPE_ID(type)						\
	(csObjectType_##type)

#endif // __IOBJECT_RTTI_H__
