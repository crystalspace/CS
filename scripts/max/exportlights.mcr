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
-- Version 05

macroScript Export_Lights_PS
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Export Lights" 
tooltip:"Export Lights" Icon:#("Maxscript",1)
(

rollout Test1 "Export Lights" width:222 height:142
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export Lights To:" pos:[21,7] width:142 height:20
	button btn2 "Export!" pos:[37,110] width:152 height:24

	on Test1 open do
	(
	   edt3.text = "D:\\Luca\\lights124.xml"
	)

	on btn2 pressed do
	(

		-- functions declaration
		global tokenize

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

		-- ////////////////////////
		-- Variables used in the program
		-- ////////////////////////


		-- check frames number: 24 frames in total
		maxframes = animationrange.end

		if (animationrange.end != 23f and animationrange.end != 47f) then
		(
			message = "You have to set the animation length to 24 frames (sun only) or to 48 (sun/rain). \n Each frame is 1 hour of the day."
			messageBox message
			return 1
		)

		if (animationrange.end == 23f) then
		(
			message = "You have set 24 frames in the animation, so full sun and heavy rain condition will have the same light settings. \n Export will now proceed."
			messageBox message
		)

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
		)

		-- get filename
		filename = edt3.text
	
		-- output file
		outFile = createFile filename
		
		-- set debug output
		debug=false
		
		-- Define verbose output (that takes more space and memory)
		verboseMode = true
	
		-- this indicates if only differences must be printed.
		optimized = false

		-- ////////////////////////
		-- Main: program starts here
		-- ////////////////////////

		-- write header
		format "<lighting>\n" to:outFile


		-- retrieve all lights objects
		lightsFound = #()
		hasMoreSectors = "false"
		sectors= #();
		format "total objects % \n" objects.count
		for i=1 to objects.count do 
		(
		    obj=objects[i];
			if (findString obj.name "_sect_" !=undefined) then (
				hasMoreSectors = "true"
				toks = tokenize obj.name "_"
				append sectors toks[3]
			)
			-- skip everything that is not a light
			if ( (classOf obj) != Omnilight) then (
				continue
			)
			append lightsFound obj
		)
		
		format "lightsFound: % \n" lightsFound 
		format "sectors: % \n" sectors 


		-- get lights information
		lightColors = #()
		lightInfo = #()

		format "Dynamic Lights \n"
		-- cycle on all frames of the animation
		-- to find out which lights are dynamic
		-- and to get their color info
		fcount = 1
		for curFrame=0 to maxframes do (
			-- move to right frame
			slidertime=curFrame

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
							lightInfo[lcount] = "dynamic"
					)
				)
				insertItem #(llred,llgreen,llblue) fLights lcount

				lcount = lcount + 1
			)
			insertItem fLights lightColors fcount
			fcount = fcount + 1
		)
		
		format "lightInfo: % \n" lightInfo

		-- reset slider time (used mainly to avoid problem in getting last frame data)
		slidertime=0

		if (optimized) then
		(
			-- skip first hour, lights are the same as defined.
			-- for each hour of the day
			fcount = 2
			for fcount=2 to 24 do
			(
				-- for each light
				lcount = 1
				for ll in lightsFound do
				(

					if (lightInfo.count>=lcount and lightInfo[lcount]=="dynamic") then
					(
						fullname = name + ll.parent.name
						-- if light changed
						prevColor = lightColors[fcount-1][lcount]
						colors = lightColors[fcount][lcount]
						if ( prevColor[1] != colors[1] or prevColor[2] != colors[2] or prevColor[3] != colors[3]) then
						(
							-- pseudo-dynamic ambient light
							if (ll.name=="ambient") then (
								-- support for multi sectors
								if (hasMoreSectors=="true") then (
									for sec in sectors do (
										format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" />\n" (fcount-1) sec colors[1] colors[2] colors[3] to:outFile
									)
								) else
									format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" />\n" (fcount-1) roomname colors[1] colors[2] colors[3] to:outFile
							-- pseudo-dynamic light
							) else
								format " <color value=\"%\" object=\"%%\" type=\"light\" r=\"%\" g=\"%\" b=\"%\" />\n" (fcount-1) fullname roomname colors[1] colors[2] colors[3] to:outFile
						)
					)
					lcount = lcount + 1
				)
				fcount = fcount + 1
			)

		) else (

			-- intensity factor for autodynamic lights
			intensityfactor = #(0,0,0,0,0,0.3,0.4,0.6,0.8,0.8,0.8,0.8,1,1,1,1,0.8,0.8,0.8,0.8,0.6,0.4,0.3,0)

			-- cycle on all frames of the animation
			fcount = 1
			for curFrame=0 to 23f do (

				-- cycle on all lights found in the scene
				lcount = 1
				for obj in lightsFound do 
				(
					colors = lightColors[fcount][lcount]
					rainColors = #()
					--format "colors: %\n" colors
					
					-- for multiple sectors
					if ( obj.parent!=undefined and (classOf obj.parent)==Dummy and (findString obj.parent.name "_sect_" !=undefined) ) then (
						toks = tokenize obj.parent.name "_"
						roomname = toks[3]
					)

					-- if light is autodynamic
					if (lightInfo.count>=lcount and lightInfo[lcount]=="autodynamic") then
					(
						-- calculate colors
						colors[1]=colors[1]*intensityfactor[fcount]
						colors[2]=colors[2]*intensityfactor[fcount]
						colors[3]=colors[3]*intensityfactor[fcount]
						rainColors[1]=colors[1]*intensityfactor[fcount]*0.8
						rainColors[2]=colors[2]*intensityfactor[fcount]*0.8
						rainColors[3]=colors[3]*intensityfactor[fcount]*0.8

						-- pseudo-dynamic ambient light
						if (obj.name=="ambient") then (
							-- support for multi sectors
							if (hasMoreSectors=="true") then (
								for sec in sectors do (
									format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" />\n" (fcount-1) sec colors[1] colors[2] colors[3] to:outFile
								)
							) else
								format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" rain_r=\"%\" rain_g=\"%\" rain_b=\"%\" />\n" (fcount-1) roomname colors[1] colors[2] colors[3] rainColors[1] rainColors[2] rainColors[3] to:outFile
						-- pseudo-dynamic light
						) else
							format " <color value=\"%\" object=\"%%\" type=\"light\" r=\"%\" g=\"%\" b=\"%\" rain_r=\"%\" rain_g=\"%\" rain_b=\"%\" />\n" (fcount-1) obj.name roomname colors[1] colors[2] colors[3] rainColors[1] rainColors[2] rainColors[3] to:outFile

					)
					-- if light is dynamic
					else if (lightInfo.count>=lcount and lightInfo[lcount]=="dynamic") then
					(
						-- get rain values
						if (maxframes == 47f ) then
							rainColors = lightColors[fcount+24][lcount]
						else
							rainColors = lightColors[fcount][lcount]

						-- pseudo-dynamic ambient light
						if (obj.name=="ambient") then (
							-- support for multi sectors
							if (hasMoreSectors=="true") then (
								for sec in sectors do (
									format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" />\n" (fcount-1) sec colors[1] colors[2] colors[3] to:outFile
								)
							) else
								format " <color value=\"%\" object=\"%\" type=\"ambient\" r=\"%\" g=\"%\" b=\"%\" rain_r=\"%\" rain_g=\"%\" rain_b=\"%\" />\n" (fcount-1) roomname colors[1] colors[2] colors[3] rainColors[1] rainColors[2] rainColors[3] to:outFile
						-- pseudo-dynamic light
						) else
							format " <color value=\"%\" object=\"%%\" type=\"light\" r=\"%\" g=\"%\" b=\"%\" rain_r=\"%\" rain_g=\"%\" rain_b=\"%\" />\n" (fcount-1) obj.name roomname colors[1] colors[2] colors[3] rainColors[1] rainColors[2] rainColors[3] to:outFile
					)
					lcount = lcount + 1
				)
				fcount = fcount + 1
			)
		)

		-- close lighting object
		format "</lighting>\n" to:outFile
		close outFile 
	
		message = "ALL DONE!"
		messageBox message
	
	)
)

gw = newRolloutFloater "Export Lights" 300 220 
addRollout Test1 gw 

)
