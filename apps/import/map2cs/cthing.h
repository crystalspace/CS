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

#ifndef CTHING_H
#define CTHING_H

#include "ithing.h"

class CCSWorld;
class CCSSector;

/**
  * This class will encapsulate a Thing to show up in a Crystal Space
  * worldfile.
  */
class CCSThing : public CIThing
{
public:
  /// The constructor (as usual)
  CCSThing(CMapEntity* pEntity);

  /// The destuctor
  virtual ~CCSThing();

  /// Write this thing as part of an sector
  virtual bool Write(CIWorld* pIWorld, CISector* pISector);

  /// Write only polygons without headings
  virtual bool WriteAsPart(CIWorld* pIWorld, CISector* pISector);
};

#endif
