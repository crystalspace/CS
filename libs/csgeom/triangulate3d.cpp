/*
    Copyright (C) 2007 by Scott Johnson

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

#include "cssysdef.h"
#include "csgeom/triangulate3d.h"

namespace CS
{
namespace Geom
{

	bool Triangulate3D::Process(csContour3& polygon, csTriangleMesh& result, csContour3& result_vertices, csContour3* holes)
	{
		return true;
	}

	bool Triangulate3D::SeparatePlanes(const csContour3& polygon, csContour3* results)
	{
		return true;
	}

} // namespace Geom
} // namespace CS