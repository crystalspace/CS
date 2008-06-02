------------------------------------------------------------
-- Author: Luca Pancallo <pancallo@netscape.net>
--
-- Copyright (C) 2008 Atomic Blue (info@planeshift.it,
-- http://www.atomicblue.org)
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
ButtonText:"Export Level to CS" 
tooltip:"Export Level to CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export Level to CS" width:226 height:350
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
	
	on Test1 open do
	(
	   version = 70 as String
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
	
	   edt3.text = "C:\CS\\"+roomName+"\world"
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
		global getMatDiffuseMapFilename
		global getMatSpecMapFilename
		global getMatNormalMapFilename
		global getMatDispMapFilename
		global getMatDiffuseMapFullPath
		global getMatSpecMapFullPath
		global getMatNormalMapFullPath
		global getMatDispMapFullPath
	
		-- particle variables
		global fireNeeded = false

		global emitNeeded = false
		global partMaterials = #()
		
		-- Shader include booleans
		global rloop_diffuse = false
		global rloop_terrainfixed = false

		-- get room name from custom property
		customPropNumber = fileProperties.findProperty #custom "roomname"
		if (customPropNumber==0) then (
			messageBox "Please click on File>File Properties and add a Custom Property called roomname with the name of the sector."
			return 1
		)
		roomName = fileProperties.getPropertyValue #custom customPropNumber 

	
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
		fn getMatDiffuseMapFilename m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.maps[2]
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
			    mat = m.maps[5]
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
			    mat = m.maps[9]
				if (mat!=undefined) then
				(
					image = mat.normal_map.filename
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
			    mat = m.maps[12]
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
	
		-- get fullpath of a materials
		fn getMatDiffuseMapFullPath m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.maps[2]
				if (mat!=undefined) then
				(
					image = mat.filename
				) else
					image="materialnotdefined"
			    image = lowercase(image)
			)
		)

		fn getMatSpecMapFullPath m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.maps[5]
				if (mat!=undefined) then
				(
					image = mat.filename
				) else
					image="materialnotdefined"
			    image = lowercase(image)
			)
		)
		
		fn getMatNormalMapFullPath m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.maps[9]
				if (mat!=undefined) then
				(
					image = mat.normal_map.filename
				) else
					image="materialnotdefined"
			    image = lowercase(image)
			)
		)
		
		fn getMatDispMapFullPath m = 
		(
		    if (m==undefined) then
				image="materialnotdefined"
			else (
			    mat = m.maps[12]
				if (mat!=undefined) then
				(
					image = mat.filename
				) else
					image="materialnotdefined"
			    image = lowercase(image)
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
			if ((classOf m)==Standardmaterial) then
			(
			  -- For each material, if not written then add it.				
			  -- Diffuse Texture:
			  diffuseImage = getMatDiffuseMapFilename m				
			  if (findItem materialsWrittenToWorld diffuseImage==0 and diffuseImage!="materialnotdefined") then
			  (
			    format "m: % \n" m
				format "    <texture name=\"%\">\n" diffuseImage to:outFile
				format "      <file>%</file>\n" diffuseImage to:outFile
				-- handles transparent materials					
				if (m.mapEnables[7]) then
				(
				  format "      <alpha>\n" to:outFile
				  format "        <binary/>\n" to:outFile
				  format "      </alpha>\n" to:outFile
				)
				format "    </texture>\n" to:outFile
                    
				append materialsWrittenToWorld diffuseImage
			  )
			  -- Spec Map Texture:
			  specMapImage = getMatSpecMapFilename m
			  if (findItem materialsWrittenToWorld specMapImage==0 and specMapImage!="materialnotdefined") then
			  (
			    format "m: % \n" m
				format "    <texture name=\"%\">\n" specMapImage to:outFile
				format "      <file>%</file>\n" specMapImage to:outFile
				format "      <class>normalmap</class>\n" to:outFile
				format "    </texture>\n" to:outFile
					
				append materialsWrittenToWorld dispMapImage
			  )
			  -- Normal Map Texture:
			  normalMapImage = getMatNormalMapFilename m
			  if (findItem materialsWrittenToWorld normalMapImage==0 and normalMapImage!="materialnotdefined") then
			  (
			    format "m: % \n" m
			    format "    <texture name=\"%\">\n" normalMapImage to:outFile
			    format "      <file>%</file>\n" normalMapImage to:outFile
			    format "      <class>normalmap</class>\n" to:outFile
			    format "    </texture>\n" to:outFile
					
			    append materialsWrittenToWorld normalMapImage
			  )
			  -- Displacement Map Texture:
			  dispMapImage = getMatDispMapFilename m
			  if (findItem materialsWrittenToWorld dispMapImage==0 and dispMapImage!="materialnotdefined") then
			  (
			    format "m: % \n" m
				format "    <texture name=\"%\">\n" dispMapImage to:outFile
				format "      <file>%</file>\n" dispMapImage to:outFile
				format "      <class>normalmap</class>\n" to:outFile
				format "    </texture>\n" to:outFile
					
				append materialsWrittenToWorld dispMapImage
			  )
			)
	
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then
			(
		      for subm in m do
			  (
				image = getMatDiffuseMapFilename subm
				if (findItem materialsWrittenToWorld image==0 and image!="materialnotdefined") then
				(
				  format "m: % subm: % \n" m subm
				  if (subm.mapEnables[7]) then
				  (
					format "    <texture name=\"%\">\n" image to:outFile
                    format "      <file>%</file>\n" image to:outFile
					format "      <alpha>\n" to:outFile
					format "        <binary/>\n" to:outFile
				    format "      </alpha>\n" to:outFile
                    format "    </texture>\n" to:outFile
				  )
				  else
				  (
					format "    <texture name=\"%\">\n" image to:outFile
                    format "      <file>%</file>\n" image to:outFile
                    format "    </texture>\n" to:outFile
				  )
				  append materialsWrittenToWorld image
				)
			  )
			)		
		  )
		
		  if (fireNeeded or emitNeeded) then
		  (
		    format "m: raindrop\n"
			format "    <texture name=\"raindrop\">\n" to:outFile
            format "      <file>raindrop.png</file>\n" to:outFile
            format "    </texture>\n" to:outFile 
		  )
	
		  -- handle additional textures for particles  
		  for m in partMaterials do
		  (
		    format "m: % \n" m
		    format "    <texture name=\"%\">\n" m to:outFile
            format "      <file>%</file>\n" m to:outFile
            format "    </texture>\n" to:outFile
		  )

		  format "  </textures>\n\n" to:outFile
	
	
		  --/// MATERIALS
		
		  materialsWrittenToWorld=#()
		
		  format "  <materials>\n" to:outFile
		
		  for m in sceneMaterials do
		  (
		    -- handle Standardmaterials
			if ((classOf m)==Standardmaterial) then (
				-- if material not written, add it
				diffuseImage = getMatDiffuseMapFilename m
				if (findItem materialsWrittenToWorld diffuseImage==0 and diffuseImage!="materialnotdefined") then (
				    format "    <material name=\"%\">\n" diffuseImage to:outFile
                    format "      <texture>%</texture>\n" diffuseImage to:outFile
					
					normalMapImage = getMatNormalMapFilename m
					dispMapImage = getMatDispMapFilename m
					if(normalMapImage!="materialnotdefined" and dispMapImage!="materialnotdefined") then (
					  format "      <shader type=\"diffuse\">parallaxAtt</shader>\n" to:outFile
					  format "      <shader type=\"ambient\">ambient</shader>\n" to:outFile
					  format "      <shadervar type=\"texture\" name=\"tex normal compressed\">%</shadervar>\n" normalMapImage to:outFile
					  format "      <shadervar type=\"texture\" name=\"tex height\">%</shadervar>\n" dispMapImage to:outFile
					)
                              specMapImage = getMatSpecMapFilename m
                              if(specMapImage!="materialnotdefined") then (
                                format "      <shadervar type=\"texture\" name=\"tex specular\">%</shadervar>\n" specMapImage to:outFile
                              )
					
                    format "    </material>\n" to:outFile
					append materialsWrittenToWorld image
				)
			)
		
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatDiffuseMapFilename subm
					isshader = false

					-- skip wrong materials
					if ( image=="materialnotdefined") then
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
						format "    <material name=\"%\">\n" imagetemp to:outFile
                        format "      <texture>%</texture>\n" image to:outFile
						-- manage shaders
						if (findString subm.name "_shader_" != undefined) then (
							if (findString subm.name "_shader_terrain" !=undefined) then (
								format "      <shadervar name=\"texture scale\" type=\"vector2\">96,96</shadervar>\n" to:outFile
								format "      <shader type=\"terrain splat\">terrain_fixed_splatting</shader>\n" to:outFile
							)
							else if (findString subm.name "_shader_ambient" !=undefined) then (
								format "      <shader type=\"ambient\">terrain_fixed_base</shader>\n" to:outFile
							) else (
								message = "ERROR: Found terrain's material "+m.name+" with submat "+subm.name+" defining a shader of unsupported type!\n"
								message = message + "Supported types are: _shader_terrain , _shader_ambient"
								messageBox message
								format "    </material>\n" to:outFile
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
			    format "    <material name=\"raindrop\">\n      <texture>raindrop</texture>\n    </material>\n" to:outFile  
		  )
	
		  -- handle additional materials for particles
		  for m in partMaterials do
		  (
			    format "    <material name=\"%\">\n      <texture>%</texture>\n    </material>\n" m m to:outFile
		  )

		  format "  </materials>\n\n" to:outFile
		
		)

		-- ////////////////////////
		-- Write shaders function
		-- ////////////////////////
		
		fn WriteShaders outFile =
		(
		  for m in sceneMaterials do
		  (
		    -- Do a shader check on the materials.
			if(rloop_diffuse and rloop_terrainfixed) then
			(
			  break;
			)
			 
			if ((not rloop_diffuse) and (classOf m)==Standardmaterial) then
			(
			  normalMapImage = getMatNormalMapFilename m
			  if(normalMapImage!="materialnotdefined") then
			  (
			    rloop_diffuse=true
			    continue;
			  )
			)
			   
			if ((not rloop_terrainfixed) and (classOf m)==Multimaterial) then
			(
			  for subm in m do
			  (
			    if (subm!=undefined and findString subm.name "_shader_"!=undefined) then
			    (
			      rloop_terrainfixed=true
			      continue;
			    )
			  )
			)
		  )

		  format "  <shaders>\n" to:outFile

		  -- Include shaders needed for terrain.
		  if (rloop_terrainfixed) then
		  (
		    format "    <shader>\n" to:outFile
			format "      <file>/shader/terrain_fixed_base.xml</file>\n" to:outFile
			format "    </shader>\n" to:outFile
			format "    <shader>\n" to:outFile
			format "      <file>/shader/terrain_fixed_splatting.xml</file>\n" to:outFile
			format "    </shader>\n" to:outFile
		  )
		  
		  -- Include all other shaders used with realtime lighting.
		  if (rloop_diffuse) then
		  (
		    format "    <shader>\n" to:outFile
            format "      <file>/shader/parallaxAtt/parallaxAtt.xml</file>\n" to:outFile
            format "    </shader>\n" to:outFile
		  )
		  
		  format "  </shaders>\n\n" to:outFile
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

					format "    <meshobj name=\"Fire01\">\n" to:outFile
					format "      <priority>alpha</priority>\n" to:outFile
					format "      <plugin>emit</plugin>\n" to:outFile
					format "      <ztest />\n" to:outFile
					format "      <move> <v x=\"0\" y=\"0\" z=\"0\" /> </move>\n" to:outFile
					format "      <params>\n" to:outFile
					format "	    <factory>emitFact</factory> \n" to:outFile
					format "	    <mixmode><add /></mixmode>\n" to:outFile

					if (partMaterial == undefined) then
						format "	    <material>candleflame.png</material>\n" to:outFile
					else
						format "	    <material>%</material>\n" partMaterial  to:outFile	

					format "	    <number>%</number> \n" number to:outFile
					format "	    <regularparticles sides=\"4\" radius=\"0.1\" /> \n" to:outFile
					format "	    <lighting>off</lighting> \n" to:outFile
					format "	    <totaltime>1000</totaltime> \n" to:outFile
					format "	    <startpos><emitsphere x=\"%\" y=\"%\" z=\"%\" p=\"0\" q=\"0.03\" /></startpos> \n" xpart zpart ypart to:outFile
					format "	    <startspeed><emitbox><min x=\"0\" y=\"0\" z=\"0\" /> \n" to:outFile
					format "	    <max x=\"0\" y=\"0.3\" z=\"0\" /></emitbox></startspeed> \n" to:outFile
					format "	    <startaccel><emitfixed x=\"0\" y=\"0\" z=\"0\" /></startaccel>\n" to:outFile
					format "	    <attractor> <emitfixed x=\"%\" y=\"%\" z=\"%\"  /> </attractor> " xpart (zpart+0.2) ypart to:outFile
					format "	    <attractorforce>1</attractorforce>\n" to:outFile
					format "	    <aging> <time>0</time><color red=\"0.7\" green=\"0.1\" blue=\"0.0\" /> \n" to:outFile
					format "	    <alpha>0.2</alpha><swirl>0.01</swirl><rotspeed>0.0</rotspeed><scale>0.4</scale></aging>\n" to:outFile
					format "	    <aging> <time>250</time><color red=\"0.6\" green=\"0.1\" blue=\"0.05\" /> \n" to:outFile
					format "	    <alpha>0.5</alpha><swirl>0.002</swirl><rotspeed>0.0</rotspeed><scale>0.4</scale></aging>\n" to:outFile
					format "	    <aging> <time>500</time><color red=\"0.8\" green=\"0.6\" blue=\"0.1\" /> \n" to:outFile
					format "	    <alpha>0.8</alpha><swirl>0.01</swirl><rotspeed>0.0</rotspeed><scale>0.6</scale></aging>\n" to:outFile
					format "	    <aging> <time>750</time><color red=\"0.4\" green=\"0.2\" blue=\"0.05\" /> \n" to:outFile
					format "	    <alpha>0.6</alpha><swirl>0.02</swirl><rotspeed>0.0</rotspeed><scale>0.4</scale></aging>\n" to:outFile
					format "	    <aging> <time>1000</time><color red=\"0.3\" green=\"0.1\" blue=\"0.02\" /> \n" to:outFile
					format "	    <alpha>1</alpha><swirl>0.001</swirl><rotspeed>0.0</rotspeed><scale>0.2</scale></aging>\n" to:outFile
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
					attractorobj = obj.children[1]
					if (attractorobj!=undefined) then (
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

					if (attractorobj!=undefined) then (
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
					if (attractorobj!=undefined) then (
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
			if ((classOf m)==Standardmaterial) then
			(
				-- if material not written, add it
				diffuseImage = getMatDiffuseMapFullPath m
				if (diffuseImage!="materialnotdefined" and findItem materialsWrittenToWorld diffuseImage==0) then
				(
					destFile2 = filenameFromPath diffuseImage
					destFile2 = destDir + "\\" + destFile2
					Format "copy from % to % \n" image destFile2
					copyFile diffuseImage destFile2
					append materialsWrittenToWorld diffuseImage
				)
				specMapImage = getMatSpecMapFullPath m
				if (specMapImage!="materialnotdefined" and findItem materialsWrittenToWorld specMapImage==0) then
				(
					destFile2 = filenameFromPath specMapImage
					destFile2 = destDir + "\\" + destFile2
					Format "copy from % to % \n" specMapImage destFile2
					copyFile specMapImage destFile2
					append materialsWrittenToWorld specMapImage 
				)
				normalMapImage = getMatNormalMapFullPath m
				if (normalMapImage!="materialnotdefined" and findItem materialsWrittenToWorld normalMapImage==0) then
				(
					destFile2 = filenameFromPath normalMapImage
					destFile2 = destDir + "\\" + destFile2
					Format "copy from % to % \n" normalMapImage destFile2
					copyFile normalMapImage destFile2
					append materialsWrittenToWorld normalMapImage
				)
				dispMapImage = getMatDispMapFullPath m
				if (dispMapImage!="materialnotdefined" and findItem materialsWrittenToWorld dispMapImage==0) then
				(
					destFile2 = filenameFromPath dispMapImage
					destFile2 = destDir + "\\" + destFile2
					Format "copy from % to % \n" dispMapImage destFile2
					copyFile dispMapImage destFile2
					append materialsWrittenToWorld dispMapImage
				)
			)

		    -- handle Multi/materials
			else if ((classOf m)==Multimaterial) then
			(
				for subm in m do
				(
					diffuseImage = getMatDiffuseMapFullPath subm
					if (diffuseImage!="materialnotdefined" and findItem materialsWrittenToWorld image==0) then
					(
						destFile2 = filenameFromPath diffuseImage
						destFile2 = destDir + "\\" + destFile2
						Format "copy from % to % \n" diffuseImage destFile2
						copyFile diffuseImage destFile2
						append materialsWrittenToWorld diffuseImage
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
			format "    <portals>\n      <portal name=\"%\">\n" obj.name to:outFile

			-- all portals as autoresolve
			format "        <autoresolve />\n" to:outFile

			-- check if poly is valid: for now should be 2 faces or have VERTS specified
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
			piv = obj.pos
			for v in verts do
			(
				currVert = getVert obj v

                --relocate in local pivot coords (needed for rotation of warping portals)
				--currVert = currVert-piv

				--scale
				scaledx = (currVert.x * xscale) + xrelocate
				scaledy = (currVert.y * yscale) + yrelocate
				scaledz = (currVert.z * zscale) + zrelocate
				format "        <v x=\"%\" y=\"%\" z=\"%\" /> \n" scaledx scaledz scaledy to:outFile
				--format "        <v x=\"%\" y=\"%\" z=\"%\" /> \n" currVert.x currVert.z currVert.y to:outFile
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

	            -- check portal rotation compared to target rotation
	            if (obj.rotation.x!=warptarget.rotation.x or obj.rotation.y!=warptarget.rotation.y or obj.rotation.z!=warptarget.rotation.z) then (
				   -- move the target to 0,0,0 for a clean rotation
				   format "        <wv x=\"%\" y=\"%\" z=\"%\" /> \n" warptarget.pos.x warptarget.pos.z warptarget.pos.y to:outFile

				   -- rotate
				   rotmatrix = (warptarget.rotation - obj.rotation) as eulerAngles
				   rotmatrixcs = (eulerAngles rotmatrix.x rotmatrix.z rotmatrix.y) as matrix3
				   format "        <matrix>\n" to:outFile
				   format "          <m11>%</m11><m12>%</m12><m13>%</m13>\n" rotmatrixcs[1][1] rotmatrixcs[1][2] rotmatrixcs[1][3] to:outFile
				   format "          <m21>%</m21><m22>%</m22><m23>%</m23>\n" rotmatrixcs[2][1] rotmatrixcs[2][2] rotmatrixcs[2][3] to:outFile
				   format "          <m31>%</m31><m32>%</m32><m33>%</m33>\n" rotmatrixcs[3][1] rotmatrixcs[3][2] rotmatrixcs[3][3] to:outFile
				   format "        </matrix> \n" to:outFile

				   -- move it back in the right spot
				   format "        <ww x=\"%\" y=\"%\" z=\"%\" /> \n" obj.pos.x obj.pos.z obj.pos.y to:outFile

				) else (
				  format "        <ww x=\"%\" y=\"%\" z=\"%\" /> \n" warpx warpz warpy to:outFile
				)
			)
			
			arrives = getUserProp obj "ARRIVESINTHEMIDDLE"
			if (arrives=="yes") then
				format "        <clip/>\n" to:outFile

			starts = getUserProp obj "STARTSINTHEMIDDLE"
			if (starts=="yes") then
				format "        <zfill/>\n" to:outFile

			portalname = getUserProp obj "PORTAL"
			if (portalname==undefined) then
			(
				message = "ERROR: PORTAL WITH NO DESTINATION % " obj.name
				messageBox message
				return 1
			)
			format "        <sector>%</sector>\n" portalname to:outFile
			format "      </portal>\n" to:outFile

			-- now displace by the pivot position
			format "    </portals>\n" to:outFile
	
		)
	
	
		-- ////////////////////////
		-- Output culleronly trimesh
		-- ////////////////////////
	
		fn OutputCullerOnly obj outFile verboseMode debug =
		(

			-- output vertexes of the object in XZY format
			for v in obj.verts do
			(
				xvert = (v.pos.x * xscale) + xrelocate
				yvert = (v.pos.y * yscale) + yrelocate
				zvert = (v.pos.z * zscale) + zrelocate
				format "      <v x=\"%\" y=\"%\" z=\"%\" />\n"  xvert zvert yvert to:outFile
			)
			
			format "\n" to:outFile

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

		fn OutputGenMeshFactory obj outFile debug auto =
        (
			format "auto setting is % for object %\n" auto obj.name
			if (auto==1) then (
			    format "AUTO setting for object %\n" obj.name
				factoryName = "_auto_" + obj.name
			) else (
				toks = tokenize obj.name "_"
				factoryName = toks[3]
			)

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
				format "    <plugin>meshFact</plugin>\n    <params>\n      <numvt>%</numvt>\n" (getNumTVerts obj) to:outFile

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

					format "      <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> \n" xvert zvert yvert Tvert[1] (1-Tvert[2]) i to:outFile
				)

				-- write all normals
				for i =1 to (getNumTVerts obj) do
				(
					Tvert = getTVert obj i
					normalvert = normalsVert[i]
					normalvert = normalize(normalvert)
					format "      <n x=\"%\" y=\"%\" z=\"%\" /> \n" normalvert.x normalvert.z normalvert.y to:outFile
				)

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

					format "      <submesh name=\"%\">\n" ("submesh"+i as String) to:outFile
                    format "        <material>%</material>\n" mname to:outFile
                    format "        <indexbuffer components=\"1\" type=\"uint\" indices=\"yes\">\n" to:outFile

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
							format "          <e c0=\"%\" /><e c0=\"%\" /><e c0=\"%\" />\n" a b c to:outFile
							--if (colldet=="no") then format "<colldet>no</colldet>" to:outFile
						) else (
							--if (colldet=="no") then format "<t v1=\"%\" v2=\"%\" v3=\"%\"><colldet>no</colldet></t>"  a b c to:outFile
							format "          <e c0=\"%\" /><e c0=\"%\" /><e c0=\"%\" />\n" a b c to:outFile
						)
					)
					format "        </indexbuffer>\n      </submesh>\n" to:outFile
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
				  format "    <priority>alpha</priority>\n" to:outFile
				  format "    <ztest />\n" to:outFile
				)
			    -- handles sky objects
				else if (findString obj.name "_sky_" !=undefined) then (
				  format "    <priority>sky</priority>\n" to:outFile
				  format "    <zuse />\n" to:outFile
				)
			    -- handles zfill objects
				else if (findString obj.name "_s_" !=undefined) then (
				  format "    <priority>object</priority>\n" to:outFile
				  format "    <zuse />\n" to:outFile
				)
				else if (culleronly=="yes" or playerbarrier=="yes" or (occluder and (classOf obj)!=Box)) then (
					format "    <mesh>\n" to:outFile
				) else if (occluder and (classOf obj)==Box) then (
					format "    <box>\n" to:outFile
				) else (
				  format "    <priority>object</priority>\n" to:outFile
				  format "    <zuse />\n" to:outFile
				)

				-- check for no shadow setting
				noshadows = getUserProp obj "NOSHADOWS"
				if (noshadows=="yes") then
					format "    <noshadows />\n" to:outFile


				if (colldet=="no") then format "<trimesh><id>colldet</id></trimesh>\n" to:outFile

				-- check if object is a range alpha trasparent
				rangetrasp = getUserProp obj "RANGETRASP"
				if (rangetrasp=="yes") then (
				  format "    <ztest/><priority>alpha</priority>\n" to:outFile
				)

				-- check if object uses a binary alpha texture
			    if ((classOf obj.mat)==Standardmaterial) then (
					if (obj.mat.mapEnables[7]) then (
					  format "    <trimesh><id>viscull</id></trimesh>\n" to:outFile
					  format "    <trimesh><default /><id>colldet</id></trimesh>\n" to:outFile
					)
				) else (
					if (obj.mat[1].mapEnables[7]) then (
					  format "    <trimesh><id>viscull</id></trimesh>\n" to:outFile
					  format "    <trimesh><default /><id>colldet</id></trimesh>\n" to:outFile
					)
				)



				format "  </meshfact>\n\n" to:outFile

			) -- end for each lodobject

			if (lodobjects.count!=1) then
				format "  </meshfact>\n\n" to:outFile

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
		
		format "<world>\n\n" to:outFile
	
		-- write variables tag
		format "  <variables>\n" to:outFile
		format "    <variable name=\"lightning reset\">\n      <color red=\"0\" green=\"0\" blue=\"0\"/>\n    </variable>\n" to:outFile

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

		format "    <variable name=\"LodM\" value=\"%\" />\n    <variable name=\"LodA\" value=\"%\" />\n" lodm loda to:outFile

		format "  </variables>\n\n" to:outFile
	
		-- create a new array with objects present in groups
		allObjects = #()
		emptyArray = #()
		collvisInfo = #()
	
	
		format "Objects in the scene: %\n" objects.count
		allObjects = emptyArray + objects
		for obj in objects do
		(
		    -- skip particle views
			if ( (classOf obj)==Particle_View ) then
			  continue;

		    -- skip lines
			if ( (classOf obj)==line ) then
			  continue;

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
		
		-- Write renderloop info
		format "  <addon>\n" to:outFile
		if (rloop_terrainfixed or rloop_diffuse) then
		(
		  format "    <plugin>crystalspace.renderloop.loop.loader</plugin>\n" to:outFile
		)		
		if (rloop_terrainfixed) then
		(
		  format "    <paramsfile>/shader/std_rloop_terrainfixed.xml</paramsfile>\n" to:outFile
		)
		if (rloop_diffuse) then
		(
		  format "    <paramsfile>/shader/std_rloop_diffuse.xml</paramsfile>\n" to:outFile
		)
		format "  </addon>\n\n" to:outFile
		
		-- write materials		
	    WriteMaterials terrainobject outFile

		-- write plugins
	    format "  <plugins>\n" to:outFile
	    format "    <plugin name=\"meshFact\">crystalspace.mesh.loader.factory.genmesh</plugin>\n" to:outFile
	    format "    <plugin name=\"mesh\">crystalspace.mesh.loader.genmesh</plugin>\n" to:outFile

	  	--if (fireNeeded) then (
		--    format "    <plugin name=\"fireFact\">crystalspace.mesh.loader.factory.fire</plugin>\n" to:outFile  
		--	format "    <plugin name=\"fire\">crystalspace.mesh.loader.fire</plugin>\n" to:outFile  
	  	--)

	  	if (fireNeeded or emitNeeded) then
		(
	      format "    <plugin name=\"emitFact\">crystalspace.mesh.loader.factory.emit</plugin>\n" to:outFile 
	      format "    <plugin name=\"emit\">crystalspace.mesh.loader.emit</plugin>\n" to:outFile 
	  	)

		  -- add plugins needed for terrain
		if (rloop_terrainfixed) then
		(
		  format "    <plugin name=\"terrainFact\">crystalspace.mesh.loader.factory.terrain</plugin>\n" to:outFile 
		  format "    <plugin name=\"terrain\">crystalspace.mesh.loader.terrain</plugin>\n" to:outFile 
		)

	    format "  </plugins>\n\n" to:outFile

		-- write particles declarations
	  	--if (fireNeeded) then (
		--    format "    <meshfact name=\"fireFact\"><plugin>fireFact</plugin><params /></meshfact>\n\n" to:outFile
	    --)
	  	if (fireNeeded or emitNeeded) then (
		    format "    <meshfact name=\"emitFact\"><plugin>emitFact</plugin><params /></meshfact>\n\n" to:outFile  
	  	)

		-- write library inclusions for grass/foliage
		for obj in allObjects do
		(
			if ( (classOf obj)==Dummy and (findString obj.name "_meshgen_" !=undefined) ) then (

				toks = tokenize obj.name "_"
				grassName = toks[3]

				format "Found Grass Object: % \n" grassName

				-- check for additional materials
				libs = getUserProp obj "LIBS"
				if (libs != undefined) then (
					toks = tokenize libs ","
					for item in toks do (
						format "    <library>%</library>\n\n" item to:outFile  
					)
				)
			)
		)

		-- search and write meshfacts and thingfacts and terrain
		factoryMeshes = #()
		for obj in allObjects do
		(
			-- skip lights, dummy, camera
			if ( (classOf obj)==Omnilight or (classOf obj)==Dummy or (classOf obj)==Targetcamera or
			    (classOf obj)==Targetobject or (classOf obj)==line or (classOf obj)==Point) then (
				continue
			)

			-- skip portal objects
			if (findString obj.name "_p_" !=undefined) then continue

			-- skip occluders
			if (findString obj.name "_occ_" !=undefined) then continue

			-- skip _n_ objects
			if (findString obj.name "_n_" !=undefined) then continue

			-- skip player barriers
			playerbarrier = getUserProp obj "PLAYERBARRIER"
			if (playerbarrier=="yes") then continue

			-- check for old tags
			if (findString obj.name "_t_" !=undefined) then (
				message = "Object " + obj.name + " has an old _t_ tag. Please remove the _t_ and use TRASPARENT=yes or RANGETRASP=yes in the properties."
				messageBox message
				close outFile
				return 1
			)


	        -- ****************************************
			-- Check for explicit genmeshes factories
	        -- ****************************************
			objName = obj.name
			if (findString objName "_g_" !=undefined) then (
				-- factory must have name like : _g_name_0
				toks = tokenize objName "_"
				if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
				(
					append factoryMeshes obj
					
					OutputGenMeshFactory obj outFile debug 0


				) -- end if genmesh factory
				continue;
			) -- end if genmesh


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

			  -- output terraformer
			  format " <addon plugin=\"crystalspace.terraformer.simple.loader\"> \n" to:outFile
			  format "   <name>%</name><heightmap>%</heightmap>\n" roomName terrimage to:outFile
			  format "   <scale x=\"%\" y=\"%\" z=\"%\" />\n" terrx terry terrz to:outFile
			  format "   <offset x=\"%\" y=\"%\" z=\"%\" />\n" obj.pos.x obj.pos.z obj.pos.y to:outFile
			  materialmap = getUserProp obj "MATERIALMAP"
			  if (materialmap!="alpha") then
  			      format "   <materialmap image=\"%\" />\n" materialmap to:outFile
			  else (
			     materialmapa = getUserProp obj "MATERIALMAPA"
				 toks = tokenize materialmapa "|"
				for a in toks do (
                    format "   <materialalphamap image=\"%\" />\n" a to:outFile
				)
			  )
			  

			  format " </addon>\n" to:outFile

			  sectorname = obj.parent.name
			  toks = tokenize sectorname "_"
			  sectorname = toks[3]
			  format "  <meshfact name=\"tFact%\">\n" sectorname to:outFile
			  format "    <plugin>terrainFact</plugin>\n" to:outFile
			  format "    <params>\n      <plugin>crystalspace.mesh.object.terrain.bruteblock</plugin>\n" to:outFile
			  format "      <terraformer>%</terraformer>\n      <sampleregion>\n" roomName to:outFile
			  format "        <min x=\"-%\" y=\"-%\" />\n        <max x=\"%\" y=\"%\" />\n      </sampleregion>\n" terrx terrz terrx terrz to:outFile
			  format "    </params>\n  </meshfact>\n\n" to:outFile
			  continue;
  			)

	        -- ****************************************
			-- For any other object we create a genmesh factory automatically
	        -- ****************************************

	        OutputGenMeshFactory obj outFile debug 1
		  
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
					
					-- check for no lighting setting
					lighting = getUserProp obj "LIGHTING"
	
					--check simple colldet setting
					colldet = getUserProp obj "COLLDET"

					-- check for viscull setting
					viscull = getUserProp obj "VISCULL"

					-- check for culleronly setting
					culleronly = getUserProp obj "CULLERONLY"

			
		) -- end for obj in allObjects
	
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
		format "  <settings>\n" to:outFile
		if (ambRed!=undefined and ambGreen!=undefined and ambBlue!=undefined) then
			format "    <clearzbuf>yes</clearzbuf>\n    <ambient red=\"%\" green=\"%\" blue=\"%\" />\n" ambRed ambGreen ambBlue to:outFile
		else
		    format "    <clearzbuf>yes</clearzbuf>\n" to:outFile
	
		-- Setting for lightmaps cell size
		customPropNumber = fileProperties.findProperty #custom "lightmapsize"
		if (customPropNumber!=0) then
			lightmapsize = fileProperties.getPropertyValue #custom customPropNumber
		if (lightmapsize!=undefined) then
			format "    <lightmapcellsize>%</lightmapcellsize>\n" lightmapsize to:outFile
		else
			format "    <lightmapcellsize>32</lightmapcellsize>\n" to:outFile

		format "  </settings>\n\n" to:outFile
	
	
	
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

			-- renderloop selection
			renderloop_type = getUserProp sector "RENDERLOOP_TYPE"
			if (renderloop_type=="terrainfixed" and rloop_terrainfixed) then (
		      format "    <renderloop>std_rloop_terrainfixed</renderloop>\n" to:outFile
			)
			else if (renderloop_type=="diffuse" and rloop_diffuse) then (
			  format "    <renderloop>std_rloop_diffuse</renderloop>\n" to:outFile
			)
			
			-- choose culler type
			sector_type = getUserProp sector "SECTOR_TYPE"
			if (sector_type=="simple" or renderloop_type=="terrainfixed") then (
				format "    <cullerp>crystalspace.culling.frustvis</cullerp>\n" to:outFile
			) else (
				format "    <cullerp>crystalspace.culling.dynavis</cullerp>\n" to:outFile
			)

			format "IF YOU GET ANY ERROR convert to editable mesh and collapse stack of offending object\n"
	
			format "Cycling on all objects...\n"
			for obj in allObjects do 
			(
			    -- skip particle views
				if ( (classOf obj)==Particle_View ) then
				  continue;		

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
		
				-- manage terrain
				isTerrain = false
				objName = obj.name
				if (findString objName "_terr_" != undefined) then (

				  if (not rloop_terrainfixed) then
				  (
					message = "You have one terrain object, but the level is not exported as terrain."
					messageBox message
				  )

				  loddistance = getUserProp obj "LODDISTANCE"

				  terrx = (obj.pos.x * xscale) + xrelocate
				  terry = (obj.pos.y * yscale) + yrelocate
				  terrz = (obj.pos.z * zscale) + zrelocate
				
				  format "    <meshobj name=\"Terrain_%\">\n" roomName to:outFile
				  format "      <plugin>terrain</plugin>\n" to:outFile
                  format "      <params>\n" to:outFile
                  format "        <factory>tFact%</factory>\n" roomName to:outFile
				  format "        <staticlighting>yes</staticlighting>\n" to:outFile
                  format "        <castshadows>yes</castshadows>\n" to:outFile
				  -- output materialmap
				  m = obj.mat
				  if (classOf m!=Multimaterial) then (
					message = "Your terrain object doesn't have a multimaterial associated."
					messageBox message
				  ) else (
				      c = 0
				  	  for subm in m do (
					  	  image = getMatDiffuseMapFilename subm

						  if (image=="materialnotdefined") then
						  	continue
							
						  imagetemp = image + "sshhaaddeerr"

					  	  if (c==0) then
						  (
							  format "        <material>%</material>\n" imagetemp to:outFile
                              format "        <materialpalette>\n" to:outFile
					      )
						  else
						  	format "          <material>%</material>\n" imagetemp to:outFile
						  c = c + 1
					  )
					  format "        </materialpalette>\n" to:outFile

				  )
				  format "        <lodvalue name=\"splatting distance\">%</lodvalue>\n" loddistance to:outFile
                  format "        <lodvalue name=\"error tolerance\">4</lodvalue>\n" to:outFile
				  format "        <!--For bruteblock-->\n" to:outFile
  				  format "        <lodvalue name=\"block resolution\">64</lodvalue>\n" to:outFile
  				  format "        <lodvalue name=\"block split distance\">64</lodvalue>\n" to:outFile
  				  format "        <lodvalue name=\"minimum block size\">32</lodvalue>\n" to:outFile
  				  format "        <lodvalue name=\"cd resolution\">256</lodvalue>\n" to:outFile
  				  format "        <lodvalue name=\"lightmap resolution\">513</lodvalue>\n" to:outFile
                  format "      </params>\n" to:outFile
				  format "      <move>\n" to:outFile
                  format "        <v x=\"0\" y=\"0\" z=\"0\" />\n" to:outFile
                  format "      </move>\n" to:outFile
                  format "    </meshobj>\n" to:outFile
				  continue;
				) -- end manage terrain


				isportal=false
		
			    -- handles portal objects first
				if (findString obj.name "_p_" !=undefined) then (
				  isportal=true

				  OutputPortal obj debug outFile
				  continue;
				)

	
                occluder = false		
			    -- output name of object and some info
				if (findString obj.name "_occ_" !=undefined) then (
					occluder = true
				)

				-- player barriers
				playerbarrier = getUserProp obj "PLAYERBARRIER"

				if (occluder) then (
					format "\nFound Object Name: % type occluder" obj.name
				) else
					format "\nFound Object Name: % Faces: %" obj.name obj.numfaces

				if (occluder or culleronly=="yes" or playerbarrier=="yes") then (
					format "    <trimesh name=\"%\">\n" obj.name to:outFile
				)
	
			
				-- check for no lighting setting
				lighting = getUserProp obj "LIGHTING"
				
				-- check for colldet setting
				colldet = getUserProp obj "COLLDET"

		
				-- check for viscull setting
				viscull = getUserProp obj "VISCULL"

				-- handles box occluders
				if (occluder and (classOf obj)==Box) then
				(
					format "      <box>\n" to:outFile
					format "      <min x=\"%\" y=\"%\" z=\"%\" />\n"  obj.min.x obj.min.z obj.min.y to:outFile
					format "      <max x=\"%\" y=\"%\" z=\"%\" />\n"  obj.max.x obj.max.z obj.max.y to:outFile
				)

				-- handles non-box occluders
				if (occluder and (classOf obj)!=Box) then
				(
					format "      <mesh>\n" to:outFile
					OutputCullerOnly obj outFile verboseMode debug 
				)

				-- export faces of the mesh
				if (culleronly=="yes" or playerbarrier=="yes") then (
					format "      <mesh>\n" to:outFile
					OutputCullerOnly obj outFile verboseMode debug 
				)

				 -- close params
				if (playerbarrier=="yes") then (
					 format "      </mesh>\n" to:outFile
					 format "      <id>colldet</id>\n" to:outFile
					 format "    </trimesh>\n" to:outFile
					 continue
				) else if (culleronly=="yes" or (occluder and (classOf obj)!=Box)) then (
					 format "      </mesh>\n" to:outFile
					 format "      <id>viscull</id>\n" to:outFile
					 format "    </trimesh>\n" to:outFile
					 continue
				) else if (occluder and (classOf obj)==Box) then (
					 format "      </box>\n" to:outFile
					 format "      <id>viscull</id>\n" to:outFile
					 format "    </trimesh>\n" to:outFile
					 continue
				)
				-- else (
				-- 	format "      </params>\n" to:outFile
					 -- close mesh object
				--	 format "    </meshobj>\n" to:outFile
				--)

                        --------------------
				-- manage genmeshes
				--------------------
				isExplicitGenMesh = false
				objName = obj.name
				if (findString objName "_g_" != undefined) then (
					isExplicitGenMesh = true

					-- skip explicit genmesh factories
					toks = tokenize objName "_"
					if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
						continue;
				)

					if (isExplicitGenMesh) then (
						format "    Working on explicit genmesh %\n" obj.name
					) else (
						format "    Working on genmesh %\n" obj.name
					)

					if (isExplicitGenMesh) then (
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
					)
	
					-- manage LOD objects
					if (isExplicitGenMesh) then
						lodlow = getUserProp genFactObj "LODLOW"

					if (lodlow!=undefined) then (
						format "    <meshref name=\"%\">\n" obj.name to:outFile
					) else (
						format "    <meshobj name=\"%\">\n" obj.name to:outFile
						format "      <plugin>mesh</plugin>\n" obj.name to:outFile
					)

					istrasparent=false
	    
					-- handles sky objects
					if (findString obj.name "_sky_" !=undefined) then (
					  format "      <priority>sky</priority>\n" to:outFile
					  format "      <zuse />\n" to:outFile
					  format "      <key name=\"lighter2\" editoronly=\"yes\" vertexlight=\"yes\" />\n" to:outFile
					)
					else if (culleronly=="yes" or playerbarrier=="yes" or (occluder and (classOf obj)!=Box)) then (
						format "      <mesh>\n" to:outFile
					) else if (occluder and (classOf obj)==Box) then (
						format "      <box>\n" to:outFile
					)

					-- check for no shadow setting
					noshadows = getUserProp obj "NOSHADOWS"
					if (noshadows=="yes") then
						format "      <noshadows />\n" to:outFile
			
					-- for meshref <factory> must go outisde <params>
					if (lodlow!=undefined and isExplicitGenMesh) then
						format "      <factory>%</factory>\n" toks[3] to:outFile

					-- meshref doesn't have <params> tag
					if (lodlow==undefined and isExplicitGenMesh) then (
						format "      <params>\n" to:outFile
						-- for meshref <factory> must go outisde <params>
						format "        <factory>%</factory>\n" toks[3] to:outFile
					)
					
					-- factory for auto genmeshes
					if (not isExplicitGenMesh) then (
						factoryName = "_auto_" + obj.name
						format "      <params>\n        <factory>%</factory>\n" factoryName to:outFile
					)


					-- check for culleronly setting
					culleronly = getUserProp obj "CULLERONLY"
	
					-- check player barrier setting
					playerbarrier = getUserProp obj "PLAYERBARRIER"
		


					-- collect materials
					facesmaterial=#()
	                -- if standard material, then 1 submesh only
					if ((classOf obj.mat)==Standardmaterial) then (
					    append facesmaterial (getMatDiffuseMapFilename (obj.mat))
					) else (
						for i =1 to obj.numFaces do
						(
						    textid = getFaceMatID obj i
							if (textid > obj.mat.count) then (
							  format "ERROR!!! Object % uses more material ID than the materials defined. \n" obj.name
							)
							m = obj.mat[textid]
		   					mname = getMatDiffuseMapFilename (m)
							if (findItem facesmaterial mname==0) then (
								append facesmaterial mname
							)
						)
					)
					format "\n facesmaterial: % \n" facesmaterial

					-- set material to all submeshes
					for i=1 to facesmaterial.count do
					(
					   mname = facesmaterial[i]
					   format "        <submesh name=\"%\">\n          <material>%</material>\n        </submesh>\n" ("submesh"+i as String) mname to:outFile
					)
	
					if (lodlow==undefined) then (
						-- localshadows no more needed for lighter2 levels.
						-- format "      <localshadows />\n" to:outFile
						format "      </params>\n" to:outFile
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
		
					format "      <move>\n        <v x=\"%\" y=\"%\" z=\"%\" />\n" xMove zMove yMove to:outFile
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
					
					-- if it's an genmesh with autogenerated factory then we don't need to check rotation
					if (isExplicitGenMesh) then 
						format "        <matrix><rotx>%</rotx><roty>%</roty><rotz>%</rotz></matrix>\n" rotx roty rotz to:outFile

					format "      </move>\n"  to:outFile

					if (lodlow==undefined) then
						format "    </meshobj>\n" to:outFile
					else
						format "    </meshref>\n" to:outFile


					-- check if this object has a meshgen associated
					meshgen = obj.children[1]
					if (meshgen!=undefined and (findString meshgen.name "_meshgen_" != undefined)) then
					(
						-- TODO check for meshgen applied to multiple objects
						factory1 = getUserProp meshgen "FACTORY1"
						toks = tokenize factory1 ","
						format "    <meshgen name=\"%\">\n" obj.name to:outFile
						format "      <geometry>\n" to:outFile
						format "	  <factory maxdist=\"%\" name=\"%\" />\n" toks[1] toks[2] to:outFile
						format "	  <radius>%</radius>\n" toks[3] to:outFile
						format "	  <density>%</density>\n" toks[4] to:outFile
						format "	  <materialfactor material=\"%\" factor=\"0\" />\n" toks[5] to:outFile
						format "	  <defaultmaterialfactor>1</defaultmaterialfactor>\n" to:outFile
						format "      </geometry>\n" to:outFile
						
						factory2 = getUserProp meshgen "FACTORY2"
						if (factory2!=undefined) then (
							toks = tokenize factory2 ","
							format "      <geometry>\n" to:outFile
							format "	  <factory maxdist=\"%\" name=\"%\" />\n" toks[1] toks[2] to:outFile
							format "	  <radius>%</radius>\n" toks[3] to:outFile
							format "	  <density>%</density>\n" toks[4] to:outFile
							format "	  <materialfactor material=\"%\" factor=\"0\" />\n" toks[5] to:outFile
							format "	  <defaultmaterialfactor>1</defaultmaterialfactor>\n" to:outFile
							format "      </geometry>\n" to:outFile
						)
						celldim = getUserProp meshgen "CELLDIM"
						density = getUserProp meshgen "DENSITY"
						alpha = getUserProp meshgen "ALPHA"
						box = getUserProp meshgen "BOX"
						toksdensity = tokenize density ","
						toksalpha = tokenize alpha ","
						toksbox = tokenize box ","
						format "	<celldim>%</celldim>\n" celldim to:outFile
						format "	<densityscale mindist=\"%\" maxdist=\"%\" maxfactor=\"%\" />\n" toksdensity[1] toksdensity[2] toksdensity[3] to:outFile
						format "	<alphascale mindist=\"%\" maxdist=\"%\" />\n" toksalpha[1] toksalpha[2] to:outFile
						format "	<samplebox>\n" to:outFile
						format "		    <min x=\"-256\" y=\"-200\" z=\"-256\" />\n " toksbox[1] toksbox[2] toksbox[3] to:outFile
						format "		    <max x=\"256\" y=\"200\" z=\"256\" />\n " toksbox[4] toksbox[5] toksbox[6] to:outFile
						format "	</samplebox>\n" to:outFile
						format "    <meshobj>%</meshobj>\n" obj.name to:outFile
						format "     </meshgen>\n" to:outFile
					)

					continue;
		
				--)
		
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
						-- convert lights from 0-255 to 0-1 and uses multiplier
						llred = ((ll.rgb.r)/255) 
						llgreen = ((ll.rgb.g)/255) 
						llblue = ((ll.rgb.b)/255) 
		
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
				format "LIGHTDEBUG: % % %\n" ll.name (lightColors[12][lcount][1]) multiplier
		
				if (ll.useFarAtten==false) then
					format "      <attenuation>none</attenuation>\n" to:outFile
				else
					format "      <attenuation>linear</attenuation>\n" to:outFile
		
				xlight = (ll.pos.x * xscale) + xrelocate
				ylight = (ll.pos.y * yscale) + yrelocate
				zlight = (ll.pos.z * zscale) + zrelocate
				llradius = ll.farAttenEnd * xscale

				llred = ((lightColors[12][lcount][1]))
				llgreen = ((lightColors[12][lcount][2]))
				llblue = ((lightColors[12][lcount][3]))

				format "      <center x=\"%\" y=\"%\" z=\"%\" />\n" xlight zlight ylight to:outFile
			    format "      <radius>%</radius>\n" llradius to:outFile
				format "      <color red=\"%\" green=\"%\" blue=\"%\" />\n" llred llgreen llblue to:outFile
				format "    </light>\n"  to:outFile
				lcount = lcount + 1
			)
		
	
		-- close sector object
		format "  </sector>\n\n" to:outFile
	
		) -- end of sectors loop

		-- outputs camera
		if (startingPos != null) then
		(
			xstart = (startingPos.pos.x * xscale) + xrelocate
			ystart = (startingPos.pos.y * yscale) + yrelocate
			zstart = (startingPos.pos.z * zscale) + zrelocate
	
			format "  <start name=\"%\">\n    <sector>%</sector>\n    <position x=\"%\" y=\"%\" z=\"%\" />\n  </start>\n\n" startingPos.name startingPosSect xstart zstart ystart to:outFile
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
		format "   </sequences>\n\n" to:outFile
	
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
	
		format "   </triggers>\n\n" to:outFile
	
		-- close world object


		format "</world>\n" to:outFile
	
		close outFile 
	
		-- copy textures to output dir
		if (chk2.checked) then (
			test = CopyTexturesToDir outFile 
		)
	

		message = "ALL DONE! \n"

		customPropNumber = fileProperties.findProperty #custom "notes"
		if (customPropNumber!=0) then (
		    notes = fileProperties.getPropertyValue #custom customPropNumber
			message = message + notes
		)

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
