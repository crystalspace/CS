/*  
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)
    modified by Petr Kocmid (pkocmid@atlas.cz)
 
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

/*
  [jan 2000] XML output added by Petr Kocmid (pkocmid@atlas.cz)
  In case of troubles, or when you do not wank link in iostreams
  you can fall back to strictly original version by defining:
  
    NO_XML_SUPPORT

  for the whole project 
 */

#ifndef XWORLD_H
#define XWORLD_H

#include "iworld.h"
#include "ithing.h"
#include "isector.h"

/**
  * This class will encapsulate a Thing to show up in a Crystal Space
  * worldfile. 
  */
class CXmlThing : public CIThing
{
public:
  /// The constructor (as usual)
  CXmlThing(CMapEntity* pEntity) : CIThing(pEntity) {}

  /// The destuctor
  ~CXmlThing() {}

  /// Write this thing as part of an sector
  virtual bool Write(CIWorld*, CISector*) {return true;}

protected:
};

/**
  * this class encapsulates a Sector in Crystal Space terminology. This means
  * it is a concave Sector of space that can contains a number of things and
  * is connected to other sectors by portals.
  */
class CXmlSector : public CISector
{
public:
  /// The constructor. Needs a MapBrush as template for the shape.
  CXmlSector (CMapBrush* pBrush) : CISector(pBrush){}

  /// The destuctor
  ~CXmlSector () {}

  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  bool Write(CIWorld*) {return true;}

protected:
}; //CXmlSector 

/**
  * this class encapsulates the worldin Crystal Space terminology. This special
  * class is able to store the world as an XML file.
  */
class CXmlWorld : public CIWorld
{
public:
  /// The constructor (as usual)
  CXmlWorld();

  /// The destuctor
  ~CXmlWorld();

  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  bool Write(const char* filename, CMapFile* pMap, const char * sourcename);

  /// return a new CXmlThing
  virtual CIThing* CreateNewThing(CMapEntity* pEntity);

  /// return a new CXmlSector
  virtual CISector* CreateNewSector(CMapBrush* pBrush);

protected:

protected:
}; //CXmlWorld

#endif

