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
-- Version 22

macroScript Export_Level_CS
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Export Level CS" 
tooltip:"Export Level CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export Level to CS" width:222 height:191
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export Level To:" pos:[21,7] width:142 height:20
	button btn2 "Export!" pos:[37,110] width:152 height:24
	label lbl6 "Scale:" pos:[58,72] width:40 height:20
	edittext edtScale "" pos:[101,67] width:63 height:27
	checkbox chkLights "Generate Fake lights for walktest" pos:[4,146] width:215 height:20 enabled:true
	on Test1 open do
	(
	   edt3.text = "D:\\Luca\\temple124"
	   edtScale.text = "0.01"	
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
		
		-- define if export for Dynavis system
		dynavis=true
		
		-- Combine polys
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
	
		-- ////////////////////////
		-- Write material function
		-- ////////////////////////
		
		fn WriteMaterials outFile =
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
				if (findItem materialsWrittenToWorld image==0) then (
					format "    <texture name=\"%\"> <file>%</file></texture>\n" image image to:outFile
					append materialsWrittenToWorld image
				)
			)
	
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatFilename subm
					if (findItem materialsWrittenToWorld image==0) then (
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
				if (findItem materialsWrittenToWorld image==0) then (
				    format "    <material name=\"%\"> <texture>%</texture></material>\n" image image to:outFile
					append materialsWrittenToWorld image
				)
			)
		
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					image = getMatFilename subm				
					if (findItem materialsWrittenToWorld image==0) then (
						format "    <material name=\"%\"> <texture>%</texture></material>\n" image image to:outFile
						append materialsWrittenToWorld image
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
		-- Output Particle function
		-- ////////////////////////
		fn OutputParticle obj outFile =
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
					regparticle2 = tokenize regparticle ","
					lighting = getUserProp obj "LIGHTING"
					totaltime = getUserProp obj "TOTALTIME"
					startpostype = getUserProp obj "STARTPOS1"
					startpostype = lowercase startpostype
					startpos = getUserProp obj "STARTPOS2"
					index = findString startpos ","
					startposp = substring startpos 1 (index-1)
					startposq = substring startpos (index+1) -1
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
					-- search attractor position using attractor name found in ATTRACTOR entry
					attractorobj=null
					for attract in objects do if (attract.name==attractor) then attractorobj=attract
					if (attractorobj==null) then ( format "NO OBJECT FOUND AS ATTRACTOR OF %" obj.name )
					xattractorobj = (attractorobj.pos.x * xscale) + xrelocate
					yattractorobj = (attractorobj.pos.y * yscale) + yrelocate
					zattractorobj = (attractorobj.pos.z * zscale) + zrelocate
		
					attractorforce = getUserProp obj "ATTRACTORFORCE"
					aging0 = getUserProp obj "AGING0"
					aging0 = tokenize aging0 ","
					aging1 = getUserProp obj "AGING1"
					aging1 = tokenize aging1 ","
					aging2 = getUserProp obj "AGING2"
					aging2 = tokenize aging2 ","
					aging3 = getUserProp obj "AGING3"
					aging3 = tokenize aging3 ","
					aging4 = getUserProp obj "AGING4"
					aging4 = tokenize aging4 ","
		
					format "    <meshobj name=\"%\">\n" obj.name to:outFile
					format "      <priority>alpha</priority>\n" to:outFile
					format "      <plugin>emit</plugin>\n" to:outFile
					format "      <ztest />\n" to:outFile
					format "      <move> <v x=\"0\" y=\"0\" z=\"0\" /> </move>\n" to:outFile
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
					format "	    <regularparticles sides=\"%\" radius=\"%\" /> \n" regparticle2[1] regparticle2[2] to:outFile
					format "	    <lighting>%</lighting> \n" lighting to:outFile
					format "	    <totaltime>%</totaltime> \n" totaltime to:outFile
					format "	    <startpos><% x=\"%\" y=\"%\" z=\"%\" p=\"%\" q=\"%\" /></startpos>\n" startpostype xpart zpart ypart startposp startposq to:outFile
					format "	    <startspeed><%><min x=\"%\" y=\"%\" z=\"%\" /> \n" startspeedtype startspeedarray[1] startspeedarray[2] startspeedarray[3] to:outFile
					format "	    <max x=\"%\" y=\"%\" z=\"%\" /></%></startspeed> \n" startspeedarray[4] startspeedarray[5] startspeedarray[6] startspeedtype to:outFile
					format "	    <startaccel><% x=\"%\" y=\"%\" z=\"%\" /></startaccel>\n" startacctype startaccarray[1] startaccarray[2] startaccarray[3] to:outFile
					format "	    <attractor> <emitfixed x=\"%\" y=\"%\" z=\"%\" /> </attractor>\n" xattractorobj zattractorobj yattractorobj to:outFile
					format "	    <attractorforce>%</attractorforce>\n" attractorforce to:outFile
					format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging0[1] aging0[2] aging0[3] aging0[4] to:outFile
					format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging0[5] aging0[6] aging0[7] aging0[8] to:outFile
					format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging0[1] aging1[2] aging1[3] aging1[4] to:outFile
					format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging1[5] aging1[6] aging1[7] aging1[8] to:outFile
					format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging2[1] aging2[2] aging2[3] aging2[4] to:outFile
					format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging2[5] aging2[6] aging2[7] aging2[8] to:outFile
					format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging3[1] aging3[2] aging3[3] aging3[4] to:outFile
					format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging3[5] aging3[6] aging3[7] aging3[8] to:outFile
					format "	    <aging> <time>%</time><color red=\"%\" green=\"%\" blue=\"%\" /> \n" aging4[1] aging4[2] aging4[3] aging4[4] to:outFile
					format "	    <alpha>%</alpha><swirl>%</swirl><rotspeed>%</rotspeed><scale>%</scale></aging>\n" aging4[5] aging4[6] aging4[7] aging4[8] to:outFile
					format "      </params>\n" to:outFile
					format "    </meshobj>\n" to:outFile
				)
		)
	
		-- ////////////////////////
		-- Main: program starts here
		-- ////////////////////////

		-- get room name
		customPropNumber = fileProperties.findProperty #custom "roomname"
		if (customPropNumber==0) then (
			messageBox "Please click on File>File Properties and add a Custom Property called roomname with the name of the sector."
			return 1
		)
		roomName = fileProperties.getPropertyValue #custom customPropNumber 
	
		if (roomName==undefined) then (
		   format "ERROR: Please set a custom property named roomname"
		   return 1
		) else (
		   format "Roomname: %\n" roomName
		)
	
		-- output file
		outFile = createFile filename		
		
		-- write header
		
		    format "<world>\n" to:outFile
		
		-- Check particles: cycle on all objects in the scene
		for obj in objects do 
		(
		    if ( (classOf obj)==Point) then (
				type = getUserProp obj "TYPE"
				format "Found Particle Object: %\n Type: %" obj.name type
				if (type == "fire") then ( fireNeeded = true )
				else if (type == "emit") then ( emitNeeded = true )
				
				-- check for additional materials
				addMaterial = getUserProp obj "MATERIAL"
				if (addMaterial != undefined) then
					append partMaterials addMaterial
			)
		)
	
	
		-- write materials		
	    WriteMaterials outFile
	
		-- write plugins
	    format "  <plugins>\n" to:outFile
	    format "    <plugin name=\"thing\">crystalspace.mesh.loader.thing</plugin>\n" to:outFile
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
	    format "  </plugins>\n\n" to:outFile
	
		-- write particles declarations
	  	if (fireNeeded) then (
		    format "    <meshfact name=\"fireFact\"><plugin>fireFact</plugin><params /></meshfact>\n\n" to:outFile
	    )
	  	if (emitNeeded) then (
		    format "    <meshfact name=\"emitFact\"><plugin>emitFact</plugin><params /></meshfact>\n\n" to:outFile  
	  	)
	
		-- search and write meshfacts
		factoryMeshes = #()
		for obj in objects do 
		(
			redefineFaces=#()
	
			-- Check for genmeshes factories
			objName = obj.name
			if (findString objName "_g_" !=undefined) then (
				-- factory must have name like : _g_name_0
				toks = tokenize objName "_"
				if (toks.count == 4 and objName[objName.count-1] == "_" and objName[objName.count] == "0") then
				(
					append factoryMeshes obj
					format "  <meshfact name=\"%\"><plugin>meshFact</plugin><params><numvt>%</numvt><numtri>%</numtri>\n" toks[3] (getNumTVerts obj) obj.numfaces to:outFile
	
					-- check texture
					if (getNumTVerts obj==0) then
					(
						message = "Export aborted: No UV maps assigned to genmesh object: " + obj.name
						messageBox message
						close outFile 
						return 1
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
	
								message = "PROBLEM on object " + obj.name + ": welded UV on vertex " + (Tface[h] as String)
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
						b = (faceVerts[3]-1) as Integer
						c = (faceVerts[2]-1) as Integer
	
						format "    <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" a b c to:outFile
				    )
					
					format "    <autonormals /></params></meshfact>\n" a b c to:outFile
				)
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
	
		-- Setting for dynavis
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
	
		format "</settings>\n\n" to:outFile
	
	
		-- start sector
	    format "  <sector name=\"%\">\n" roomName to:outFile
	
		if (dynavis) then
			format "    <cullerp>crystalspace.culling.dynavis</cullerp>\n" to:outFile
	
		format "IF YOU GET ANY ERROR convert to editable mesh and collapse stack of offending object\n"
	
	
		lightsFound = #()
		startingPos = null
	
	
	
		------------------------------------
		-- cycle on all objects in the scene
		------------------------------------ 
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
		
			-- convert particles
			if ( (classOf obj)==Point) then (
				format "Found particle Object: %\n" obj.name
				OutputParticle obj outFile
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
	
				format "    <meshobj name=\"%\">\n" obj.name to:outFile
				-- check for no shadow setting
				noshadows = getUserProp obj "NOSHADOWS"
				if (noshadows=="yes") then
					format "      <noshadows />\n" to:outFile
				format "      <plugin>mesh</plugin>\n" obj.name to:outFile
				format "      <params><factory>%</factory>\n" toks[3] to:outFile
				format "      <material>%</material></params>\n" (getMatFilename obj.mat) to:outFile
				-- search factory
				genFactObj=null
				format "factoryMeshes: %\n" factoryMeshes.count
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
				-- calc distance from factory
				--xMove = ((obj.pos.x-genFactObj.pos.x) * xscale) + xrelocate
				--yMove = ((obj.pos.y-genFactObj.pos.y) * yscale) + yrelocate
				--zMove = ((obj.pos.z-genFactObj.pos.z) * zscale) + zrelocate
				xMove = (obj.pos.x * xscale) + xrelocate
				yMove = (obj.pos.y * yscale) + yrelocate
				zMove = (obj.pos.z * zscale) + zrelocate
	
				format "      <move><v x=\"%\" y=\"%\" z=\"%\" />\n" xMove zMove yMove to:outFile
				rotvalues = quattoeuler obj.rotation order:2
				rotx = (rotvalues.x * pi)/180
				roty = (rotvalues.y * pi)/180
				rotz = (rotvalues.z * pi)/180
				format "      <matrix><rotx>%</rotx><roty>%</roty><rotz>%</rotz></matrix>\n" rotx roty rotz to:outFile
				format "      </move>\n"  to:outFile
				format "    </meshobj>\n" to:outFile
				continue;
	
			)
	
		    -- output name of object and some info
			format "\n\nFound Object Name: % Faces: %\n" obj.name obj.numfaces
		
		    --format "    ;Object Name: % Faces: %\n" obj.name obj.numfaces to:outFile 


		
		    format "    <meshobj name=\"%\">\n" obj.name to:outFile
			format "      <plugin>thing</plugin>\n" to:outFile
	
			isportal=false
	
			-- handles transparent objects
			if (findString obj.name "_t_" !=undefined) then (
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
		    -- handles portal objects
			else if (findString obj.name "_p_" !=undefined) then (
				format "      <priority>wall</priority>\n" to:outFile
				isportal=true
			)
			else (
			  format "      <priority>object</priority>\n" to:outFile
			  format "      <zuse />\n" to:outFile
			)
	
			-- check for no shadow setting
			noshadows = getUserProp obj "NOSHADOWS"
			if (noshadows=="yes") then
				format "      <noshadows />\n" to:outFile
	
			-- check for no lightning setting
			lighting = getUserProp obj "LIGHTING"
			
			-- check for colldet setting
			colldet = getUserProp obj "COLLDET"
	
	
			format "      <params>\n" to:outFile
	
			-- check for smooth setting
			smooth = getUserProp obj "SMOOTH"
			if (smooth=="yes") then
				format "      <smooth />\n" to:outFile
	
	
		   -- output vertexes of the object in XZY format
		   for v in obj.verts do
		   (
		     xvert = (v.pos.x * xscale) + xrelocate
			 yvert = (v.pos.y * yscale) + yrelocate
			 zvert = (v.pos.z * zscale) + zrelocate
		     format "      <v x=\"%\" y=\"%\" z=\"%\" />\n"  xvert zvert yvert to:outFile
		   )
	

		   format "\n" to:outFile
	
		   -- list of faces already printed
		   skiplist = #()
		   numMat = 0
	
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
	
			-- defult csinvisibletexture to no
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
				    	currentMat = obj.mat.name
					else
						currentMat = obj.mat[matID].name
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
								texturefilename=getMatFilename obj.mat
								if (texturefilename=="csinvisible.tga") then
									csinvisibletexture="yes"
								else
									csinvisibletexture="no"
						    	format "      <material>%</material>\n" texturefilename to:outFile
							) else (
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
		
						if (verboseMode) then
						(
						    polyName = obj.name + "_"+ i as String;
					    	format "      <p name=\"%\">" polyName to:outFile
						) else
							format "      <p>" to:outFile
		
						format "      <v>%</v><v>%</v><v>%</v><v>%</v> \n" (a-1) (b-1) (c-1) (d-1) to:outFile
		
						if (debug) then format "      <v>%</v><v>%</v><v>%</v><v>%</v> \n" (a-1) (b-1) (c-1) (d-1)
	
						-- handles no lighting objects and invisible objects
						if (lighting=="no" or csinvisibletexture=="yes") then
							format "      <lighting>no</lighting>\n" to:outFile
	
						-- handles no collision detection objects
						if (colldet=="no" or csinvisibletexture=="yes") then
							format "      <colldet>no</colldet>\n" to:outFile
						
						-- additional parameter to have right visibility culler on invisible objects
						if (csinvisibletexture=="yes") then
							format "      <viscull>yes</viscull>\n" to:outFile
	
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
								format "<uv idx=\"%\" u=\"%\" v=\"%\" />" f u v to:outFile
								outputV = outputV + 1
							)
	
							--if (outputV<3) then format "," to:outFile
	
							f=f+1
						)
						format "</texmap></p>\n" to:outFile
	
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
					    polyName = obj.name + "_" + i as String;
				    	format "      <p name=\"%\"> " polyName to:outFile
					) else
						format "      <p> " to:outFile
	
					format "      <v>%</v><v>%</v><v>%</v>\n" (a-1) (b-1) (c-1) to:outFile
	
					-- handles no lighting objects and invisible objects
					if (lighting=="no" or csinvisibletexture=="yes") then
						format "      <lighting>no</lighting>\n" to:outFile
	
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
	
					format "      <texmap><uv idx=\"0\" u=\"%\" v=\"%\" /><uv idx=\"1\" u=\"%\" v=\"%\" /><uv idx=\"2\" u=\"%\" v=\"%\" /></texmap>\n" aTV[1] (1-aTV[2]) bTV[1] (1-bTV[2]) cTV[1] (1-cTV[2]) to:outFile
					format "      </p>\n" to:outFile
	
				)
			)
		 )
	
		 -- close params
		 format "      </params>\n" to:outFile
	
		 -- close mesh object
		 format "    </meshobj>\n" to:outFile
	
		 format "\n" to:outFile
	
		  --print (getUserPropBuffer obj) to:outFile -- output buffer as a string literal 
		  gc()
		) 
	
		-- get info on dynamic lights
		maxframes = animationrange.end
	
		if (maxframes != 23f and chkLights.checked) then
		(
			message = "You have to set the animation length to 24 frames total. \n Each frame is 1 hour of the day."
			messageBox message
			return 1
		)

		format "Lights\n"
		
		lightColors = #()
		lightInfo = #()
		if (chkLights.checked) then
		(
			format "Dynamic Lights \n"
			-- cycle on all frames of the animation
			fcount = 1
			for curFrame=0 to maxframes do (
				-- move to right frame
				slidertime=curFrame
				
				-- outputs lights
				lcount = 1
				fLights = #()
				for ll in lightsFound do
				(
					format "Lights % \n" lcount
					-- convert lights from 0-255 to 0-1
					llred = ((ll.rgb.r)/255) * ll.multiplier
					llgreen = ((ll.rgb.g)/255) * ll.multiplier
					llblue = ((ll.rgb.b)/255) * ll.multiplier

					if (fcount!=1) then
					(
						--format "Comparing % with % \n" lightColors[fcount-1][lcount] #(llred,llgreen,llblue)
						prevColor = lightColors[fcount-1][lcount]
						if ( prevColor[1] != llred or prevColor[2] != llgreen or prevColor[3] != llblue) then
							lightInfo[lcount] = "dynamic"
					)
					insertItem #(llred,llgreen,llblue) fLights lcount

					lcount = lcount + 1
				)
				insertItem fLights lightColors fcount
				fcount = fcount + 1
			)

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
		    --format " ;Light: % \n" ll.name to:outFile
			format "    <light name=\"%\">\n" ll.name to:outFile
	
			if (lightInfo.count>=lcount and lightInfo[lcount]=="dynamic") then 
				format "      <dynamic />\n " to:outFile
		
			multiplier = ll.multiplier
	
			if (ll.useNearAtten==false and ll.useFarAtten==false) then
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
	
		-- outputs camera
		if (startingPos != null) then
		(
			xstart = (startingPos.pos.x * xscale) + xrelocate
			ystart = (startingPos.pos.y * yscale) + yrelocate
			zstart = (startingPos.pos.z * zscale) + zrelocate
	
			format "  <start name=\"%\"><sector>%</sector><position x=\"%\" y=\"%\" z=\"%\" /></start>\n" startingPos.name roomName xstart zstart ystart to:outFile
		)

		-- output Fake Sequence for testing dynamic lights
		if (chkLights.checked) then
		(
			format "   <triggers>\n" to:outFile
			format "     <trigger name=\"simulation\">\n" to:outFile
			format "       <sectorvis sector=\"%\" />\n" roomName to:outFile
			format "       <fire sequence=\"main\" />\n" to:outFile
			format "     </trigger>\n" to:outFile
			format "   </triggers>\n" to:outFile
			format "   <sequences>\n" to:outFile
			format "     <sequence name=\"main\">\n" to:outFile

			-- skip first hour, lights are the same as defined.
			format "       <delay>10000</delay>\n" to:outFile
			-- for each hour of the day
			fcount = 2
			for fcount=2 to 24 do
			(
				-- for each light
				lcount = 1
				for ll in lightsFound do
				(
					-- if light is dynamic
					if (lightInfo.count>=lcount and lightInfo[lcount]=="dynamic") then
					(
						-- if light changed 
						prevColor = lightColors[fcount-1][lcount]
						colors = lightColors[fcount][lcount]
						if ( prevColor[1] != colors[1] or prevColor[2] != colors[2] or prevColor[3] != colors[3]) then
							format "       <fadelight light=\"%\" red=\"%\" green=\"%\" blue=\"%\" duration=\"10000\" />\n" ll.name colors[1] colors[2] colors[3] to:outFile
					)
					lcount = lcount + 1
				)
				format "       <delay>10000</delay>\n" to:outFile

				fcount = fcount + 1
			)
			format "       <enable trigger=\"simulation\" />\n" to:outFile
			format "     </sequence>\n" to:outFile
			format "   </sequences>\n" to:outFile

		)

		-- close world object
		format "</world>\n" to:outFile
	
		close outFile 
	
		message = "ALL DONE!"
		messageBox message
	
	)
)

gw = newRolloutFloater "Export Level" 300 220 
addRollout Test1 gw 

)
