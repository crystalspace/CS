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


macroScript Export_Level_CS
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Export Level CS" 
tooltip:"Export Level CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export Level to CS" width:226 height:450
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export Level To:" pos:[21,7] width:142 height:20
	button btn2 "Export!" pos:[37,110] width:152 height:24
	label lbl6 "Scale:" pos:[58,72] width:40 height:20
	edittext edtScale "" pos:[101,67] width:63 height:27
	checkbox chkLights "Generate Fake lights for walktest" pos:[6,179] width:215 height:20 enabled:true
	label lbl3 "Duration (msecs):" pos:[16,207] width:108 height:20
	edittext edtDuration "" pos:[128,206] width:76 height:22
	checkbox chk2 "Copy textures to dest dir" pos:[6,152] width:210 height:22
	GroupBox grp2 "Sanity Check" pos:[10,243] width:201 height:94
	checkbox chkSanity "Check Only, no Deletion" pos:[22,267] width:159 height:23 enabled:true checked:true
	button sanity "SanityCheck!" pos:[32,297] width:145 height:31
	label lblVersion "V." pos:[180,5] width:40 height:21

	groupBox grp3 "Terrain" pos:[13,348] width:189 height:89
	checkbox chkTerrain "Export as Terrain Level" pos:[22,372] width:168 height:26
	on Test1 open do
	(
	   version = 48 as String
	   lblVersion.text = "V."+version
	
		-- get room name from custom property
		customPropNumber = fileProperties.findProperty #custom "roomname"
		if (customPropNumber==0) then (
			messageBox "Please click on File>File Properties and add a Custom Property called roomname with the name of the sector."
			return 1
		)
		roomName = fileProperties.getPropertyValue #custom customPropNumber 
	
		-- get default scale from custom property
		customPropNumber = fileProperties.findProperty #custom "scale"
		if (customPropNumber==0) then (
			messageBox "Please click on File>File Properties and add a Custom Property called \"scale\" with the scale of the sector.\n 1 generic unit=1 meter. You can model at higher scale, then specify scale=0.1 or 0.01"
			return 1
		)
		defaultScale = fileProperties.getPropertyValue #custom customPropNumber 
	
	   edt3.text = "D:\Luca\PS\gfxpackage3.4\levels\\"+roomName+"\world"
	   edtScale.text = defaultScale
	)
	on btn2 pressed do
	(
	
		-- ////////////////////////
		-- Variables used in the program
		-- ////////////////////////
	
		-- get filename
		filename = edt3.text
		-- set debug output
		debug=false
		debug2=false

		-- define if the program should combine 2 planar triangles into one polygon
		polyCombine = true
	
		-- Define verbose output (that takes more space and memory)
		verboseMode = true
	
		-- parameters for scaling and relocation
		global xscale = edtScale.text as Float
		global yscale = edtScale.text as Float
		global zscale = edtScale.text as Float
	
		global xrelocate = 0
		global yrelocate = 0
		global zrelocate = 0
	
		-- functions declaration
		global tokenize
		global lowercase
		global getMatFilename
		global getMatFullPath
	
		-- particle variables
		global fireNeeded = false

		global emitNeeded = false
		global partMaterials = #()



	
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
		fn getMatFilename m = 
		(
		    if (m==undefined) then
				image="MATERIALNOTDEFINED"
			else (
			    mat = m.maps[2]
				if (mat!=undefined) then
				(
					image = mat.filename
					indx = tokenize image "\\"
					image = indx[indx.count]
				) else
					image="MATERIALNOTDEFINED"
			)
		)
	
		-- get fullpath of a material
		fn getMatFullPath m = 
		(
		    if (m==undefined) then
				image="MATERIALNOTDEFINED"
			else (
			    mat = m.maps[2]
				if (mat!=undefined) then
				(
					image = mat.filename
				) else
					image="MATERIALNOTDEFINED"
			)
		)
	
		-- ////////////////////////
		-- Write material function
		-- ////////////////////////
		
		fn WriteMaterials terrainobject outFile =
		(
		
		  --/// TEXTURES
		  
		  materialsWrittenToWorld=#()
		
		  format "  <textures>\n" to:outFile
	
		  for m in sceneMaterials do
		  (
		    -- handle Standardmaterials
			if ((classOf m)==Standardmaterial) then (
				-- if material not written, add it
				image = getMatFilename m
				if (findItem materialsWrittenToWorld image==0 and image!="MATERIALNOTDEFINED") then (
				format "m: % \n" m
					-- handles trasparent materials
					if (m.mapEnables[7]) then
						format "    <texture name=\"%\"> <file>%</file><alpha><binary/></alpha></texture>\n" image image to:outFile
					else 
						format "    <texture name=\"%\"> <file>%</file></texture>\n" image image to:outFile

					append materialsWrittenToWorld image
				)
			)
	
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatFilename subm
					if (findItem materialsWrittenToWorld image==0 and image!="MATERIALNOTDEFINED") then (
						format "m: % subm: % \n" m subm
						if (subm.mapEnables[7]) then
							format "    <texture name=\"%\"> <file>%</file><alpha><binary/></alpha></texture>\n" image image to:outFile
						else 
							format "    <texture name=\"%\"> <file>%</file></texture>\n" image image to:outFile
						append materialsWrittenToWorld image
					)
				)
			)
		
		  )
		
		  if (fireNeeded or emitNeeded) then (
			    format "    <texture name=\"raindrop\"> <file>raindrop.png</file></texture>\n" to:outFile  
		  )
	
		  -- handle additional textures for particles  
		  for m in partMaterials do
		  (
			    format "    <texture name=\"%\"> <file>%</file></texture>\n" m m to:outFile
		  )


		  format "  </textures>\n" to:outFile
	
	
		  --/// MATERIALS
		
		  materialsWrittenToWorld=#()
		
		  format "  <materials>\n" to:outFile
		
		  for m in sceneMaterials do
		  (
		    -- handle Standardmaterials
			if ((classOf m)==Standardmaterial) then (
				-- if material not written, add it
				image = getMatFilename m
				if (findItem materialsWrittenToWorld image==0 and image!="MATERIALNOTDEFINED") then (
				    format "    <material name=\"%\"> <texture>%</texture></material>\n" image image to:outFile
					append materialsWrittenToWorld image
				)
			)
		
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatFilename subm
					isshader = false

					-- skip wrong materials
					if ( image=="MATERIALNOTDEFINED") then
						continue;

					-- determine if it's a shader material
					if (findString subm.name "_shader_"!=undefined) then
						isshader = true

					-- check if already written
					found = false
					imagetemp = image
					if (isshader) then (
						imagetemp = image + "sshhaaddeerr"
						if(findItem materialsWrittenToWorld imagetemp!=0) then
							found=true
					) else (
						if(findItem materialsWrittenToWorld image!=0) then
							found=true
					)
					
					if (not found) then (

						format "    <material name=\"%\"> <texture>%</texture>\n" imagetemp image to:outFile
						-- manage shaders
						if (findString subm.name "_shader_" != undefined) then (
							if (findString subm.name "_shader_terrain" !=undefined) then (
								format "     <shadervar name=\"texture scale\" type=\"vector2\">16,16</shadervar>\n" to:outFile
								format "     <shader type=\"terrain splat\">terrain_fixed_splatting</shader>\n" to:outFile
							)
							else if (findString subm.name "_shader_ambient" !=undefined) then (
								format "     <shader type=\"ambient\">terrain_fixed_base</shader>\n" to:outFile
							) else (
								message = "ERROR: Found terrain's material "+m.name+" with submat "+subm.name+" defining a shader of unsupported type!\n"
								message = message + "Supported types are: _shader_terrain , _shader_ambient"
								messageBox message
								format "     </material>\n" to:outFile
								return 1
							)

						) else
							append materialsWrittenToWorld imagetemp

						format "    </material>\n" image image to:outFile

					)
				)
			)
		  )

	
		  if (fireNeeded or emitNeeded) then (
			    format "    <material name=\"raindrop\"> <texture>raindrop</texture></material>\n" to:outFile  
		  )
	
		  -- handle additional materials for particles
		  for m in partMaterials do
		  (
			    format "    <material name=\"%\"> <texture>%</texture></material>\n" m m to:outFile
		  )

		  format "  </materials>\n\n" to:outFile
		
		)

		-- ////////////////////////
		-- Write shaders function
		-- ////////////////////////
		
		fn WriteShaders outFile =
		(

		  -- add shaders needed for terrain
		  if (chkTerrain.checked) then (
		   filestr = openFile "ps_terrain.shaders.txt"
		   if (filestr==undefined) then (
				message = "ERROR: ps_terrain.shaders.txt not found!"
				messageBox message
				return 1
		   )
		   while (not eof filestr) do
		   (
		   	line = readLine filestr
			format "%\n" line to:outFile
		   )
		  )

		)
		

		-- ////////////////////////
		-- Output Particle function
		-- ////////////////////////
		fn OutputParticle obj allObjects outFile =
		(
				type = getUserProp obj "TYPE"
				partMaterial = getUserProp obj "MATERIAL"
				mixmode = getUserProp obj "MIXMODE"
	
				xpart = (obj.pos.x * xscale) + xrelocate
				ypart = (obj.pos.y * yscale) + yrelocate
				zpart = (obj.pos.z * zscale) + zrelocate
	
		        --format "    ;Particle Name: % Type: %\n" obj.name type to:outFile
	
				-- //////
				-- Fire
				-- //////
				if (type == "fire") then (
					number = getUserProp obj "NUMBER"
					dropsize = getUserProp obj "DROPSIZE"
					lighting = getUserProp obj "LIGHTING"
					swirl = getUserProp obj "SWIRL"
					colorscale = getUserProp obj "COLORSCALE"
	
					format "    <meshobj name=\"%\">\n" obj.name to:outFile
					format "      <priority>alpha</priority>\n" to:outFile
					format "      <plugin>fire</plugin>\n" to:outFile
					format "      <ztest />\n" to:outFile
					format "      <move><v x=\"%\" y=\"%\" z=\"%\" /></move>\n" xpart zpart ypart to:outFile
					format "      <params>\n" to:outFile
					format "	    <factory>fireFact</factory> \n" to:outFile
					if (mixmode == undefined) then
						format "	    <mixmode><add /></mixmode>\n" to:outFile
					else
						format "	    <mixmode><% /></mixmode>\n" mixmode to:outFile
					if (partMaterial == undefined) then
						format "	    <material>raindrop</material>\n" to:outFile
					else
						format "	    <material>%</material>\n" partMaterial  to:outFile	
					format "	    <origin x=\"0\" y=\"0\" z=\"0\" />\n" to:outFile
	
					-- get parameters from UserProps
					format "	    <number>%</number> \n" number to:outFile
					format "	    <dropsize w=\"%\" h=\"%\" /> \n" dropsize dropsize to:outFile
					format "	    <lighting>%</lighting> \n" lighting to:outFile
					format "	    <swirl>%</swirl> \n" swirl to:outFile
					format "	    <colorscale>%</colorscale>\n" colorscale to:outFile
					format "      </params>\n" to:outFile
					format "    </meshobj>\n" to:outFile
	
				)
				-- //////
				-- Emit
				-- //////
				else if (type == "emit") then (
					number = getUserProp obj "NUMBER"
					regparticle = getUserProp obj "REGULARPARTICLES"
					if (regparticle==undefined) then
					(
						rectparticle = getUserProp obj "RECTPARTICLES"
						rectparticle2 = tokenize rectparticle ","
					) else (
						regparticle2 = tokenize regparticle ","
					)
					lighting = getUserProp obj "LIGHTING"
					totaltime = getUserProp obj "TOTALTIME"
					-- STARTPOS: STARTPOS1=EMITSPHERE  STARTPOS2=0,0.1
					startpostype = getUserProp obj "STARTPOS1"
					startpostype = lowercase startpostype
					startpos = getUserProp obj "STARTPOS2"
					index = findString startpos ","
					startposarray = tokenize startpos ","
					-- STARTSPEED=EMITBOX(-1,-1,-1,1,1,1)
					startspeed = getUserProp obj "STARTSPEED"
					index = findString startspeed "("
					startspeedtype = lowercase (substring startspeed 1 (index-1))
					startspeed = substring startspeed (index+1) -1
					startspeed = substring startspeed 1 ((startspeed.count)-1)
					startspeedarray = tokenize startspeed ","
					-- STARTACCEL=EMITFIXED(0,0,0)
					startaccel = getUserProp obj "STARTACCEL"
					index = findString startaccel "("
					startacctype = lowercase (substring startaccel 1 (index-1))
					startaccel = substring startaccel (index+1) -1
					startaccel = substring startaccel 1 ((startaccel.count)-1)
					startaccarray = tokenize startaccel ","
					attractor = getUserProp obj "ATTRACTOR"
					if (attractor!=undefined) then (
						-- search attractor position using attractor name found in ATTRACTOR entry
						attractorobj= getNodeByName attractor
								--OLD: for attract in allObjects do if (attract.name==attractor) then attractorobj=attract
						if (attractorobj==null) then ( format "NO OBJECT FOUND AS ATTRACTOR OF %" obj.name )
						xattractorobj = (attractorobj.pos.x * xscale) + xrelocate
						yattractorobj = (attractorobj.pos.y * yscale) + yrelocate
						zattractorobj = (attractorobj.pos.z * zscale) + zrelocate
						typeattractor = getUserProp attractorobj "TYPE"
						attractorforce = getUserProp obj "ATTRACTORFORCE"
					)
	
					-- fieldspeed
					fieldspeed = getUserProp obj "FIELDSPEED"
					format "fieldspeed: % \n" fieldspeed
					if (fieldspeed!=undefined) then (
						index = findString fieldspeed "("
						fieldspeedtype = lowercase (substring fieldspeed 1 (index-1))
						format "fieldspeedtype: % \n" fieldspeedtype
						fieldspeed = substring fieldspeed (index+1) -1
						fieldspeed = substring fieldspeed 1 ((fieldspeed.count)-1)
						fieldspeedarray = tokenize fieldspeed ","
					)

					aging0 = getUserProp obj "AGING0"
					if (aging0!=undefined) then
						aging0 = tokenize aging0 ","
					aging1 = getUserProp obj "AGING1"
					if (aging1!=undefined) then
						aging1 = tokenize aging1 ","
					aging2 = getUserProp obj "AGING2"
					if (aging2!=undefined) then
						aging2 = tokenize aging2 ","
					aging3 = getUserProp obj "AGING3"
					if (aging3!=undefined) then
						aging3 = tokenize aging3 ","
					aging4 = getUserProp obj "AGING4"
					if (aging4!=undefined) then
						aging4 = tokenize aging4 ","
		
					format "    <meshobj name=\"%\">\n" obj.name to:outFile
					format "      <priority>alpha</priority>\n" to:outFile
					format "      <plugin>emit</plugin>\n" to:outFile
					format "      <ztest />\n" to:outFile

					if (attractor!=undefined) then (
						movexpart = 0; moveypart = 0; movezpart = 0
						startposxpart = xpart; startposypart = ypart ; startposzpart = zpart
					) else (
						movexpart = xpart; moveypart = ypart ; movezpart = zpart
						startposxpart = 0; startposypart = 0 ; startposzpart = 0
					)

					format "      <move> <v x=\"%\" y=\"%\" z=\"%\" /> </move>\n" movexpart movezpart moveypart to:outFile

					format "      <params>\n" to:outFile
					format "	    <factory>emitFact</factory> \n" to:outFile
					if (mixmode == undefined) then
						format "	    <mixmode><add /></mixmode>\n" to:outFile
					else
						format "	    <mixmode><% /></mixmode>\n" mixmode to:outFile
					if (partMaterial == undefined) then
						format "	    <material>raindrop</material>\n" to:outFile
					else
						format "	    <material>%</material>\n" partMaterial  to:outFile			
	
					format "	    <number>%</number> \n" number to:outFile
					if (regparticle==undefined) then
					(
						format "	    <rectparticles w=\"%\" h=\"%\" /> \n" rectparticle2[1] rectparticle2[2] to:outFile
					) else (
						format "	    <regularparticles sides=\"%\" radius=\"%\" /> \n" regparticle2[1] regparticle2[2] to:outFile
					)
					format "	    <lighting>%</lighting> \n" lighting to:outFile
					format "	    <totaltime>%</totaltime> \n" totaltime to:outFile
					-- STARTPOS
					if (startpostype=="emitsphere") then (
						--pscaled = (startposarray[1]  * xscale) + xrelocate
						--qscaled = (startposarray[2]  * yscale) + yrelocate
						format "	    <startpos><% x=\"%\" y=\"%\" z=\"%\" p=\"%\" q=\"%\" /></startpos>\n" startpostype startposxpart startposzpart startposypart startposarray[1] startposarray[2] to:outFile
					) else if (startpostype=="emitfixed") then
						format "	    <startpos><% x=\"%\" y=\"%\" z=\"%\" /></startpos>\n" startpostype startposxpart startposzpart startposypart to:outFile
					else if (startpostype=="emitbox") then
					(
						format "	    <startpos><%><min x=\"%\" y=\"%\" z=\"%\" /> \n" startpostype startposarray[1] startposarray[2] startposarray[3] to:outFile
						format "	    <max x=\"%\" y=\"%\" z=\"%\" /></%></startpos> \n" startposarray[4] startposarray[5] startposarray[6] startpostype to:outFile
					)
					-- STARTSPEED
					if (startspeedtype=="emitbox") then
					(
	
						format "	    <startspeed><%><min x=\"%\" y=\"%\" z=\"%\" /> \n" startspeedtype startspeedarray[1] startspeedarray[2] startspeedarray[3] to:outFile
						format "	    <max x=\"%\" y=\"%\" z=\"%\" /></%></startspeed> \n" startspeedarray[4] startspeedarray[5] startspeedarray[6] startspeedtype to:outFile
					) else if (startspeedtype=="emitfixed") then
						format "	    <startspeed><% x=\"%\" y=\"%\" z=\"%\" /> </startspeed>\n" startspeedtype startspeedarray[1] startspeedarray[2] startspeedarray[3] to:outFile
					format "	    <startaccel><% x=\"%\" y=\"%\" z=\"%\" /></startaccel>\n" startacctype startaccarray[1] startaccarray[2] startaccarray[3] to:outFile

					-- ATTRACTOR
					if (attractor!=undefined) then (
						format "	    <attractor> <emitfixed x=\"%\" y=\"%\" z=\"%\" /> </attractor>\n" xattractorobj zattractorobj yattractorobj to:outFile
						format "	    <attractorforce>%</attractorforce>\n" attractorforce to:outFile
					)

					-- FIELDSPEED
                	if (fieldspeedtype=="emitcylindertangent") then
					(
						format "        <fieldspeed><% p=\"%\" q=\"%\">\n" fieldspeedtype fieldspeedarray[1] fieldspeedarray[2] to:outFile
						format "        <min x=\"%\" y=\"%\" z=\"%\" /><max x=\"%\" y=\"%\" z=\"%\" /> \n" fieldspeedarray[3] fieldspeedarray[4] fieldspeedarray[5] fieldspeedarray[6] fieldspeedarray[7] fieldspeedarray[8] to:outFile
						format "        </emitcylindertangent></fieldspeed>\n" to:outFile
					)

					if (aging0!=undefined) then
					(
						format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging0[1] aging0[2] aging0[3] aging0[4] to:outFile
						format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging0[5] aging0[6] aging0[7] aging0[8] to:outFile
					)
					if (aging1!=undefined) then
					(					
						format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging1[1] aging1[2] aging1[3] aging1[4] to:outFile
						format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging1[5] aging1[6] aging1[7] aging1[8] to:outFile
					)
					if (aging2!=undefined) then
					(	
						format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging2[1] aging2[2] aging2[3] aging2[4] to:outFile
						format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging2[5] aging2[6] aging2[7] aging2[8] to:outFile
					)
					if (aging3!=undefined) then
					(
						format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging3[1] aging3[2] aging3[3] aging3[4] to:outFile
						format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging3[5] aging3[6] aging3[7] aging3[8] to:outFile
					)
					if (aging4!=undefined) then
					(
						format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging4[1] aging4[2] aging4[3] aging4[4] to:outFile
						format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging4[5] aging4[6] aging4[7] aging4[8] to:outFile
					)
					format "      </params>\n" to:outFile
					format "    </meshobj>\n" to:outFile
				)
		)
	
		-- ////////////////////////
		-- copy textures to output dir
		-- ////////////////////////
		fn CopyTexturesToDir outFile =
		(
		  destDir = getFilenamePath edt3.text
		  destFile = filenameFromPath edt3.text
		  destDir = destDir + destFile + "textures"
		  format "makedir % \n" destDir
		  makeDir destDir
		  
		  materialsWrittenToWorld=#()
	
		  for m in sceneMaterials do
		  (
		    -- handle Standardmaterials
			if ((classOf m)==Standardmaterial) then (
				-- if material not written, add it
				image = getMatFullPath m
				if (findItem materialsWrittenToWorld image==0) then (
					destFile2 = filenameFromPath image
					destFile2 = destDir + "\\" + destFile2
					format "copy from % to % \n" image destFile2
					copyFile image destFile2
					append materialsWrittenToWorld image
				)
			)
	

		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatFullPath subm
					if (findItem materialsWrittenToWorld image==0) then (
						destFile2 = filenameFromPath image
						destFile2 = destDir + "\\" + destFile2
						format "copy from % to % \n" image destFile2
						copyFile image destFile2
						append materialsWrittenToWorld image
					)
				)
			)
		  )
		)
	
	
	
		-- ////////////////////////
		-- Output a Portal object
		-- ////////////////////////
		fn OutputPortal obj debug outFile =
		(
			format "    <portal name=\"%\">\n" obj.name to:outFile

			-- all portals as autoresolve
			format "    <autoresolve />\n" to:outFile

			-- check if poly is valid: for now should be 2 faces MAX!
			faces = getNumFaces obj
			vertsProp = getUserProp obj "VERTS"
			if (faces!=2 and vertsProp==undefined) then (
				message = "ERROR: If you want to add a portals with more than 2 triangles add a property called VERTS with list of verts clockwise."
				messageBox message
				return 1
			)

			verts = #()
			-- manage polygon portals
			if (faces!=2) then (
				toks = tokenize vertsProp ","
				for elem in toks do (
					vertPoly = getvert obj (elem as Integer)
					append verts (elem as Integer)
					format "extracted %: % \n" elem vertPoly
				)

			-- manage 2 faces portals
			) else (

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
		
				-- get faces verts
			    vertsFace1=getface obj 1
				vertsFace2=getface obj 2
		
				-- trasform to array (needed for findItem func)
				verts1 = #()
				verts2 = #()
				for h=1 to 3 do append verts1 vertsFace1[h]
				for h=1 to 3 do append verts2 vertsFace2[h]

				additionalVertex = 0
				oppositeVertex = 0
		
				if (debug) then format "Vertex of first face: %\n" verts1
		
				-- search non-common vertex on face2
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
		
				-- list vertexes in right order for resulting polygon
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
		
							addedAdd = true
		
							continue
						)
					 )
					
					-- add vertex
					append verts verts1[h]
		
					if (h==3 and (not addedAdd) ) then (
						append verts verts2[additionalVertex]	
					)
				)
		
				-- clock-wise vertexes listing for CS
				if (not flipModel) then (
					tmpVert = verts[2]
					verts[2] = verts[4]
					verts[4] = tmpVert
				)

			) -- end if 2 faces

			-- export in XZY format
			for v in verts do
			(
				currVert = getVert obj v
				--scale
				scaledx = (currVert.x * xscale) + xrelocate
				scaledy = (currVert.y * yscale) + yrelocate
				scaledz = (currVert.z * zscale) + zrelocate
				format "      <v x=\"%\" y=\"%\" z=\"%\" /> \n" scaledx scaledz scaledy to:outFile
				--format "      <v x=\"%\" y=\"%\" z=\"%\" /> \n" currVert.x currVert.z currVert.y to:outFile
			)
	
			-- search displacement target
			warp = getUserProp obj "WARPTARGET"
			if (warp!=undefined) then
			(
			    warptarget = getNodeByName warp
				if (warptarget==undefined) then (
					message = "ERROR: WARP TARGET "+ warp + " specified on object "+obj.name+" doesn't exist"
					messageBox message
					return 1
				)
				-- calcs distance from warptarget
				warpx = obj.pos.x - warptarget.pos.x
				warpy = obj.pos.y - warptarget.pos.y
				warpz = obj.pos.z - warptarget.pos.z
				warpx = (warpx * xscale) + xrelocate
				warpy = (warpy * yscale) + yrelocate
				warpz = (warpz * zscale) + zrelocate
	
				format "      <ww x=\"%\" y=\"%\" z=\"%\" /> \n" warpx warpz warpy to:outFile
			)


			arrives = getUserProp obj "ARRIVESINTHEMIDDLE"
			if (arrives=="yes") then
				format "      <clip/>" to:outFile

			starts = getUserProp obj "STARTSINTHEMIDDLE"
			if (starts=="yes") then
				format "      <zfill/>" to:outFile

			portalname = getUserProp obj "PORTAL"
			if (portalname==undefined) then
			(
				message = "ERROR: PORTAL WITH NO DESTINATION % "
				messageBox message
				return 1
			)
			format "	  <sector>%</sector>\n" portalname to:outFile
			format "    </portal>\n" portalname to:outFile
	
		)
	
	
		-- ////////////////////////
		-- Output Faces of a mesh
		-- ////////////////////////
		fn OutputMeshFaces obj outFile polyCombine verboseMode debug isportal istrasparent lighting colldet viscull playerbarrier =
		(
		   format "OutputMeshFaces for %\n" obj.name
		   -- list of faces already printed
		   skiplist = #()
		   numMat = 0
	
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
		    format "\n Object needs flipping: %\n" obj.name
		   )
	
		   -- determine number of materials on object
	 	   if (obj.mat == undefined) then (
				message = "   OBJECT " +obj.name+ " HAS NO TEXTURE! Aborting export."
				messageBox message
				return 1
		   )
		   else if ((classOf obj.mat)==Standardmaterial) then
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
	
			-- default csinvisibletexture to no
	   		csinvisibletexture="no";
	
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
					(
						if (obj.mat==undefined) then
						(
							message = "OBJECT " +obj.name+ " has an UNDEFINED material on face " + (i as String)
							messageBox message
							return 1
						)
				    	currentMat = obj.mat.name
					) else (
						if (obj.mat[matID]==undefined) then
						(
							message = "   OBJECT " +obj.name+ " has an UNDEFINED material on face " + (i as String)
							messageBox message
							return 1
						)
						currentMat = obj.mat[matID].name
					)
				)
	
				if (debug) then format "Material found: % on face %\n" currentMat i
	
			   -- output face only if it has current MatID
			   if (currentMat==matName) then
			   (
					
					-- get texture filename
					texturefilename="";
	
			        -- output material only if has changed since previous face
					if (matChanged) then (
					    if (obj.mat==undefined) then
							format "UNDEFINED MATERIAL for object %\n" obj.name
						else (
						    if ((classOf obj.mat)==Standardmaterial) then
							(
							   --format "standard material found for %\n" obj.name
								texturefilename=getMatFilename obj.mat
								if (texturefilename=="csinvisible.tga") then
									csinvisibletexture="yes"
								else
									csinvisibletexture="no"
						    	format "      <material>%</material>\n" texturefilename to:outFile
							) else (
							    if (debug) then format "multi material found for %\n" obj.name
								texturefilename=getMatFilename obj.mat[matID]
								if (texturefilename=="csinvisible.tga") then
									csinvisibletexture="yes"
								else
									csinvisibletexture="no"
								format "      <material>%</material>\n" texturefilename to:outFile
							)
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
	
						if (flipModel) then (
			 				a = verts[4] as Integer
							b = verts[1] as Integer
	 						c = verts[2] as Integer
							d = verts[3] as Integer
						)
	
						if (verboseMode) then



						(
						    polyName = obj.name + "_"+ i as String;
					    	format "      <p name=\"%\">" polyName to:outFile
						) else
							format "<p>" to:outFile
		
						format "      <v>%</v><v>%</v><v>%</v><v>%</v> \n" (a-1) (b-1) (c-1) (d-1) to:outFile
		
						if (debug) then format "      <v>%</v><v>%</v><v>%</v><v>%</v> \n" (a-1) (b-1) (c-1) (d-1)
	
						-- handles no lighting objects and invisible objects
						if (lighting=="no" or csinvisibletexture=="yes") then
							format "      <lighting>no</lighting>\n" to:outFile
	
						-- handles no collision detection meshes
						if (colldet=="no" ) then
							format "      <colldet>no</colldet>\n" to:outFile

						-- additional parameter to have right visibility culler on invisible objects
						if (csinvisibletexture=="yes") then
							format "      <viscull>yes</viscull>\n" to:outFile

						-- handles trasparent objects
						if (istrasparent or viscull=="no") then (
								format "      <viscull>no</viscull>\n" to:outFile
						)
	
						-- handles portals
						if (isportal) then (
							portalname = getUserProp obj "PORTAL"
							if (portalname==undefined) then
								format " ERROR: PORTAL WITH NO DESTINATION % " obj.name
	
							format "	    <portal><sector>%</sector><zfill /><clip /></portal>\n" portalname to:outFile
						)
	
						-- if no mapping close and go to next obj
						if (not Tmapping) then
						(
							format "      </p>\n" to:outFile
							continue
						)
	
						-- output UV Mapping with origin in bottom-left corner
						format "      <texmap>" to:outFile
						f = 0
						outputV = 0
	
						vertOrder = #(3,2,1,4)
						if (flipModel) then (
							if (debug) then (
								format "flipping UV for flipmodel\n"
							)
							vertOrder = #(4,1,2,3)
						)
	
						for h in vertOrder do
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
								if (debug) then (
									format "<uv idx=% u=% v=%>" f u v
								)
								format "<uv idx=\"%\" u=\"%\" v=\"%\" />" f u v to:outFile
								outputV = outputV + 1
							)
	
							--if (outputV<3) then format "," to:outFile
	
							f=f+1
						)
						format "</texmap></p>\n" to:outFile
	
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
	
					if (flipModel) then (
						a = verts[2] as Integer
						b = verts[3] as Integer
						c = verts[1] as Integer
					)
	
					if (verboseMode) then
					(
					    polyName = obj.name + "_" + i as String;
				    	format "      <p name=\"%\"> " polyName to:outFile
					) else
						format "      <p> " to:outFile
	
					format "      <v>%</v><v>%</v><v>%</v>\n" (a-1) (b-1) (c-1) to:outFile
	
					-- handles no lighting objects and invisible objects
					if (lighting=="no" or csinvisibletexture=="yes") then
						format "      <lighting>no</lighting>\n" to:outFile
	
					-- handles no collision detection objects
					if (colldet=="no" or csinvisibletexture=="yes") then
						format "      <colldet>no</colldet>\n" to:outFile



					
					-- additional parameter to have right visibility culler on invisible objects
					if (csinvisibletexture=="yes") then (
						format "      <viscull>yes</viscull>\n" to:outFile
	
					-- handles trasparent objects
					) else if (istrasparent or viscull=="no") then (
						format "      <viscull>no</viscull>\n" to:outFile
					)
	
					-- no TVerts assigned
				   if (getNumTVerts obj==0) then
				   (
				   		format "No UV maps assigned to object: %\n" obj.name
						format "      </p>\n" to:outFile
						continue
				   )
	
					-- output UV Mapping with origin in bottom-left corner
			        TVerts = getTVFace obj i
					aTV = getTVert obj TVerts[1]
					bTV = getTVert obj TVerts[3]
					cTV = getTVert obj TVerts[2]
	
					if (flipModel) then (
						aTV = getTVert obj TVerts[2]
						bTV = getTVert obj TVerts[3]
						cTV = getTVert obj TVerts[1]
					)
					format "      <texmap><uv idx=\"0\" u=\"%\" v=\"%\" /><uv idx=\"1\" u=\"%\" v=\"%\" /><uv idx=\"2\" u=\"%\" v=\"%\" /></texmap>\n" aTV[1] (1-aTV[2]) bTV[1] (1-bTV[2]) cTV[1] (1-cTV[2]) to:outFile
					format "      </p>\n" to:outFile
	
				)
			)
		  )
	    )
	
	
		-- ////////////////////////
		-- Output culleronly polymesh
		-- ////////////////////////
	
		fn OutputCullerOnly obj outFile verboseMode debug =
		(
			-- cycle on all faces of object
			for i =1 to obj.numFaces do
			(
					-- get its 3 vertices as a point3
					-- export in XZY format
				    verts=getface obj i
					a = verts[1] as Integer
					b = verts[3] as Integer
					c = verts[2] as Integer
	
					if (verboseMode) then
					(
				    	format "      <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" (a-1) (b-1) (c-1) to:outFile
					) else (
						format "<t v1=\"%\" v2=\"%\" v3=\"%\" />\n" (a-1) (b-1) (c-1) to:outFile
					)
			)
		)
	
		-- ////////////////////////
		-- Defines if an object should have <colldet>
		-- ////////////////////////
		fn doesCollide obj groupsInfo =
		(
	
			-- check for colldet setting
			colldetProp = getUserProp obj "COLLDET"
	
			-- if it should not collide (e.g. water)			
			if (colldetProp=="no") then
				return false;
	

			-- if is not in a group by defaults it collides
			if ( not isGroupMember obj) then
				return true;
	
			-- check for colldet based on groups
			
		)
	
	
		-- ////////////////////////
		-- Main: program starts here
		-- ////////////////////////
	
		-- output file
		destDir = getFilenamePath edt3.text
		format "makedir % \n" destDir
		makeDir destDir	-- If output folder does not exist...
		outFile = createFile filename
	
		-- write header
		
		format "<world>\n" to:outFile
	
	
		-- write variables tag
		format " <variables>\n" to:outFile
		format "  <variable name=\"lightning reset\"><color red=\"0\" green=\"0\" blue=\"0\"/></variable>\n" to:outFile

		-- lod variables
		customPropNumber = fileProperties.findProperty #custom "lodmax"
		if (customPropNumber==0) then (
			lodmax = 50
			lodmin = 200
		) else
			lodmax = (fileProperties.getPropertyValue #custom customPropNumber) as integer

		customPropNumber = fileProperties.findProperty #custom "lodmin"
		if (customPropNumber==0) then (
			lodmax = 50
			lodmin = 200
		) else
			lodmin = (fileProperties.getPropertyValue #custom customPropNumber) as integer

		lodm = 1.0 / (lodmax-lodmin) as float
		loda = -lodm * lodmin as float

		format "  <variable name=\"LodM\" value=\"%\" /><variable name=\"LodA\" value=\"%\" />\n" lodm loda to:outFile

		format " </variables>\n" to:outFile
	
		-- create a new array with objects present in groups
		allObjects = #()
		emptyArray = #()
		collvisInfo = #()
	
	
		format "Objects in the scene: %\n" objects.count
		allObjects = emptyArray + objects
		for obj in objects do
		(
		    -- search groups
			if ( (classOf obj)==Dummy and (isGroupHead obj)) then (
				if (isOpenGroupHead obj) then
				(
					message = "WARNING: OBJECT " +obj.name+ " IS AN OPEN GROUP. Skipping"
					continue
				)
	
				format "Found Group Object: %\n" obj.name
				-- search children of group
				childrenObjs = obj.children
				collDectName = ""
				visCullName = ""
				for child in childrenObjs do
				(
					if (findString child.name "_coll_" !=undefined) then (
						collDectName = child.name
					)
					else if (findString child.name "_vis_" !=undefined) then (
						visCullName = child.name
					)
					append allObjects child
				)
				-- add info
				info = #(obj.name, collDectName, visCullName, 0)
				append collvisInfo info
			) else
				continue
		)
		format "Objects in the scene after groups: %\n" allObjects.count
	
		format "\ncollvisInfo: %\n" collvisInfo
	
		-- Check particles and terrain: cycle on all objects in the scene
		terrainobject = undefined
		for obj in allObjects do 
		(
		    if ( (classOf obj)==Point) then (
				type = getUserProp obj "TYPE"
				format "Found Particle Object: %   Type: % \n" obj.name type
				if (type == "fire") then ( fireNeeded = true )
				else if (type == "emit") then ( emitNeeded = true )
	
				-- check for additional materials
				addMaterial = getUserProp obj "MATERIAL"
				if (addMaterial != undefined) then (
					-- if not already added
					if (findItem partMaterials addMaterial==0) then (
	

						append partMaterials addMaterial
					)
				)
			)
			
			if (findString obj.name "_terr_" !=undefined) then
				terrainobject = obj
		)

		-- write shaders
		WriteShaders outFile
	
		-- write materials		
	    WriteMaterials terrainobject outFile

		-- write plugins
	    format "  <plugins>\n" to:outFile
	    format "    <plugin name=\"thing\">crystalspace.mesh.loader.thing</plugin>\n" to:outFile
		format "    <plugin name=\"thingFact\">crystalspace.mesh.loader.factory.thing</plugin>\n" to:outFile
	    format "    <plugin name=\"meshFact\">crystalspace.mesh.loader.factory.genmesh</plugin>\n" to:outFile
	    format "    <plugin name=\"mesh\">crystalspace.mesh.loader.genmesh</plugin>\n" to:outFile

	  	if (fireNeeded) then (
		    format "    <plugin name=\"fireFact\">crystalspace.mesh.loader.factory.fire</plugin>\n" to:outFile  
			format "    <plugin name=\"fire\">crystalspace.mesh.loader.fire</plugin>\n" to:outFile  
	  	)
	  	if (emitNeeded) then (
	    	format "    <plugin name=\"emitFact\">crystalspace.mesh.loader.factory.emit</plugin>\n" to:outFile 
	    	format "    <plugin name=\"emit\">crystalspace.mesh.loader.emit</plugin>\n" to:outFile 
	  	)

		  -- add plugins needed for terrain
		  if (chkTerrain.checked) then (
		      format "    <plugin name=\"terrainFact\">crystalspace.mesh.loader.factory.terrain</plugin>\n" to:outFile 
			  format "    <plugin name=\"terrain\">crystalspace.mesh.loader.terrain</plugin>\n" to:outFile 
		  )

	    format "  </plugins>\n\n" to:outFile

		-- write renderpriorities and renderloop for terrain
		  if (chkTerrain.checked) then (
		   filestr = openFile "ps_terrain.render.txt"
		   if (filestr==undefined) then (
				message = "ERROR: ps_terrain.render.txt not found!"
				messageBox message
				return 1
		   )
		   while (not eof filestr) do
		   (
		   	line = readLine filestr
			format "%\n" line to:outFile
		   )
		  )

		-- write particles declarations
	  	if (fireNeeded) then (
		    format "    <meshfact name=\"fireFact\"><plugin>fireFact</plugin><params /></meshfact>\n\n" to:outFile
	    )
	  	if (emitNeeded) then (
		    format "    <meshfact name=\"emitFact\"><plugin>emitFact</plugin><params /></meshfact>\n\n" to:outFile  
	  	)
	

		-- search and write meshfacts and thingfacts and terrain
		factoryMeshes = #()
		factoryThingMeshes = #()
		for obj in allObjects do
		(
	
			-- Check for genmeshes factories
			objName = obj.name
			if (findString objName "_g_" !=undefined) then (
				-- factory must have name like : _g_name_0
				toks = tokenize objName "_"
				if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
				(
					append factoryMeshes obj

					-------------------
					-- support for LOD.
					-- it has to check all objects listed and export each as a nested meshfact
					-------------------
					lodlow = getUserProp obj "LODLOW"
					if (lodlow!=undefined) then (
						format "FOUND LOD OBJECT: %" obj.name

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
						format "<meshfact name=\"%\">\n" toks[3] to:outFile
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
							format "  <meshfact name=\"%_%\">\n" toks[3] (k-1) to:outFile
							format "  <lodlevel>%</lodlevel>\n" (k-1) to:outFile
							format "  <nullmesh><min x=\"-0.1\" y=\"-0.1\" z=\"-0.1\" /><max x=\"0.1\" y=\"0.1\" z=\"0.1\" /></nullmesh>\n" to:outFile
							format "  </meshfact>\n\n" to:outFile
							continue;
						)

						obj = lodobjects[k]

						-- output normalmeshfact
						-- meshfact name changes for hierarchy
						if (lodlow!=undefined) then
							format "  <meshfact name=\"%_%\">\n" toks[3] (k-1) to:outFile
						else
							format "  <meshfact name=\"%\">\n" toks[3] to:outFile

						if (lodobjects.count!=1) then
							format "<lodlevel>%</lodlevel>\n" (k-1) to:outFile
						format "  <plugin>meshFact</plugin><params><numvt>%</numvt><numtri>%</numtri>\n" (getNumTVerts obj) obj.numfaces to:outFile

						-- just for LOD hierarchical objects material must be speficied in the factory
						if (lodlow!=undefined) then (
							m = obj.mat;
							if ((classOf m)==Multimaterial) then (
								matUsed = getFaceMatID obj 1
								m = obj.mat[matUsed];
							)
							format "      <material>%</material>\n" (getMatFilename m) to:outFile
						)

						-- check texture
						if (getNumTVerts obj==0) then
						(
							message = "Export aborted: No UV maps assigned to genmesh object: " + obj.name
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
					    format "\n GenMesh Factory Object needs flipping: %\n" obj.name
					   )
					   
						vertTInfo = #(getNumTVerts obj)
						vertTInfo[1] = undefined
			
						-- cycle on all Faces of the object to get Tverts x,y,z positions
						for i=1 to obj.numfaces do
						(
							-- get face
							Tface = getTVFace obj i
							face = getFace obj i
			
							-- get its 3 vertices
							for h=1 to 3 do (
						    	curVert=getVert obj face[h]
								if (curVert==undefined) then
									format "\n\nUNDEF: %\n\n" Tface[h]
								if (debug) then
									format " face: % curVert %: %\n" i Tface[h] curVert
		
								-- if the value is different we have a problem on Welded UV
								if (vertTInfo[Tface[h]]!=undefined and vertTInfo[Tface[h]]!=curVert) then (
									message = "PROBLEM on object " + obj.name + ".\n UV should not be welded on vertex " + (Tface[h] as String)
									messageBox message
									close outFile
									return 1
								)
								vertTInfo[Tface[h]]=curVert
							)
						)
		
						-- cycle on all TVerts of the object
						for i =1 to (getNumTVerts obj) do
						(
							-- get its 3 vertices as a point3
							-- export in XZY format
						    vert=vertTInfo[i]
							if (vert==undefined) then (
								message = "PROBLEM on object " + obj.name + ": UV mapping seems to be messed up. Please UnWrap it then collapse it.";
								messageBox message
								close outFile
								return 1
							)
							xvert = (vert.x * xscale) + xrelocate
							yvert = (vert.y * yscale) + yrelocate
							zvert = (vert.z * zscale) + zrelocate
				
							Tvert = getTVert obj i
				
							format "      <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> \n" xvert zvert yvert Tvert[1] (1-Tvert[2]) i to:outFile
						)

						-- cycle on all faces of object
						for i =1 to obj.numFaces do
						(
							faceVerts=getTVface obj i
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
								format "    <t v1=\"%\" v2=\"%\" v3=\"%\">" a b c to:outFile
								--if (colldet=="no") then format "<colldet>no</colldet>" to:outFile
								format "</t>\n" to:outFile
							) else (
								--if (colldet=="no") then format "<t v1=\"%\" v2=\"%\" v3=\"%\"><colldet>no</colldet></t>"  a b c to:outFile
								format "    <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" a b c to:outFile
							)
					    )
						
						-- check if object is a multiple transparent object (like trees)
						transpmulti = getUserProp obj "TRANSPMULTIPLE"
						if (transpmulti=="yes") then (
	 					  --format "      <back2front />\n" to:outFile  NO MORE USED!
						)

						-- check for no shadow setting
						noshadows = getUserProp obj "NOSHADOWS"
						if (noshadows=="yes") then
							format "      <noshadows />\n" to:outFile

						format "    <autonormals /></params>\n" to:outFile
						if (colldet=="no") then format "<polymesh><colldet/></polymesh>\n" to:outFile

						format "    </meshfact>\n" to:outFile

					) -- end for each lodobject

					if (lodobjects.count!=1) then
						format "  </meshfact>\n" to:outFile

				) -- end if genmesh factory
			) -- end if genmesh

			-- check for thingFacts
			if (findString objName "_f_" !=undefined) then (
				-- factory must have name like : _f_name_0
				toks = tokenize objName "_"
				if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
				(
					append factoryThingMeshes obj
					format "  <meshfact name=\"%\"><plugin>thingFact</plugin><params>\n" toks[3] to:outFile
	
					-- output vertexes of the object in XZY format
					for v in obj.verts do
					(
						xvert = (v.pos.x * xscale) + xrelocate
						yvert = (v.pos.y * yscale) + yrelocate
	
						zvert = (v.pos.z * zscale) + zrelocate
						format "      <v x=\"%\" y=\"%\" z=\"%\" />\n"  xvert zvert yvert to:outFile
					)
	
					istrasparent=false
					-- alpha, ztest and zuse not supported on meshfact
					trasp = getUserProp obj "TRASPARENT"
					if (trasp=="yes") then (
					  format "found thingmesh factory as trasparent %\n" obj.name
					  -- istrasparent=true NO MORE USED!!
					)
	
					-- no shadow not supported on meshfact
					noshadows = getUserProp obj "NOSHADOWS"
					if (noshadows=="yes") then
						format "      <noshadows />\n" to:outFile
	
					-- check for no lighting setting
					lighting = getUserProp obj "LIGHTING"
	
					-- check for colldet based on groups
					--colldet = doesCollide obj collvisInfo
					--check simple colldet setting
					colldet = getUserProp obj "COLLDET"
					
					-- check for viscull setting
					viscull = getUserProp obj "VISCULL"
	
					-- check for culleronly setting
					culleronly = getUserProp obj "CULLERONLY"

					-- check player barrier setting
					playerbarrier = getUserProp obj "PLAYERBARRIER"

					 -- export faces of the mesh
					 OutputMeshFaces obj outFile polyCombine verboseMode debug false istrasparent lighting colldet viscull playerbarrier

					-- check for smooth setting
					smooth = getUserProp obj "SMOOTH"
					if (smooth=="yes") then
						format "      <smooth />\n" to:outFile

					format "    </params></meshfact>\n" to:outFile
				)
			)

			-- check for terrain
			if (findString objName "_terr_" !=undefined) then (
			
			  terrx = getUserProp obj "CELLSIZEX"
			  terrz = getUserProp obj "CELLSIZEY"
			  terry = getUserProp obj "MAXHEIGHT"
			  terrimage = getUserProp obj "HEIGHTMAP"

			  if (terrx==undefined or terry==undefined or terrz==undefined or terrimage==undefined ) then (
				messageBox "ERROR: The terrain object doesn't have the necessary properties set."
				return 1
			  )

			  -- output simple terraformer
			  format " <addon><plugin>crystalspace.terraformer.simple.loader</plugin> \n" to:outFile
			  format " <params><name>simple</name><heightmap>%</heightmap>\n" terrimage to:outFile
			  format " <scale x=\"%\" y=\"%\" z=\"%\" />\n" terrx terry terrz to:outFile
			  format " </params></addon>\n" to:outFile

			  format " <meshfact name=\"TerrainFact\">\n" to:outFile
			  format " <plugin>terrainFact</plugin>\n" to:outFile
			  format " <params><plugin>crystalspace.mesh.object.terrain.bruteblock</plugin>\n" to:outFile
			  format " <terraformer>simple</terraformer><sampleregion>\n" to:outFile
			  format " <min x=\"-%\" y=\"-%\" /> <max x=\"%\" y=\"%\" /></sampleregion>\n" terrx terrz terrx terrz to:outFile
			  format " </params></meshfact>\n" to:outFile
  			)
			
		)
	
		-- get ambient light
		customPropNumber = fileProperties.findProperty #custom "red"
		if (customPropNumber!=0) then
			ambRed = fileProperties.getPropertyValue #custom customPropNumber
		customPropNumber = fileProperties.findProperty #custom "green"
		if (customPropNumber!=0) then
			ambGreen = fileProperties.getPropertyValue #custom customPropNumber
		customPropNumber = fileProperties.findProperty #custom "blue"
		if (customPropNumber!=0) then
			ambBlue = fileProperties.getPropertyValue #custom customPropNumber
	
		-- Settings
		format "  <settings>" to:outFile
		if (ambRed!=undefined and ambGreen!=undefined and ambBlue!=undefined) then
			format "<clearzbuf>yes</clearzbuf><ambient red=\"%\" green=\"%\" blue=\"%\" />" ambRed ambGreen ambBlue to:outFile
		else
		    format "<clearzbuf>yes</clearzbuf>" to:outFile
	
		-- Setting for lightmaps cell size
		customPropNumber = fileProperties.findProperty #custom "lightmapsize"
		if (customPropNumber!=0) then
			lightmapsize = fileProperties.getPropertyValue #custom customPropNumber
		if (lightmapsize!=undefined) then
			format "<lightmapcellsize>%</lightmapcellsize>" lightmapsize to:outFile
		else
			format "<lightmapcellsize>32</lightmapcellsize>" to:outFile


		format "</settings>\n\n" to:outFile
	
	
	
		-- check groups for viscull/colldect
		groups = #();
		for obj in allObjects do 
		(
			if ( (classOf obj)==Dummy) then (
				if (isGroupHead obj) then (
					format "Found Group Object: %\n" obj.name
					append groups obj
				)
			)
		)
	
		-- manage sectors done by dummies
		sectors = #();
		for obj in allObjects do
		(
			objName = obj.name
			if ( (classOf obj)==Dummy and (findString objName "_sect_" !=undefined) ) then (
				append sectors obj
			)
		)
		format "sectors found % \n" sectors
	
		--if no sector present then use allObjects
		if (sectors.count==0) then (
			append sectors objects[1]; --trick to have the following "for" loop execute one time
			nosectors=true;
	
			-- get room name from custom property
			customPropNumber = fileProperties.findProperty #custom "roomname"
			if (customPropNumber==0) then (
				messageBox "Please click on File>File Properties and add a Custom Property called roomname with the name of the sector."
				return 1
			)
			roomName = fileProperties.getPropertyValue #custom customPropNumber 
	
			if (roomName==undefined) then (
			   messageBox "ERROR: Please set a custom property named roomname"
			   return 1
			) else (
			   format "Roomname: %\n" roomName
			)
		) else
			nosectors = false;
	
	
		-- TODO: this should be fixed. right now it doesn't work correctly for multiple sectors.
		lightsThresholdValues = #()
		lightsThresholdObjs = #()

		-- these 3 vars are here to be used by generatefakelights function, will be populated with the last sector analized
		lightsFound = #()
		lightColors = #()
		lightInfo = #()
		--

		------------------------------------
		-- cycle on all objects of the sector
		------------------------------------ 
		startingPos = null
		startingPosSect = null
		for sector in sectors do
		(
	
			lightsFound = #()
	
			--selects the objects present in the sector
			if (not nosectors) then (
				allObjects = sector.children
				objName = sector.name
				toks = tokenize objName "_"
				roomName = toks[3]
				format "Working on sector: % with children count: %\n" roomName allObjects.count
			) else (
				-- roomname already defined above
				format "Working on single sector level: %\n" roomName
			)
	
			-- start sector
		    format "  <sector name=\"%\">\n" roomName to:outFile

			--Setting for terrain
			if (chkTerrain.checked) then (
			    format "<renderloop>myLoop</renderloop>\n" to:outFile
			)

			-- choose culler type
			sector_type = getUserProp sector "SECTOR_TYPE"
			if (sector_type=="simple" or chkTerrain.checked) then (
				format "    <cullerp>crystalspace.culling.frustvis</cullerp>\n" to:outFile
			) else (
				format "    <cullerp>crystalspace.culling.dynavis</cullerp>\n" to:outFile
			)

			format "IF YOU GET ANY ERROR convert to editable mesh and collapse stack of offending object\n"
	
			format "Cycling on all objects...\n"
			for obj in allObjects do 
			(
			    -- skip groups
				if ( (classOf obj)==Dummy) then (
					format "Skipping Dummy Object: %\n" obj.name
					continue
				)
		
			    -- skip objects named _n_xxxx
				if (findString obj.name "_n_" != undefined) then (
					format "Skipping Unwanted Object: %\n" obj.name
					continue
				)
			
				-- store lights for later use
				if ( (classOf obj)==Omnilight) then (
				    append lightsFound obj
					-- search threshold
					turnonoff_r = getUserProp obj "TURNONOFF_R"
					turnonoff_g = getUserProp obj "TURNONOFF_G"
					turnonoff_b = getUserProp obj "TURNONOFF_B"
					turnonoff_fade = getUserProp obj "TURNONOFF_FADE"
					if (turnonoff_r==undefined) then (
						--append lightsThreshold #("-1","-1","-1", "-1")
					) else (
						-- look if threshold is equal to other lights
						found = false
						pos = 1
						for thr in lightsThresholdValues do (
							if ( thr[1] == turnonoff_r and thr[2] == turnonoff_g and thr[3] == turnonoff_b) then
							(
								found = true;
								exit;
							)
							pos = pos + 1
						)
						
						-- add to the same array element
						if (found) then
						(
							format "found equal at %" pos
							elem = lightsThresholdObjs[pos]
							append elem obj
							lightsThresholdObjs[pos] = elem
						-- add to a new element
						) else (
							format "adding new"
							append lightsThresholdValues #(turnonoff_r, turnonoff_g, turnonoff_b)
							append lightsThresholdObjs #(obj)
						)
					)
					format "Found Omnilight Object: %\n" obj.name
					continue
				)
	
				-- store camera for later use
				if ( (classOf obj)==Targetcamera) then (
				    if (obj.name=="Camera01") then (
					    startingPos = obj
						startingPosSect = roomName
						format "Found Targetcamera Object: %\n" obj.name
					)
					continue
				)

				-- skip target camera
				if ( (classOf obj)==Targetobject) then (
					format "Skipping Targetobject Object: %\n" obj.name
					continue
				)
			
				-- convert particles
				if ( (classOf obj)==Point) then (
					format "Found particle Object: %\n" obj.name
					OutputParticle obj allObjects outFile
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
		
		
				-- manage genmeshes
				isGenMesh = false
				objName = obj.name
				if (findString objName "_g_" != undefined) then (
					isGenMesh = true
					-- skip factories
					toks = tokenize objName "_"
					if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
						continue;
		
					format "    Working on genmesh %\n" obj.name

					-- search factory
					genFactObj=null
					if (debug) then format "factoryMeshes.count: %\n" factoryMeshes.count
					for genFact in factoryMeshes do
					(
						genFactName = "_g_"+toks[3]+"_0"
						if (genFact.name==genFactName) then genFactObj=genFact 
					)
					if (genFactObj==null) then (
						message = "Export aborted: NO OBJECT FOUND AS GENMESH FACTORY OF " + obj.name
						messageBox message
						close outFile
						return 1
					)

					-- manage LOD objects
					lodlow = getUserProp genFactObj "LODLOW"
					if (lodlow!=undefined) then (
						format "    <meshref name=\"%\">\n" obj.name to:outFile
					) else (
						format "    <meshobj name=\"%\">\n" obj.name to:outFile
						format "      <plugin>mesh</plugin>\n" obj.name to:outFile
					)
					-- check for no shadow setting
					noshadows = getUserProp obj "NOSHADOWS"
					if (noshadows=="yes") then
						format "      <noshadows />\n" to:outFile

					-- manage skydome as genmesh
					if (findString objName "_skydome" != undefined or findString objName "_skybox" != undefined) then (
						format "      <priority>sky</priority>\n" to:outFile
					)

					-- for meshref <factory> must go outisde <params>
					if (lodlow!=undefined) then
						format "      <factory>%</factory>\n" toks[3] to:outFile


            -- NO MORE USED!!!
					-- if factory is transparent
					--trasp = getUserProp genFactObj "TRASPARENT"
					--if (trasp=="yes") then (
					--	format "      <priority>alpha</priority><ztest />\n" to:outFile
					--)

					-- if factory is multiple transparent
					--transpmulti = getUserProp genFactObj "TRANSPMULTIPLE"
					--if (transpmulti=="yes") then (
					--  format "      <priority>alpha</priority>\n" to:outFile
 					--  format "      <ztest />\n" to:outFile
					--)

					-- meshref doesn't have <params> tag
					if (lodlow==undefined) then (
						format "      <params>" to:outFile
						-- for meshref <factory> must go outisde <params>
						format "      <factory>%</factory>\n" toks[3] to:outFile
					)

					m = obj.mat;
					if ((classOf m)==Multimaterial) then (
						matUsed = getFaceMatID obj 1
						m = obj.mat[matUsed];
					)

					if (lodlow==undefined) then (
						-- ALL genmeshes must have this to support light changes.
						format "      <localshadows />\n" to:outFile
						format "      <material>%</material></params>\n" (getMatFilename m) to:outFile
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
				    format "\n GenMesh Instance Object needs flipping: %\n" obj.name
				   )
				   
					-- calc distance from factory
					xMove = (obj.pos.x * xscale) + xrelocate
					yMove = (obj.pos.y * yscale) + yrelocate
					zMove = (obj.pos.z * zscale) + zrelocate
		
					format "      <move><v x=\"%\" y=\"%\" z=\"%\" />\n" xMove zMove yMove to:outFile
					--if (flipModel) then
					--	rotvalues = quattoeuler obj.rotation
					--else
						rotvalues = quattoeuler obj.rotation order:2
		
					-- seems that all models have rotation with wrong sign
					-- all objects must use Pivot Align to Object
					if (flipModel) then (
						rotx = ((rotvalues.x) * pi)/180
						roty = -((rotvalues.y) * pi)/180
						rotz = ((rotvalues.z) * pi)/180
					) else (
						rotx = (rotvalues.x * pi)/180
						roty = -((rotvalues.y * pi)/180)
						rotz = (rotvalues.z * pi)/180
					)
		
					format "      <matrix><rotx>%</rotx><roty>%</roty><rotz>%</rotz></matrix>\n" rotx roty rotz to:outFile
					format "      </move>\n"  to:outFile

					if (lodlow==undefined) then
						format "    </meshobj>\n" to:outFile
					else
						format "    </meshref>\n" to:outFile

					continue;
		
				)
		
				-- manage thingmeshes
				isThingMesh = false

				objName = obj.name
				if (findString objName "_f_" != undefined) then (
					isThingMesh = true
					-- skip factories
					toks = tokenize objName "_"
					if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
						continue;
		
					format "    <meshobj name=\"%\">\n" obj.name to:outFile
					-- check for no shadow setting
					noshadows = getUserProp obj "NOSHADOWS"
					if (noshadows=="yes") then
						format "      <noshadows />\n" to:outFile
		
					format "      <plugin>thing</plugin>\n" obj.name to:outFile
	
					-- search factory
					genFactObj=null
					for genFact in factoryThingMeshes do
					(
						genFactName = "_f_"+toks[3]+"_0"
						if (genFact.name==genFactName) then genFactObj=genFact 
					)
					if (genFactObj==null) then (
						message = "Export aborted: NO OBJECT FOUND AS THINGMESH FACTORY OF " + obj.name
						messageBox message
	
						close outFile
						return 1
					)
	
			-- NO MORE USED!!
					-- if factory is trasparent
					--trasp = getUserProp genFactObj "TRASPARENT"
					--if (trasp=="yes") then (
					--	format "      <priority>alpha</priority><ztest />\n" to:outFile
					--)
	
					format "      <params><factory>%</factory>\n" toks[3] to:outFile
		
		
					-- check that instance and factory have same number of faces
					if (genFactObj.numfaces!=obj.numfaces) then (
						message = "Export aborted. thingmesh "+obj.name+ " and thingfactory "+genFactObj.name + " have a different number of faces";
						messageBox message
						close outFile
						return 1
					)
					-- change material if necessary
					-- cycle on all faces of object
					matReplaced = #()
					for i =1 to obj.numFaces do
					(
						matIDOld = getFaceMatID genFactObj i
						matIDNew = getFaceMatID obj i
					    if ((classOf obj.mat)==Standardmaterial) then
					    	currentNewMat = obj.mat
						else
							currentNewMat = obj.mat[matIDOld]
					    if ((classOf genFactObj.mat)==Standardmaterial) then
					    	currentOldMat = genFactObj.mat
						else
							currentOldMat = genFactObj.mat[matIDNew]
		
						if (currentOldMat.name!=currentNewMat.name) then
						(

							-- check it has not been already replaced
							oldMatName = getMatFilename currentOldMat
							newMatName = getMatFilename currentNewMat
							if (findItem matReplaced oldMatName==0) then (
								format "<replacematerial old=\"%\" new=\"%\" />\n" oldMatName newMatName to:outFile
								append matReplaced oldMatName
								format "%\n" matReplaced
		
							)
						)
					)
					-- format "      <material>%</material></params>\n" (getMatFilename obj.mat) to:outFile NO MATERIAL CHANGE FOR NOW
					format "      </params>\n" to:outFile
		
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
				    format "\n ThingMesh Instance Object needs flipping: %\n" obj.name
				   )
		
					xMove = (obj.pos.x * xscale) + xrelocate
					yMove = (obj.pos.y * yscale) + yrelocate
					zMove = (obj.pos.z * zscale) + zrelocate
		
					format "      <move><v x=\"%\" y=\"%\" z=\"%\" />\n" xMove zMove yMove to:outFile
					rotvalues = quattoeuler obj.rotation order:2
		
					if (flipModel) then (
						rotx = ((rotvalues.x) * pi)/180
						roty = -((rotvalues.y) * pi)/180
						rotz = ((rotvalues.z) * pi)/180
					) else (
						rotx = (rotvalues.x * pi)/180
						roty = -((rotvalues.y * pi)/180)
						rotz = (rotvalues.z * pi)/180
					)
		
					format "      <matrix><rotx>%</rotx><roty>%</roty><rotz>%</rotz></matrix>\n" rotx roty rotz to:outFile
					format "      </move>\n"  to:outFile
					format "    </meshobj>\n" to:outFile
					continue;
		
				)

				-- manage terrain
				isTerrain = false
				objName = obj.name
				if (findString objName "_terr_" != undefined) then (

				  if (not chkTerrain.checked) then
				  (
					message = "You have one terrain object, but the level is not exported as terrain."
					messageBox message
				  )

				  materialmap = getUserProp obj "MATERIALMAP"
				  loddistance = getUserProp obj "LODDISTANCE"

				  terrx = (obj.pos.x * xscale) + xrelocate
				  terry = (obj.pos.y * yscale) + yrelocate
				  terrz = (obj.pos.z * zscale) + zrelocate
				
				  format "      <meshobj name=\"Terrain\">\n"  to:outFile
				  format "      <plugin>terrain</plugin><params><factory>TerrainFact</factory>\n"  to:outFile
				  format "      <staticlighting>yes</staticlighting><castshadows>yes</castshadows>\n"  to:outFile
				  -- output materialmap
				  m = obj.mat
				  if (classOf m!=Multimaterial) then (
					message = "Your terrain object doesn't have a multimaterial associated."
					messageBox message
				  ) else (
				      c = 0
				  	  for subm in m do (
					  	  image = getMatFilename subm


						  if (image=="MATERIALNOTDEFINED") then
						  	continue
							
						  imagetemp = image + "sshhaaddeerr"

					  	  if (c==0) then
							  format "      <material>%</material><materialpalette>\n" imagetemp to:outFile
						  else
						  	format "      <material>%</material>\n" imagetemp to:outFile
						  c = c + 1
					  )
					  format "    </materialpalette>\n"  to:outFile

				  )
				  format "      	<lodvalue name=\"splatting distance\">%</lodvalue><lodvalue name=\"error tolerance\">4</lodvalue>\n" loddistance to:outFile
				  format "      	<!--For bruteblock-->" to:outFile
  				  format "          <lodvalue name=\"block resolution\">16</lodvalue>\n" to:outFile
  				  format "          <lodvalue name=\"block split distance\">8</lodvalue>\n" to:outFile
  				  format "          <lodvalue name=\"minimum block size\">32</lodvalue>\n" to:outFile
  				  format "          <lodvalue name=\"cd resolution\">256</lodvalue>\n" to:outFile
  				  format "          <lodvalue name=\"lightmap resolution\">513</lodvalue>\n" to:outFile
				  format "      <materialmap image=\"%\" /></params>\n" materialmap to:outFile
				  format "      <move><v x=\"%\" y=\"%\" z=\"%\" /></move></meshobj>\n" terrx terrz terry to:outFile
				  continue;
				)


				--
				-- normal thing objects managed here
				--
                occluder = false
		
				-- check for culleronly setting
				culleronly = getUserProp obj "CULLERONLY"

				-- check player barrier setting
				playerbarrier = getUserProp obj "PLAYERBARRIER"

			    -- output name of object and some info
				if (findString obj.name "_occ_" !=undefined) then (
					occluder = true
				)
				
				if (occluder) then (
					format "\nFound Object Name: % type occluder" obj.name
				) else
					format "\nFound Object Name: % Faces: %" obj.name obj.numfaces

				isportal=false
		
			    -- handles portal objects first
				if (findString obj.name "_p_" !=undefined) then (
				  isportal=true

				  OutputPortal obj debug outFile
				  continue;
				)
				else if (occluder or culleronly=="yes" or playerbarrier=="yes") then (
					format "    <polymesh name=\"%\">\n" obj.name to:outFile
				) else (
				    format "    <meshobj name=\"%\">\n" obj.name to:outFile
					format "      <plugin>thing</plugin>\n" to:outFile
				)
		
				istrasparent=false

		-- NO MORE USED!!!
				-- handles transparent objects
				--if (findString obj.name "_t_" !=undefined) then (
				--  format "      <priority>alpha</priority>\n" to:outFile
				--  format "      <ztest />\n" to:outFile
				--  istrasparent=true
				--)

			    -- handles sky objects
				if (findString obj.name "_sky_" !=undefined) then (
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
	
				if ( (not occluder) and playerbarrier!="yes" and culleronly!="yes" and (not isportal)) then (
				  format "      <params>\n" to:outFile
				)
		
				-- check for no lighting setting
				lighting = getUserProp obj "LIGHTING"
				
				-- check for colldet setting
				colldet = getUserProp obj "COLLDET"
		
				-- check for smooth setting
				smooth = getUserProp obj "SMOOTH"
				if (smooth=="yes") then
					format "      <smooth />\n" to:outFile
		
				-- check for viscull setting
				viscull = getUserProp obj "VISCULL"

				-- handles box occluders
				if (occluder and (classOf obj)==Box) then
				(
					format "      <min x=\"%\" y=\"%\" z=\"%\" />\n"  obj.min.x obj.min.z obj.min.y to:outFile
					format "      <max x=\"%\" y=\"%\" z=\"%\" />\n"  obj.max.x obj.max.z obj.max.y to:outFile
				) else (

					-- output vertexes of the object in XZY format
					for v in obj.verts do
					(
						xvert = (v.pos.x * xscale) + xrelocate
						yvert = (v.pos.y * yscale) + yrelocate
						zvert = (v.pos.z * zscale) + zrelocate
						format "      <v x=\"%\" y=\"%\" z=\"%\" />\n"  xvert zvert yvert to:outFile
					)
					
					format "\n" to:outFile
					
					-- export faces of the mesh
					if (culleronly=="yes" or playerbarrier=="yes" or occluder) then (
						OutputCullerOnly obj outFile verboseMode debug 
					) else (
						OutputMeshFaces obj outFile polyCombine verboseMode debug isportal istrasparent lighting colldet viscull playerbarrier
					)
				)

				 -- close params
				if (playerbarrier=="yes") then (
					 format "      </mesh>\n" to:outFile
					 format "      <colldet />\n" to:outFile
					 format "    </polymesh>\n" to:outFile
				) else if (culleronly=="yes" or (occluder and (classOf obj)!=Box)) then (
					 format "      </mesh>\n" to:outFile
					 format "      <viscull />\n" to:outFile
					 format "    </polymesh>\n" to:outFile
				) else if (occluder and (classOf obj)==Box) then (
					 format "      </box>\n" to:outFile
					 format "      <viscull />\n" to:outFile
					 format "    </polymesh>\n" to:outFile
				) else (
				 	format "      </params>\n" to:outFile
					 -- close mesh object
					 format "    </meshobj>\n" to:outFile
				)
		
			 	format "\n" to:outFile
		
			  	--print (getUserPropBuffer obj) to:outFile -- output buffer as a string literal 
			  	gc()

			)  -- end of for obj in allObjects

		 	-- get info on dynamic lights
			maxframes = animationrange.end

			if (animationrange.end != 23f and animationrange.end != 47f) then
			(
				message = "You have to set the animation length to 24 frames (sun only) or to 48 (sun/rain). \n Each frame is 1 hour of the day."
				if (queryBox (message + " \n \nClick Yes for 24 frames (sun only) \nClick No to keep the current number of frames and exit") == true)
				then ( animationRange = interval 0 23 ) else return 1
			)

			if (animationrange.end != 23f and animationrange.end != 47f) then
			(
				message = "You have to set the animation length to 24 frames (sun only) or to 48 (sun/rain). \n Each frame is 1 hour of the day."
				messageBox message
				return 1
			)
		
			format "Exporting Lights...: %\n" lightsFound.count
			
			lightColors = #()
			lightInfo = #()
			-- it was performing this only if fakelight was checked
			-- now we do it always
			--if (chkLights.checked) then
			if (true) then
			(
				format "Dynamic Lights flag set\n"
				-- cycle on all frames of the animation
				fcount = 1
				for curFrame=0 to maxframes do (
					-- move to right frame
					slidertime=curFrame
		
					-- check which lights are dynamic
					lcount = 1
					fLights = #()
		
					for ll in lightsFound do
					(
						--format "Lights % \n" lcount
						-- convert lights from 0-255 to 0-1
						llred = ((ll.rgb.r)/255) * ll.multiplier
						llgreen = ((ll.rgb.g)/255) * ll.multiplier
						llblue = ((ll.rgb.b)/255) * ll.multiplier
		
						if (fcount!=1) then
						(
							-- check autodynamic setting
							autodyn = getUserProp ll "AUTODYNAMIC"
							if (autodyn=="yes") then
								lightInfo[lcount] = "autodynamic"
							-- check if light is dynamic
							else (
								--format "Comparing % with % \n" lightColors[fcount-1][lcount] #(llred,llgreen,llblue)
								prevColor = lightColors[fcount-1][lcount]
								if ( prevColor[1] != llred or prevColor[2] != llgreen or prevColor[3] != llblue) then
								(
								    format "light % changed on frame % \n" ll.name fcount
									lightInfo[lcount] = "dynamic"
								)
							)
						)
						insertItem #(llred,llgreen,llblue) fLights lcount
		
						lcount = lcount + 1
					)
					insertItem fLights lightColors fcount
					fcount = fcount + 1
				)
		
				format "\n dynamic lights % \n" lightInfo
				format "\n dynamic lights % \n" lightsFound
				
				-- debug only: REMOVE
				--for item in lightColors do
				--	format "% \n" item
				
				--format "lightInfo % % \n" lightInfo.count lightInfo
		
			)
	
			-- reset slider time (used mainly to avoid problem in getting last frame data)
			slidertime=0
		
			-- outputs lights
			lcount = 1
			for ll in lightsFound do
			(
	
			
				-- skip ambient light
				if (ll.name=="ambient") then (
					lcount = lcount + 1
					continue
				)
		 		    --format " ;Light: % \n" ll.name to:outFile
				format "    <light name=\"%%\">\n" ll.name roomname to:outFile
		
				-- check threshold setting to flag light as dynamic
				turnonoff_r = getUserProp ll "TURNONOFF_R"
		
				if (lightInfo.count>=lcount and (lightInfo[lcount]=="dynamic" or lightInfo[lcount]=="autodynamic")) then 
					format "      <dynamic />\n " to:outFile
				else if (turnonoff_r!=undefined) then (
					format "      <dynamic />\n " to:outFile
				)
		
				multiplier = ll.multiplier
		
				if (ll.useFarAtten==false) then
					format "    <attenuation>none</attenuation>\n" to:outFile
		
				xlight = (ll.pos.x * xscale) + xrelocate
				ylight = (ll.pos.y * yscale) + yrelocate
				zlight = (ll.pos.z * zscale) + zrelocate
				llradius = ll.farAttenEnd * xscale
		 			-- convert lights from 0-255 to 0-1
				llred = ((ll.rgb.r)/255) * multiplier
				llgreen = ((ll.rgb.g)/255) * multiplier
				llblue = ((ll.rgb.b)/255) * multiplier
		
				format "      <center x=\"%\" y=\"%\" z=\"%\" />\n" xlight zlight ylight to:outFile
			    format "      <radius>%</radius>\n" llradius to:outFile
				format "      <color red=\"%\" green=\"%\" blue=\"%\" />\n" llred llgreen llblue to:outFile
				format "    </light>\n"  to:outFile
				lcount = lcount + 1
			)
		
	
		-- close sector object
		format "  </sector>\n" to:outFile
	
		) -- end of sectors loop

		-- outputs camera
		if (startingPos != null) then
		(
			xstart = (startingPos.pos.x * xscale) + xrelocate
			ystart = (startingPos.pos.y * yscale) + yrelocate
			zstart = (startingPos.pos.z * zscale) + zrelocate
	
			format "  <start name=\"%\"><sector>%</sector><position x=\"%\" y=\"%\" z=\"%\" /></start>\n" startingPos.name startingPosSect xstart zstart ystart to:outFile
		) else (
			message = "No starting position found. Level exported anyway. To add a starting position add a Camera object called Camera01."
			messageBox message
		)
	
		-- start sequences (if any)
	
		format "   <sequences>\n" to:outFile
	
		-- output lightning sequence
		customPropNumber = fileProperties.findProperty #custom "lightning"
		if (customPropNumber!=0) then (
			lightning = fileProperties.getPropertyValue #custom customPropNumber 
			if (lightning=="yes") then
			(
				format "     <sequence name=\"% lightning\"> \n" roomName to:outFile
				format "      <setambient sector=\"%\" red=\"1.0\" green=\"1.0\" blue=\"1.5\" /> \n" roomName to:outFile
				format "      <delay min=\"50\" max=\"100\" /> \n" to:outFile
				format "      <setambient sector=\"%\" color_var=\"lightning reset\" /> \n" roomName to:outFile
				format "      <delay min=\"50\" max=\"150\" /> \n" to:outFile
				format "      <setambient sector=\"%\" red=\"1.0\" green=\"1.0\" blue=\"1.5\" /> \n" roomName to:outFile
				format "      <delay min=\"50\" max=\"150\" /> \n" to:outFile
				format "      <setambient sector=\"%\" color_var=\"lightning reset\" /> \n" roomName to:outFile
				format "     </sequence> \n\n" to:outFile
			)
		)
	
		-- debug output
		format "   lightsThreshold: %\n\n" lightsThreshold
		format "   lightsThresholdValues: %\n\n" lightsThresholdValues
		format "   lightsThresholdObjs: %\n\n" lightsThresholdObjs
	
	
		-- output threshold sequence
		lcount = 1
		for thr in lightsThresholdValues do
		(
			-- name sequence with first light
			seqlightname = (lightsThresholdObjs[lcount][1]).name
			 			-- output turn on sequence
			format "    <sequence name=\"light_%%_on\"> \n" seqlightname roomname to:outFile
			-- for each light in this threshold
			for ll in lightsThresholdObjs[lcount] do
			(
				duration = getUserProp ll "TURNONOFF_FADE"
				-- convert lights from 0-255 to 0-1
				llred = ((ll.rgb.r)/255) * ll.multiplier
				llgreen = ((ll.rgb.g)/255) * ll.multiplier
				llblue = ((ll.rgb.b)/255) * ll.multiplier
				format "        <fadelight light=\"%%\" red=\"%\" green=\"%\" blue=\"%\" duration=\"%\" />\n" ll.name roomname llred llgreen llblue duration to:outFile
			)
			format "        <enable trigger=\"light_%%_off\" /> \n " seqlightname roomname to:outFile
			format "    </sequence> \n " to:outFile
	
			-- output turnoff sequence
			format "    <sequence name=\"light_%%_off\"> \n " seqlightname roomname to:outFile
			-- for each light in this threshold
			for ll in lightsThresholdObjs[lcount] do
			(
				duration = getUserProp ll "TURNONOFF_FADE"
				format "        <fadelight light=\"%%\" red=\"0\" green=\"0\" blue=\"0\" duration=\"%\" />\n" ll.name roomname duration to:outFile
			)
			format "        <enable trigger=\"light_%%_on\" /> \n " seqlightname roomname to:outFile
			format "    </sequence> \n " to:outFile
	
			lcount = lcount + 1
		)


		-- output Fake Sequence for testing dynamic lights
		if (chkLights.checked) then
		(
		    if (edtDuration.text=="") then (
				messageBox "Enter a number in the Duration field (eg. 200)"
			)
	
			format "     <sequence name=\"%seq\">\n" roomName to:outFile
	
			-- for each hour of the day
			fcount = 1
			for fcount=1 to 24 do
			(
				-- for each light
				lcount = 1
				for ll in lightsFound do
				(
					-- if light is dynamic
					if (lightInfo.count>=lcount and lightInfo[lcount]=="dynamic") then
					(
						-- if light changed 
						if (fcount==1) then (
							-- for first hour, read the lights at last frame and compare.
							prevColor = lightColors[24][lcount]
						) else
							prevColor = lightColors[fcount-1][lcount]
	
						colors = lightColors[fcount][lcount]
						if ( prevColor[1] != colors[1] or prevColor[2] != colors[2] or prevColor[3] != colors[3]) then
						(
							-- ambient light
							if (ll.name=="ambient") then
								format "       <fadeambient sector=\"%\" red=\"%\" green=\"%\" blue=\"%\" duration=\"%\" />\n" roomname colors[1] colors[2] colors[3] edtDuration.text to:outFile
							else
								format "       <fadelight light=\"%%\" red=\"%\" green=\"%\" blue=\"%\" duration=\"%\" />\n" ll.name roomname colors[1] colors[2] colors[3] edtDuration.text to:outFile
						)
					)
					lcount = lcount + 1
				)
				format "       <delay time=\"%\" />\n" edtDuration.text to:outFile
				fcount = fcount + 1
			)
			format "       <enable trigger=\"%trig\" />\n" roomName to:outFile
			format "     </sequence>\n" to:outFile
		)
	
		-- end sequences (if any)
		format "   </sequences>\n" to:outFile
	
		-- start triggers (if any)
		format "   <triggers>\n" to:outFile
		
		-- output threshold triggers
		lcount = 1
		for thr in lightsThresholdValues do
		(
			-- name trigger with first light
			triglightname = (lightsThresholdObjs[lcount][1]).name
			
			-- output turn off sequence
			format "    <trigger name=\"light_%%_on\"> \n" triglightname roomname to:outFile
			format "        <lightvalue light=\"crystal%\" operator=\"greater\" red=\"%\" green=\"%\" blue=\"%\" />\n" roomname thr[1] thr[2] thr[3] to:outFile
			format "        <fire sequence=\"light_%%_off\" /> \n " triglightname roomname to:outFile
			format "    </trigger> \n " to:outFile
	
			-- output turn on sequence
			format "    <trigger name=\"light_%%_off\"> \n " triglightname roomname to:outFile
			format "        <lightvalue light=\"crystal%\" operator=\"less\" red=\"%\" green=\"%\" blue=\"%\" />\n" roomname thr[1] thr[2] thr[3] to:outFile
	
			format "        <fire sequence=\"light_%%_on\" /> \n " triglightname roomname to:outFile
			format "    </trigger> \n " to:outFile
	
			lcount = lcount + 1
		)
	
		-- output Fake triggers for testing dynamic lights
	
		if (chkLights.checked) then
		(
			format "     <trigger name=\"%trig\">\n" roomName to:outFile
			format "       <sectorvis sector=\"%\" />\n" roomName to:outFile
			format "       <fire sequence=\"%seq\" />\n" roomName to:outFile
			format "     </trigger>\n" to:outFile
		)
	
		-- end triggers (if any)
	
		format "   </triggers>\n" to:outFile
	
		-- close world object


		format "</world>\n" to:outFile
	
		close outFile 
	
		-- copy textures to output dir
		if (chk2.checked) then (
			test = CopyTexturesToDir outFile 
		)
	

		message = "ALL DONE!"
		messageBox message
	
	)
	on sanity pressed do
	(
		deleteFaces = not chkSanity.checked
		include "sanityCheck.ms"
	)
)gw = newRolloutFloater "Export Level" 300 400 
addRollout Test1 gw 

)
