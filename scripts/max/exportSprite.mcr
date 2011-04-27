------------------------------------------------------------
-- Author: Luca Pancallo <pancallo@netscape.net>
--	
-- Copyright (C) 2007 Atomic Blue (info@planeshift.it, 
-- http://www.atomicblue.org , http://www.planeshift.it)
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
-- Version 01

macroScript Export_Sprite_CS
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Export Sprite to CS" 
tooltip:"Export Sprite to CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export Sprite" width:238 height:128
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export Sprite To:" pos:[21,7] width:142 height:20
	button btn2 "Export!" pos:[39,72] width:152 height:24



		-- parameters for scaling and relocation
		global xscale = 1.0
		global yscale = 1.0
		global zscale = 1.0
	
		global xrelocate = 0
		global yrelocate = 0
		global zrelocate = 0




		fn OutputGenMeshFactory obj outFile debug auto =
        (
			factoryName = obj.name

			-------------------
			-- support for LOD.
			-- it has to check all objects listed and export each as a nested meshfact
			-------------------
			lodlow = getUserProp obj "LODLOW"
			if (lodlow!=undefined) then (
				format "FOUND LOD OBJECT: %" factoryName

  					-- search low detail obj
				lodobjects = #()
				if (lodlow=="NULL") then
					append lodobjects "null"
				else
					append lodobjects (getNodeByName lodlow)

  					-- search med detail obj
				lodmed = getUserProp obj "LODMED"
				if (lodmed=="NULL") then
					append lodobjects "null"
				else
					append lodobjects (getNodeByName lodmed)

  					-- search high detail obj
				append lodobjects obj

			-------------------------
			-- no LOD for this object
			-------------------------
			) else (
				lodobjects = #()
				append lodobjects obj
			)

  					-- export low, med and high version
			if (lodobjects.count!=1) then
			(
				format "<meshfact name=\"%\">\n" factoryName to:outFile
				-- get proper nullmesh bbox size
				rmin = [-1,-1,-1]
				rmax = [1,1,1]
				for k=1 to lodobjects.count do
				(
					if (lodobjects[k]!="null") then
					(
						tempmin = lodobjects[k].min - lodobjects[k].center
						tempmax = lodobjects[k].max - lodobjects[k].center
						if (tempmin.x<rmin.x) then rmin.x=tempmin.x
						if (tempmin.y<rmin.y) then rmin.y=tempmin.y
						if (tempmin.z<rmin.z) then rmin.z=tempmin.z
						if (tempmax.x>rmax.x) then rmax.x=tempmax.x
						if (tempmax.y>rmax.y) then rmax.y=tempmax.y
						if (tempmax.z>rmax.z) then rmax.z=tempmax.z
					)
				)
				rmin = (rmin * xscale) + xrelocate
				rmax = (rmax * xscale) + xrelocate
				format "<nullmesh><min x=\"%\" y=\"%\" z=\"%\" /><max x=\"%\" y=\"%\" z=\"%\" /></nullmesh>\n" rmin.x rmin.z rmin.y rmax.x rmax.z rmax.y to:outFile
				format "<staticlod><distance varm=\"LodM\" vara=\"LodA\" /></staticlod>\n" to:outFile
			)

			for k=1 to lodobjects.count do
			(

				--check simple colldet setting
				colldet = getUserProp obj "COLLDET"

		   		-- output nullmesh
				if (lodobjects[k]=="null") then
				(
					format "  <meshfact name=\"%_%\">\n" factoryName (k-1) to:outFile
					format "  <lodlevel>%</lodlevel>\n" (k-1) to:outFile
					format "  <nullmesh><min x=\"-0.1\" y=\"-0.1\" z=\"-0.1\" /><max x=\"0.1\" y=\"0.1\" z=\"0.1\" /></nullmesh>\n" to:outFile
					format "  </meshfact>\n\n" to:outFile
					continue;
				)

				obj = lodobjects[k]

				-- output normalmeshfact
				-- meshfact name changes for hierarchy
				if (lodlow!=undefined) then
					format "  <meshfact name=\"%_%\">\n" factoryName (k-1) to:outFile
				else
					format "  <meshfact name=\"%\">\n" factoryName to:outFile

				if (lodobjects.count!=1) then
					format "<lodlevel>%</lodlevel>\n" (k-1) to:outFile
				format "  <plugin>crystalspace.mesh.loader.factory.genmesh</plugin><params>\n" to:outFile

				-- just for LOD hierarchical objects material must be speficied in the factory
				if (lodlow!=undefined) then (
					m = obj.mat;
					if ((classOf m)==Multimaterial) then (
						matUsed = getFaceMatID obj 1
						m = obj.mat[matUsed];
					)
					format "      <material>%</material>\n" (getMatDiffuseMapFilename m) to:outFile
				)

				-- check texture
				if (getNumTVerts obj==0) then
				(
					message = "Export aborted: No UV maps assigned to genmesh object: " + factoryName
					messageBox message
					close outFile 
					return 1
				)

				-- checks if model has left-oriented system or not
				face = getface obj 1
				v1= getvert obj face[1]
				v2 = getvert obj face[2]
				v3 = getvert obj face[3]
				

				vect1 = v1-v2
				vect2 = v3-v2
				normal1 = cross vect1 vect2
				facenorm = normal1/(length normal1)
				maxnorm = getfacenormal obj 1
				flipModel = false
	
				dotProd = dot facenorm maxnorm
				if (dotProd>0) then (
					flipModel = true
				)
	
			   if (flipModel) then (
			    format "\n GenMesh Factory Object needs flipping: %\n" factoryName
			   )
			   
				vertTInfo = #(getNumTVerts obj)
				vertTInfo[1] = undefined
				normalsVert = #()
	
				isSky = false
				if (findString obj.name "_sky_" !=undefined) then
				 isSky = true

				-- cycle on all Faces of the object to get Tverts x,y,z positions
				for i=1 to obj.numfaces do
				(
					-- get face
					Tface = getTVFace obj i
					face = getFace obj i

					-- check smooth group of the face
					smoothgroup = getFaceSmoothGroup obj i

					-- get its 3 vertices
					for h=1 to 3 do (
				    	curVert=getVert obj face[h]
						if (curVert==undefined) then
							format "\n\nUNDEF: %\n\n" Tface[h]
						if (debug) then
							format " face: % curVert %: %\n" i Tface[h] curVert

						-- if the value is different we have a problem on Welded UV
						if (vertTInfo[Tface[h]]!=undefined and vertTInfo[Tface[h]]!=curVert) then (
							message = "PROBLEM on object " + factoryName + ".\n UV should not be welded on vertex " + (Tface[h] as String)
							messageBox message
							close outFile
							return 1
						)
						vertTInfo[Tface[h]]=curVert

						-- get normal for sky (TODO: double check, seems not right)
						if (isSky) then (
							normalsVert[Tface[h]]=getNormal obj face[h]
						-- get normal in case of smoothing groups
                        ) else if (smoothgroup!=0) then (
							-- get average normal
							faces = meshop.getFacesUsingVert obj face[h] as array
							normal = [0,0,0]
							facescount = 0
							for j in faces do (
							    if (getFaceSmoothGroup obj j==smoothgroup) then
									normal += (getFaceNormal obj j)
									facescount+=1
							)
							normal = normal/facescount
							normalsVert[Tface[h]]=normal
						-- get normal in case no smooth group present
						) else (
							--normalsVert[Tface[h]]=getNormal obj face[h]
							normal = meshop.getFaceRNormals obj i
							normalsVert[Tface[h]]=normal[1]
						)
					)
				)

				-- cycle on all TVerts of the object
				for i =1 to (getNumTVerts obj) do
				(
					-- get its 3 vertices as a point3
					-- export in XZY format
				    vert=vertTInfo[i]
					if (vert==undefined) then (
						message = "PROBLEM on object " + factoryName + ": UV mapping seems to be messed up. Please UnWrap it then collapse it.";
						messageBox message
						close outFile
						return 1
					)
					xvert = (vert.x * xscale) + xrelocate
					yvert = (vert.y * yscale) + yrelocate
					zvert = (vert.z * zscale) + zrelocate
		
					Tvert = getTVert obj i
					
					-- if it's an autogenerated genmesh then relocate to 0,0,0
					if (auto==1) then (
						xvert = xvert - obj.pos.x
						yvert = yvert - obj.pos.y
						zvert = zvert - obj.pos.z
					)

					Tvert = getTVert obj i
					normalvert = normalsVert[i]
					normalvert = normalize(normalvert)


					format "      <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" nx=\"%\" ny=\"%\" nz=\"%\" /> \n" xvert zvert yvert Tvert[1] (1-Tvert[2]) normalvert.x normalvert.z normalvert.y to:outFile
				)

				-- write all normals (Moved into the above cycle based on Jorrit inputs 2008.5.21)
				-- for i =1 to (getNumTVerts obj) do
				-- (
				-- 	Tvert = getTVert obj i
				-- 	normalvert = normalsVert[i]
				-- 	normalvert = normalize(normalvert)
				-- 	format "      <n x=\"%\" y=\"%\" z=\"%\" /> \n" normalvert.x normalvert.z normalvert.y to:outFile
				-- )

				-- cycle on all faces of object and split by material
				facesmaterial=#()
				facesmaterialfaces=#()
                -- if standard material, then 1 submesh only
				if ((classOf obj.mat)==Standardmaterial) then (
				    append facesmaterial (getMatDiffuseMapFilename (obj.mat))
					facesmaterialfaces[1]=#()
					for i =1 to obj.numFaces do
					(
						append facesmaterialfaces[1] i
					)
				) else (
					for i =1 to obj.numFaces do
					(
					    textid = getFaceMatID obj i
						if (textid > obj.mat.count) then (
						  format "ERROR!!! Object % uses more material ID than the materials defined. \n" obj.name
						)
						--format "% % \n" i textid
						m = obj.mat[textid]
	   					mname = getMatDiffuseMapFilename (m)
						if (findItem facesmaterial mname==0) then (
							append facesmaterial mname
						)
						pos = findItem facesmaterial mname
						if (facesmaterialfaces[pos]==undefined) then
						   facesmaterialfaces[pos]=#()
					    append facesmaterialfaces[pos] i
					)
				)
				format "\n facesmaterial: % \n" facesmaterial
				format "\n facesmaterialid: % \n" facesmaterialfaces

				-- write one submesh for each material
				for i =1 to facesmaterial.count do
				(
					mname = facesmaterial[i];

					format "    <submesh name=\"%\">\n" ("submesh"+i as String) to:outFile
                    format "    <material>%</material>\n" mname to:outFile
                    format "    <indexbuffer components=\"1\" type=\"uint\" indices=\"yes\">\n" to:outFile

                    -- cycle on all faces using same material
                    for j=1 to facesmaterialfaces[i].count do
					(
						currentfaceset = facesmaterialfaces[i]
						faceVerts=getTVface obj currentfaceset[j]
						a = (faceVerts[1]-1) as Integer
	
						if (flipModel) then (
							b = (faceVerts[2]-1) as Integer
							c = (faceVerts[3]-1) as Integer
						) else (
							b = (faceVerts[3]-1) as Integer
							c = (faceVerts[2]-1) as Integer
						)
	
						-- exclude collision detection on low lod versions
						if (k!=lodobjects.count) then (
							format "    <e c0=\"%\" /><e c0=\"%\" /><e c0=\"%\" />\n" a b c to:outFile
							--if (colldet=="no") then format "<colldet>no</colldet>" to:outFile
						) else (
							--if (colldet=="no") then format "<t v1=\"%\" v2=\"%\" v3=\"%\"><colldet>no</colldet></t>"  a b c to:outFile
							format "    <e c0=\"%\" /><e c0=\"%\" /><e c0=\"%\" />\n" a b c to:outFile
						)
					)
					format "    </indexbuffer></submesh>\n" to:outFile
			    )


				-- check for smooth setting
				-- temporary disabled
				--smooth = getUserProp obj "SMOOTH"
				--if (smooth=="yes") then
				--	format "      <autonormals/>\n" to:outFile
					
				format "    </params>\n" to:outFile
	
				-- check for no lighting setting
				lighting = getUserProp obj "LIGHTING"

				--check simple colldet setting
				colldet = getUserProp obj "COLLDET"

				-- check for viscull setting
				viscull = getUserProp obj "VISCULL"

				-- check for culleronly setting
				culleronly = getUserProp obj "CULLERONLY"

				-- check player barrier setting
				playerbarrier = getUserProp obj "PLAYERBARRIER"

                occluder = false		
				-- check for culleronly setting
				culleronly = getUserProp obj "CULLERONLY"

				-- check player barrier setting
				playerbarrier = getUserProp obj "PLAYERBARRIER"

			    -- output name of object and some info
				if (findString obj.name "_occ_" !=undefined) then (
					occluder = true
				)

				istrasparent=false
				-- check if object is a range alpha trasparent
				rangetrasp = getUserProp obj "RANGETRASP"
				if (rangetrasp=="yes") then (
				  istrasparent=true
				)

				-- check if object is a binary alpha trasparent
				trasp = getUserProp obj "TRASPARENT"
				if (trasp=="yes") then (
				  istrasparent=true
				)

				-- handles transparent objects
				if (istrasparent) then (
				  format "      <priority>alpha</priority>\n" to:outFile
				  format "      <ztest />\n" to:outFile
				)
			    -- handles sky objects
				else if (findString obj.name "_sky_" !=undefined) then (
				  format "      <priority>sky</priority>\n" to:outFile
				  format "      <zuse />\n" to:outFile
				)
			    -- handles zfill objects
				else if (findString obj.name "_s_" !=undefined) then (
				  format "      <priority>object</priority>\n" to:outFile
				  format "      <zuse />\n" to:outFile
				)
				else if (culleronly=="yes" or playerbarrier=="yes" or (occluder and (classOf obj)!=Box)) then (
					format "      <mesh>\n" to:outFile
				) else if (occluder and (classOf obj)==Box) then (
					format "      <box>\n" to:outFile
				) else (
				  format "      <priority>object</priority>\n" to:outFile
				  format "      <zuse />\n" to:outFile
				)

				-- check for no shadow setting
				noshadows = getUserProp obj "NOSHADOWS"
				if (noshadows=="yes") then
					format "      <noshadows />\n" to:outFile


				if (colldet=="no") then format "<trimesh><id>colldet</id></trimesh>\n" to:outFile

				-- check for no shadow setting
				-- NO MORE WORKING??
				noshadows = getUserProp obj "NOSHADOWS"
				if (noshadows=="yes") then
					format "      <noshadows />\n" to:outFile


				-- check if object is a range alpha trasparent
				rangetrasp = getUserProp obj "RANGETRASP"
				if (rangetrasp=="yes") then (
				  format "      <ztest/><priority>alpha</priority>\n" to:outFile
				)

				-- check if object uses a binary alpha texture
			    if ((classOf obj.mat)==Standardmaterial) then (
					if (obj.mat.mapEnables[7]) then (
					  format "      <trimesh><id>viscull</id></trimesh>\n" to:outFile
					  format "      <trimesh><default /><id>colldet</id></trimesh>\n" to:outFile
					)
				) else (
					if (obj.mat[1].mapEnables[7]) then (
					  format "      <trimesh><id>viscull</id></trimesh>\n" to:outFile
					  format "      <trimesh><default /><id>colldet</id></trimesh>\n" to:outFile
					)
				)



				format "    </meshfact>\n" to:outFile

			) -- end for each lodobject

			if (lodobjects.count!=1) then
				format "  </meshfact>\n" to:outFile

        )



	on Test1 open do
	(
	   name = selection[1].name
	   edt3.text = "d:\\Luca\\"+name+".meshfact"

	)
	on btn2 pressed do
	(
	
		-- ////////////////////////
		-- Variables used in the program
		-- ////////////////////////
	
		-- get filename
		filename = edt3.text
	
		-- output file
		outFile = createFile filename
	
		-- set debug output
		debug=false
	
		-- Define verbose output (that takes more space and memory)
		verboseMode = true
	
		-- functions declaration
		global tokenize
		global OutputGenMeshFactory
		global getMatDiffuseMapFilename
		global getMatSpecMapFilename
		global getMatNormalMapFilename
		global getMatDispMapFilename
		global lowercase
	
		-- Tokenize utility function
		fn tokenize instring sep = 
		(
		   outarray = #()
		   temp=copy instring
		   i = 1
		   while (true) do (
		   	index = findstring temp sep
			if (index==undefined) then
			(
				outarray[i]=temp
				exit
			) else (
				outarray[i] = substring temp 1 (index-1)
				temp = substring temp (index+1) -1
			)
			i = i +1
		   )
		   outarray
		)
	
		-- get filename from a material
		fn getMatDiffuseMapFilename m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.diffuseMap
				if (mat!=undefined) then
				(
					image = mat.filename
					indx = tokenize image "\\"
					image = indx[indx.count]
				) else
					image="materialnotdefined"

				image = lowercase(image)
			)
		)

            fn getMatSpecMapFilename m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.specularMap
				if (mat!=undefined) then
				(
					image = mat.filename
					indx = tokenize image "\\"
					image = indx[indx.count]
				) else
					image="materialnotdefined"

				image = lowercase(image)
			)
		)
		
		fn getMatNormalMapFilename m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.bumpMap
				if (mat!=undefined) then
				(
					image = mat.filename
					indx = tokenize image "\\"
					image = indx[indx.count]
				) else
					image="materialnotdefined"

				image = lowercase(image)
			)
		)
		
	      fn getMatDispMapFilename m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.displacementMap
				if (mat!=undefined) then
				(
					image = mat.filename
					indx = tokenize image "\\"
					image = indx[indx.count]
				) else
					image="materialnotdefined"

				image = lowercase(image)
			)
		)
	
		-- LowerCase utility function
		fn lowercase instring = 
		(
		   local upper, lower, outstring
		   upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		   lower="abcdefghijklmnopqrstuvwxyz" 
		
		   outstring=copy instring 
		
		   for i=1 to outstring.count do 
		   (  j=findString upper outstring[i] 
		      if (j != undefined) do outstring[i]=lower[j] 
		   )

		   outstring
		)
	
	
		-- ////////////////////////
		-- Main: program starts here
		-- ////////////////////////
		
			-- identify main sprite object
			obj=selection[1]
			objectname = obj.name
	
		    -- output name of object and some info
			format "\n\nFound Object Name: % Faces: %\n" objectname obj.numfaces
		
	
	   format "<library>\n" to:outFile
	   
	   -- search all materials
		facesmaterial=#()
		if ((classOf obj.mat)==Standardmaterial) then (
		    append facesmaterial (obj.mat)
		) else (
			for i =1 to obj.numFaces do
			(
			    textid = getFaceMatID obj i
				if (textid > obj.mat.count) then (
				  format "ERROR!!! Object % uses more material ID than the materials defined. \n" obj.name
				)
				--format "% % \n" i textid
				m = obj.mat[textid]
				if (findItem facesmaterial m==0) then
                        (
				    append facesmaterial m
				)
			)
		)

		-- export all textures
		format "  <textures>\n" to:outFile

		for material in facesmaterial do
		(
			-- Diffuse Texture:
			diffuseImage = getMatDiffuseMapFilename material				
			format "    <texture name=\"%\">\n" diffuseImage to:outFile
			format "      <file>%</file>\n" diffuseImage to:outFile

			-- handles transparent materials					
			if (material.mapEnables[7]) then
			(
				format "      <alpha>\n" to:outFile
				format "        <binary/>\n" to:outFile
				format "      </alpha>\n" to:outFile
			)
			format "    </texture>\n" to:outFile
           	         
			-- Spec Map Texture:
			specMapImage = getMatSpecMapFilename material
			if (specMapImage!="materialnotdefined") then
			(
				format "    <texture name=\"%\">\n" specMapImage to:outFile
				format "      <file>%</file>\n" specMapImage to:outFile
				format "    </texture>\n" to:outFile			
			)

			-- Normal Map Texture:
			normalMapImage = getMatNormalMapFilename material
			if (normalMapImage!="materialnotdefined") then
			(
			    format "    <texture name=\"%\">\n" normalMapImage to:outFile
			    format "      <file>%</file>\n" normalMapImage to:outFile
			    format "      <class>normalmap</class>\n" to:outFile
			    format "    </texture>\n" to:outFile
			)

			-- Displacement Map Texture:
			dispMapImage = getMatDispMapFilename material
			if (dispMapImage!="materialnotdefined") then
			(
				format "    <texture name=\"%\">\n" dispMapImage to:outFile
				format "      <file>%</file>\n" dispMapImage to:outFile
				format "    </texture>\n" to:outFile
			)
		)

		format "  </textures>\n" to:outFile

		-- export all materials
		format "  <materials>\n" to:outFile

		for material in facesmaterial do
		(
			diffuseMapImage = getMatDiffuseMapFilename material
                  format "    <material name=\"%\">\n" diffuseMapImage to:outFile

			-- handles transparent materials
			if (material.mapEnables[7]) then
                  (
				format "      <shader type=\"depthwrite\">*null</shader>\n" to:outFile
				format "      <shader type=\"base\">lighting_default_binalpha</shader>\n" to:outFile
				format "      <shader type=\"diffuse\">lighting_default_binalpha</shader>\n" to:outFile
			)

                  -- handles diffuse maps
                  format "      <shadervar type=\"texture\" name=\"tex diffuse\">%</shadervar>\n" diffuseMapImage to:outFile

                  -- handles normal maps
                  normalMapImage = getMatNormalMapFilename material 
                  if(normalMapImage!="materialnotdefined") then
                  (
                      format "      <shadervar type=\"texture\" name=\"tex normal compressed\">%</shadervar>\n" normalMapImage to:outFile
                  )

                  -- handles displacement(height) maps
                  dispMapImage = getMatDispMapFilename material 
                  if(dispMapImage!="materialnotdefined") then
                  (
                      format "      <shadervar type=\"texture\" name=\"tex height\">%</shadervar>\n" dispMapImage to:outFile
                  )

                  -- handles specular maps
                  specMapImage = getMatSpecMapFilename material 
                  if(specMapImage!="materialnotdefined") then
                  (
                      format "      <shadervar type=\"texture\" name=\"tex specular\">%</shadervar>\n" specMapImage to:outFile
                  )
                             
                  format "    </material>\n" to:outFile
		)

		format "  </materials>\n" to:outFile

	    OutputGenMeshFactory obj outFile debug 0

	   format "</library>\n" to:outFile
		
		close outFile 
		message = "ALL DONE!"
		
		messageBox message
	
	)
)
gw = newRolloutFloater "Export Sprite" 300 410 
addRollout Test1 gw 


)

---------------------------------------------------------------------------------------
--  Documentation
--
--
-- Faces that need UV Unwield: 
-- This means that some UV coords are shared on multiple vertices
-- Go in the UVUnwrap function and Unweld all the vertexes of the face found
-------------------
