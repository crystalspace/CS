------------------------------------------------------------
-  (c) 2002 Luca Pancallo <pancallo@netscape.net>
-	
- Copyright (C) 2002 PlaneShift Team (info@planeshift.it, 
- http://www.planeshift.it)
-
- This program is free software; you can redistribute it and/or
- modify it under the terms of the GNU General Public License
- as published by the Free Software Foundation (version 2 of the License)
- This program is distributed in the hope that it will be useful,
- but WITHOUT ANY WARRANTY; without even the implied warranty of
- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
- GNU General Public License for more details.
- You should have received a copy of the GNU General Public License
- along with this program; if not, write to the Free Software
- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-
------------------------------------------------------------


-- ////////////////////////
-- Write material function
-- ////////////////////////

fn WriteMaterials outFile =
(

  format "  TEXTURES (\n" to:outFile

  for m in sceneMaterials do
  (
    -- skip multitexture materials
	if ((classOf m)==Standardmaterial) then
	    format "    TEXTURE '%' (FILE ('%')) \n" m.name m.name to:outFile

  )

  format "  )\n" to:outFile

  format "  MATERIALS (\n" to:outFile

  for m in sceneMaterials do
  (
	if ((classOf m)==Standardmaterial) then
	    format "    MATERIAL '%' (TEXTURE ('%'))\n" m.name m.name to:outFile
  )
  
  format "  )\n\n" to:outFile

)


-- ////////////////////////
-- Main: program starts here
-- ////////////////////////

-- output file
outFile = createFile "d:\\Luca\\Crystal Space\\CS\\world"

-- set debug output
debug=false

-- define if export for Dynavis system
dynavis=true

-- Combine polys
polyCombine = true

-- Define verbose output (that takes more space and memory)
verboseMode = false

-- parameters for scaling and relocation
	xscale = 0.01
	yscale = 0.01
	zscale = 0.01
	
	xrelocate = 0;
	yrelocate = 0;
	zrelocate = 0;

-- write header

    format "WORLD (\n" to:outFile

-- write materials

    WriteMaterials outFile

-- write plugins

    format "  PLUGINS (\n" to:outFile
    format "    PLUGIN 'thing' ('crystalspace.mesh.loader.thing')\n" to:outFile
    format "  )\n\n" to:outFile

-- writestartpoints
    --WriteStartPoints();

-- start sector
    format "  SECTOR 'room' (\n" to:outFile

	if (dynavis) then
		format "    CULLERP ('crystalspace.culling.dynavis') " to:outFile

	format "IF YOU GET ANY ERROR convert to editable mesh and collapse stack of offending object\n"


lightsFound = #()
startingPos = null

-- cycle on all objects in the scene 
for obj in objects do 
(

    -- skip groups for now
	if ( (classOf obj)==Dummy) then (
		format "Skipping Dummy Object: %\n" obj.name
		continue
	)

	-- store lights for later use
	if ( (classOf obj)==Omnilight) then (
	    append lightsFound obj
		format "Found Omnilight Object: %\n" obj.name
		continue
	)

	-- store camera for later use
	if ( (classOf obj)==Targetcamera) then (
	    startingPos = obj
		format "Found Targetcamera Object: %\n" obj.name
		continue
	)

	-- skip target camera
	if ( (classOf obj)==Targetobject) then (
		format "Skipping Targetobject Object: %\n" obj.name
		continue
	)

	-- converts all objects to Mesh, this reduces a lot of errors BUT takes too long
	--converttomesh obj

	-- give warning about Polymesh
	if ((classOf obj)==Poly_Mesh) then (
		format "WARNING Object % is a PolyMesh, converting to Editable Mesh... \n" obj.name
		converttomesh obj
	)

	-- give warning about Editable_Poly
	if ((classOf obj)==Editable_Poly) then (
		format "WARNING Object % is a Editable_Poly, converting to Editable Mesh... \n" obj.name
		converttomesh obj
	)

    -- output name of object and some info
	format "\n\nFound Object Name: % Faces: %\n" obj.name obj.numfaces

    format "    ;Object Name: % Faces: %\n" obj.name obj.numfaces to:outFile 

    format "    MESHOBJ '%' (\n" obj.name to:outFile
	format "      PLUGIN ('thing')\n" to:outFile

	-- handles transparent objects
	if (findString obj.name "_t_" !=undefined) then (
	  format "      PRIORITY('alpha')\n" to:outFile
	  format "      ZUSE ()\n" to:outFile
	)

    -- handles sky objects
	else if (findString obj.name "sky" !=undefined) then (
	  format "      PRIORITY('sky')\n" to:outFile
	  format "      ZFILL ()\n" to:outFile
	)
    -- handles zfill objects
	else if (findString obj.name "_s_" !=undefined) then (
	  format "      PRIORITY('wall')\n" to:outFile
	  format "      ZFILL ()\n" to:outFile
	)
	else ( 
	  format "      PRIORITY('object')\n" to:outFile
	  format "      ZUSE ()\n" to:outFile
	)
	format "      PARAMS (\n" to:outFile

   -- output vertexes of the object in XZY format
   for v in obj.verts do
   (
     xvert = (v.pos.x * xscale) + xrelocate
	 yvert = (v.pos.y * yscale) + yrelocate
	 zvert = (v.pos.z * zscale) + zrelocate
     format "      V (%,%,%)\n"  xvert zvert yvert to:outFile
   )

   format "\n" to:outFile

   -- list of faces already printed
   skiplist = #()
   numMat = 0

   -- determine number of materials on object
   if ((classOf obj.mat)==Standardmaterial) then
   		numMat = 1
   else
   		numMat = obj.mat.count

   -- cycle on all materials present in object
   for j=1 to numMat do
   (
    -- flag as new material
    matChanged = true

    if ((classOf obj.mat)==Standardmaterial) then
    	matName = obj.mat.name
	else
		matName = obj.mat[j].name

	if (debug) then format "Searching faces with material: %\n" matName

	-- cycle on all faces of object
	for i =1 to obj.numFaces do
	(
	   -- test if face was part of a poly already written
	   if (findItem skiplist i!=0) then (
	       if (debug) then format "skipping face %\n" i
		   continue
	   )

       -- get material name on current face
	   currentMat = ""
	   matID = getFaceMatID obj i
	    if (obj.mat==undefined) then
			format "UNDEFINED MATERIAL for object %\n" obj.name
		else (
		    if ((classOf obj.mat)==Standardmaterial) then
		    	currentMat = obj.mat.name
			else
				currentMat = obj.mat[matID].name
		)
		
		if (debug) then format "Material found: % on face %\n" currentMat i

	   -- output face only if it has current MatID
	   if (currentMat==matName) then
	   (
	        -- output material only if has changed since previous face
			if (matChanged) then (
			    if (obj.mat==undefined) then
					format "UNDEFINED MATERIAL for object %\n" obj.name
				else (
				    if ((classOf obj.mat)==Standardmaterial) then
				    	format "      MATERIAL ('%')\n" obj.mat.name to:outFile
					else
						format "      MATERIAL ('%')\n" obj.mat[matID].name to:outFile
				)
				matName=currentMat
				matChanged=false
			)

			if (polyCombine) then (
			   -- get polygon of current face
			   poly = meshop.getPolysUsingFace obj obj.faces[i]
	
			   -- check if poly is valid: for now should be 2 faces MAX! and have same material
			   m=0
			   good = true
			   otherface = undefined
			   for a in poly do
			   (
			        m=m+1
					-- check max 2 faces
					if (m>2) then
						good=false
				    else if (a!=obj.faces[i].index) then
					   otherface = a
	
					-- check same material
					if (getFaceMatID obj i != (getFaceMatID obj a)) then
						good=false
			   )
	
			   -- check if only one face
			   if (m<2) then
			   		good=false
			) else
				good = false

		   --////////////////////
		   --poly can be combined
		   --////////////////////
		   if (good) then
		   (
			   if (debug) then format "Found poly: %\n" poly

			   -- add face to merge to skip-list
			   append skiplist otherface

				-- get its 4 vertices
			    Xverts1=getface obj i
				Xverts2=getface obj otherface

				-- trasform to array
				verts1 = #()
				verts2 = #()
				for h=1 to 3 do append verts1 Xverts1[h]
				for h=1 to 3 do append verts2 Xverts2[h]

				verts = #()
				additionalVertex = 0
				oppositeVertex = 0

				if (debug) then format "Vertex of first face: %\n" verts1

				-- search additional vertex
				for h=1 to 3 do
				(
					if (findItem verts1 verts2[h]==0) then
						additionalVertex = h
				)
				if (debug) then format "Additional vertex: % " verts2[additionalVertex]

				-- search opposite vertex
				for h=1 to 3 do
				(
					if (findItem verts2 verts1[h]==0) then
						oppositeVertex = h
				)
				if (debug) then format "Opposite vertex: % " verts1[oppositeVertex]


				-- check texture
				Tmapping = true
			   if (getNumTVerts obj==0) then
			   (
			   		format "No UV maps assigned to object: %\n" obj.name
					Tmapping = false
			   ) else (
		        textVert1 = getTVFace obj i
				textVert2 = getTVFace obj otherface
			   )

				-- build poly
				Tverts = #()
				commonV = false
				addedAdd = false
				addedOpp = false
				for h=1 to 3 do
				(
					 -- check next vertex
					 if (h!=3 and h!=oppositeVertex) then (
					    nextV = verts1[h+1]
						-- first common vertex
					 	if (nextV != verts1[oppositeVertex]) then (
							append verts verts1[h]
							append verts verts2[additionalVertex]
							
							-- add mapping vertexes
							if (Tmapping) then
							(
								append Tverts (getTVert obj textVert1[h])
								append Tverts (getTVert obj textVert2[additionalVertex])
							)
							addedAdd = true
							continue
						)
					 )
					
					-- add vertex
					append verts verts1[h]
					-- add mapping vertexes
					if (Tmapping) then
						append Tverts (getTVert obj textVert1[h])
					
					if (h==3 and (not addedAdd) ) then (
						append verts verts2[additionalVertex]
						if (Tmapping) then	
							append Tverts (getTVert obj textVert2[additionalVertex])	
					)
				)

				-- export in XZY format
				if (debug) then format "POLY: %" verts

				a = verts[3] as Integer
				b = verts[2] as Integer
				c = verts[1] as Integer
				d = verts[4] as Integer

				if (verboseMode) then
				(
				    polyName = obj.name + i as String;
			    	format "      P '%' ( " polyName to:outFile
				) else
					format "      P '' ( " to:outFile

				format "      V(%,%,%,%) \n" (a-1) (b-1) (c-1) (d-1) to:outFile

				if (debug) then format "      V(%,%,%,%) \n" (a-1) (b-1) (c-1) (d-1)

				-- if no mapping close and go to next obj
				if (not Tmapping) then
				(
					format "      )\n" to:outFile
					continue
				)

				-- output UV Mapping with origin in bottom-left corner
				format "      TEXTURE (UV (" to:outFile
				f = 0
				outputV = 0
				for h in #(3,2,1,4) do
				(
					if (verts[h]==verts2[additionalVertex]) then
					(
						f=f+1
						continue
					)
					else
					(
					    u = (Tverts[h])[1]
						v = (1-(Tverts[h])[2])
						format "%,%,%" f u v to:outFile
						outputV = outputV + 1
					)

					if (outputV<3) then format "," to:outFile

					f=f+1
				)
				format ")) )\n" to:outFile

				--aTV = Tverts[3]
				--bTV = Tverts[2]
				--cTV = Tverts[1]
				--dTV = Tverts[4]
				--dTV = getTVert obj textVert2[additionalVertex]

				--format "      TEXTURE (UV (0,%,%,2,%,%,3,%,%)) )\n" aTV[1] (1-aTV[2]) bTV[1] (1-bTV[2]) cTV[1] (1-cTV[2]) to:outFile
				--format "      TEXTURE (UV (0,%,%,1,%,%,2,%,%)) )\n" aTV[1] (1-aTV[2]) bTV[1] (1-bTV[2]) cTV[1] (1-cTV[2]) to:outFile

				continue

		   )

		   --////////////////////
		   --poly can't be combined
		   --////////////////////
		   else
		   	if (debug) then format "POLY can't be combined\n"

			-- get its 3 vertices as a point3
			-- export in XZY format
		    verts=getface obj i
			a = verts[1] as Integer
			b = verts[3] as Integer
			c = verts[2] as Integer

				if (verboseMode) then
				(
				    polyName = obj.name + i as String;
			    	format "      P '%' ( " polyName to:outFile
				) else
					format "      P '' ( " to:outFile

			format "      V(%,%,%) \n" (a-1) (b-1) (c-1) to:outFile

			-- no TVerts assigned
		   if (getNumTVerts obj==0) then
		   (
		   		format "No UV maps assigned to object: %\n" obj.name
				format "      )\n" to:outFile
				continue
		   )

			-- output UV Mapping with origin in bottom-left corner
	        TVerts = getTVFace obj i
			aTV = getTVert obj TVerts[1]
			bTV = getTVert obj TVerts[3]
			cTV = getTVert obj TVerts[2]

			format "      TEXTURE (UV (0,%,%,1,%,%,2,%,%)) )\n" aTV[1] (1-aTV[2]) bTV[1] (1-bTV[2]) cTV[1] (1-cTV[2]) to:outFile

		)
	)
 )

 -- close params
 format "      )\n" to:outFile
 
 -- close mesh object
 format "    )\n" to:outFile
 
 format "\n" to:outFile

  --print (getUserPropBuffer obj) to:outFile -- output buffer as a string literal 
) 

-- outputs lights
for ll in lightsFound do
(
    format " ;Light: % \n" ll.name to:outFile
	format "    LIGHT (\n" to:outFile

	xlight = ll.pos.x * xscale
	ylight = ll.pos.y * yscale
	zlight = ll.pos.z * zscale
	llradius = ll.farAttenEnd * xscale
	
	format "      CENTER (%,%,%)\n" xlight zlight ylight to:outFile
    format "      RADIUS(%)\n" llradius to:outFile
	format "      COLOR (%,%,%) \n" ll.rgb.r ll.rgb.g ll.rgb.b to:outFile
	format "    )\n"  to:outFile
)

-- outputs camera
-- TODO

 -- close sector object
 format "  )\n" to:outFile

-- outputs camera

 if (startingPos != null) then
 (
	xstart = startingPos.pos.x * xscale
	ystart = startingPos.pos.y * yscale
	zstart = startingPos.pos.z * zscale

	format "  START '%' ('room', %, %, %)\n" startingPos.name xstart zstart ystart to:outFile
 )

 -- close world object
 format ")\n" to:outFile
 

close outFile 

format "ALL DONE!\n"
	