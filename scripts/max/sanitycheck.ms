------------------------------------------------------------
-- Author: Luca Pancallo <pancallo@netscape.net>
--	
-- Copyright (C) 2002 PlaneShift Team (info@planeshift.it, 
-- http://www.planeshift.it)
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation (version 2 of the License)
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--
------------------------------------------------------------
version = 6
-- catch the parameter from calling script
if (deleteFaces=="undefined") then (
	deleteFaces = false;
)

count = 0;

tot = objects.count
longmessage = ""
message = ""
unweld = false

for i = 1 to tot do
(
	obj = objects[i]
	if ((classOf obj)!=Editable_mesh) then
	(
		format "discard %" obj.name
		continue
	)

	format "analizying object % \n" obj.name

	if (obj.numfaces==0) then (
		format "Object % has 0 faces! \n" obj.name
		longmessage = longmessage + obj.name + " has 0 faces!\n"
	)

	for i = 1 to obj.numfaces do 
	(
		if (i>obj.numfaces) then
			continue;

		face = getface obj i
		if (face==undefined) then
			continue;

		v1 = getvert obj face[1]
		v2 = getvert obj face[2]
		v3 = getvert obj face[3]
		
		-- we had problems with rounding. some decimals are not shown, but present in the float
		-- the hacky solution is to convert to string. that truncates the decimals to 5
		v1x = v1.x as String
		v1y = v1.y as String
		v1z = v1.z as String
		v2x = v2.x as String
		v2y = v2.y as String
		v2z = v2.z as String
		v3x = v3.x as String
		v3y = v3.y as String
		v3z = v3.z as String

		--format "% % % % \n" obj.name v1 v2 v3
		
		bad = 0;
		equal = 0;
		-- check for at least 2 axis are equal on all tree vertexes
		if (v1x==v2x and v2x==v3x) then
			equal = equal + 1;
		if (v1y==v2y and v2y==v3y) then
			equal = equal + 1;
		if (v1z==v2z and v2z==v3z) then
			equal = equal + 1;

		if (equal>1) then
			bad = 1;

		-- check for 2 vertexes in the same spot
		equal = 0;
		if (v1x==v2x and v1y==v2y and v1z==v2z) then
			equal = equal + 1;
		if (v1x==v3x and v1y==v3y and v1z==v3z) then
			equal = equal + 1;
		if (v2x==v3x and v2y==v3y and v2z==v3z) then
			equal = equal + 1;

		if (equal>0) then
			bad = 1;

		if (bad==1) then (
			format "The object % has an INVALID polygon: %\n" obj.name face
			longmessage = longmessage + obj.name + " " +(face as String)+ "\n"
			count = count + 1;

			-- delete the polygon
			if (deleteFaces==true) then (
				format "deleting % face %\n" obj.name i
				select obj
				subObjectLevel = 3 -- set subobjectlevel to triangle
				delete obj.faces[i]
			)
		)

	) -- end for numfaces

	verts = getNumVerts obj
	uvVerts = getNumTVerts obj
	format "% % \n" verts uvVerts
	if (uvVerts>verts) then (
	  if (unweld==false) then (
	    unweld=true
		longmessage = longmessage + "You have a number of unwelded UV vertexes on object: "+obj.name
	  ) else (
	    longmessage = longmessage + ", " +obj.name
	  )
	)
)

format "Total bad polygons: % " count;

cs = count as String
if (deleteFaces) then
	message = "Total bad polygons found and deleted: " + cs
else
	message = "Total bad polygons found: " + cs

message = message + "\n" + (longmessage as String)
messageBox message
