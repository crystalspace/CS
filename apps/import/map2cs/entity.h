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

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "brush.h"

class CMapParser;
class CMapFile;

class CMapKeyValuePair
{
public:
  CMapKeyValuePair(const char* key, const char* value);
  ~CMapKeyValuePair();
  const char* GetKey()   {return m_Key;}
  const char* GetValue() {return m_Value;}
protected:
  char* m_Key;
  char* m_Value;
};

/**
  * An entity is the basic object in a MAP file. Everything inside a map
  * file is an entity. A entity can contain an unlimited number of key
  * value pairs.
  * It can also contain an unlimited number of brushes. The brushes will
  * then shape this entity.
  * If the entity contains no brushes, it is a point based entity, and one
  * key/ value pair will contain the position. Point based entities are for
  * example lights, player startposition, environment sounds, special items
  * like weapon or ammo.
  * Brush based entities are the world itself, doors, water, triggers...
  */
class CMapEntity
{
public:
  /**
    * Constructs an entity
    */
  CMapEntity();

  /**
    * Destroys an entity. This will take care of removal of the brushes
    * too.
    */
  ~CMapEntity();

  /**
    * This will read a complete entity from a file (until the closing "}")
    * It will do all errorhandling and error messages itself. If there
    * was an error, it will return false. In that case you should not
    * continue reading the file.
    */
  bool Read(CMapParser* pParser, CMapFile* pMap);

  /**
    * Creates the polygons for all brushes.
    */
  void CreatePolygons();

  /// Access all brushes from outside
  size_t     GetNumBrushes()     {return m_Brushes.Length();}
  CMapBrush* GetBrush(size_t index) {return m_Brushes.Get(index);}

  /// Access all curves from outside
  size_t     GetCurveCount()      {return m_Curves.Length();}
  CMapCurve* GetCurve(size_t index) {return m_Curves.Get(index);}

  /**
    * Get the classname of this entity or "", if this entity has no
    * classname.
    */
  const char* GetClassname();

  /**
    * Get a name for this entity (may be '', or not unique!)
    */
  const char* GetName();

  /**
    * Gets the Origin (position) of the entity. If this entity does not have
    * an origin (like all brush based entities in Half-Life), this will return
    * false. Otherwise it will return true and set the vector v to the requested
    * position.
    */
  bool GetOrigin(CdVector3& v);

  /**
    * Get the string value of the given key. If the given key is not present this
    * function will return the given defaultvalue (0 if nothing is given)
    */
  const char* GetValueOfKey(const char* key, const char* defaultvalue = 0);

  /**
    * Get the numeric value of the given key. If the given key is not present this
    * function will return the given defaultvalue (0.0 if nothing is given)
    */
  double GetNumValueOfKey(const char* key, double defaultvalue = 0.0);

  /**
    * Get the numeric value of the given key. If the given key is not present this
    * function will not alter V1, v2 and v3 and will return false. Otherwise it sets
    * v1, v2 and v3 to the values of the key and returns true.
    */
  bool GetTripleNumValueOfKey(const char* key, double& v1, double& v2, double& v3);

  /**
    * Get the boolean value of the given key. If the given key is not present this
    * function will return the given defaultvalue (false if nothing is given)
    */
  bool GetBoolValueOfKey(const char* key, bool defaultvalue = false);

  /// Get the number of Key/Value pairs
  size_t GetNumberOfKeyValuePairs() {return m_Keymap.Length();}

  /// Get the key value pair at the given index (0..GetNumberOfKeyValuePairs()-1)
  CMapKeyValuePair* GetKeyValuePair(size_t index) {return m_Keymap[index];}
protected:
  /**
    * Here are all contained brushes stored.
    */
  CMapBrushVector        m_Brushes;

  /**
    * Here are all contained curves stored.
    */
  CMapCurveVector        m_Curves;

  /**
    * Here are all Key/ Value pairs stored.
    */
  CMapKeyValuePairVector m_Keymap;

  /**
    * This name is used as unique name for the entity, if no
    * other name has been given
    */
  csString m_UniqueName;

  /**
    * Add a key/ value pair. (For future purposes, right now, they will
    * just be ignored.)
    */
  void AddKeyValuePair(const char* Key, const char* Value);
};

#endif // __ENTITY_H__

