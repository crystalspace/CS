/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

/*
 * This file contains parsing functions for various 'primitives', such as
 * vectors, matrices and color specifications.
 */

#include "cssysdef.h"
#include "qint.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csgfx/rgbpixel.h"
#include "csloader.h"
#include "ivideo/graph3d.h"

bool csLoader::ParseQuaternion (char* buf, csQuaternion &q)
{
  csScanStr (buf, "%f,%f,%f,%f", &q.x, &q.y, &q.z, &q.r);
  return true;
}

bool csLoader::ParseColor (char *buf, csRGBcolor &c)
{
  csColor cc;
  if (ParseColor (buf, cc))
  {
    c.red = QInt (cc.red * 255.99f);
    c.green = QInt (cc.green * 255.99f);
    c.blue = QInt (cc.blue * 255.99f);
    return true;
  }
  return false;
}

bool csLoader::ParseColor (char *buf, csColor &c)
{
  float color[3];
  int num;
  csScanStr (buf, "%F", color, &num);
  if (num >= 3) 
  {
    c.red   = color[0];
    c.green = color[1];
    c.blue  = color[2];
  } 
  else if (num >= 1)
  {
    c.red = c.green = c.blue = color[0];
  } 
  else 
  {
    ReportError (
      "crystalspace.maploader.parse.badformat",
      "No color specified");
    return false;
  }
  return true;
}
