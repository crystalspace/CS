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
-- Version 07

macroScript Export_Sprite_CS
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Export Sprite CS" 
tooltip:"Export Sprite CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export Sprite to CS" width:238 height:345
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export Sprite To:" pos:[21,7] width:142 height:20
	button btn2 "Export!" pos:[42,310] width:152 height:24


	radiobuttons rdo1 "" pos:[15,67] width:218 height:16 labels:#("All Frames", "One Frame")


	edittext edtScale "" pos:[173,98] width:50 height:27
	label lbl2 "Scale:" pos:[131,100] width:40 height:22


	edittext edtVert1 "" pos:[146,187] width:67 height:27
	label lbl15 "Vertex on Foot 1" pos:[24,192] width:103 height:22
	edittext edtVert2 "" pos:[146,220] width:67 height:27
	label lbl16 "Vertex on Foot 2" pos:[26,221] width:113 height:22
	edittext edtFrame "" pos:[146,260] width:67 height:27
	label lbl17 "Frame with Foot2 on ground" pos:[26,259] width:114 height:35


	checkbox chkDispl "Use Displacement" pos:[23,160] width:194 height:22
	GroupBox grp1 "Displacement" pos:[5,134] width:219 height:169


	edittext edtAction "" pos:[53,96] width:67 height:27
	label lbl28 "Action:" pos:[11,100] width:40 height:22




	on Test1 open do
	(
	   name = selection[1].name
	   edt3.text = "d:\\Luca\\"+name+".spr"
	   
	   edtScale.text = "1"
	   
	   edtAction.text = "stand"
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
	
		-- parameters for scaling and relocation
		xscale = edtScale.text as Float
		yscale = edtScale.text as Float
		zscale = edtScale.text as Float
	
		xrelocate = 0
		yrelocate = 0
		zrelocate = 0
	
		animationName = edtAction.text

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
		-- Main: program starts here
		-- ////////////////////////
		
			-- identify main sprite object
			obj=selection[1]
			objectname = obj.name
			image = obj.material.maps[2].filename
			indx = tokenize image "\\"
			materialname = indx[indx.count]

		    -- output name of object and some info
			format "\n\nFound Object Name: % Faces: %\n" objectname obj.numfaces
		
		    --format "    ;Object Name: % Faces: %\n" objectname obj.numfaces to:outFile 
		
			-- write header
			
		    format "<meshfact name=\"%\">\n" objectname to:outFile
		    format "   <plugin>crystalspace.mesh.loader.factory.sprite.3d</plugin>\n" to:outFile
		    format "  <params>\n" to:outFile
		    format "    <material>%</material>\n" materialname to:outFile
		
			frames = #()
		
			redefineFaces=#()

			-- check if model has left-oriented system or not
			face = getface obj 1
			v1= getvert obj face[1]
			v2 = getvert obj face[2]
			v3 = getvert obj face[3]
		
			vect1 = v1-v2
			vect2 = v3-v2
			normal1 = cross vect1 vect2
			normal2 = normal1/(length normal1)
			maxnorm = getfacenormal obj 1
			flipModel = false
		
			for j=1 to 3 do (
				if (maxnorm[j]>0 and normal2[j]>0) then (
					flipModel =true
				)
				if (maxnorm[j]<0 and normal2[j]<0) then (
					flipModel =true
				)
			)
	
			--// end check model orientation

			-- get vertex information for displacement
			displDist = #()
			if (chkDispl.checked) then
			(
				vertex1 = edtVert1.text as Integer
				vertex2 = edtVert2.text as Integer
				middleFrame = edtFrame.text as Integer
			)
			posV1 = undefined
			posV2 = undefined
			firstPosV1 = undefined
			firstPosV2 = undefined

			---------------------------------------
			-- cycle on all frames of the animation
			---------------------------------------
			maxframes = animationrange.end
	
			if (animationrange.end==100f) then
				maxframes = 0;

			if (rdo1.state==2) then
				maxframes=0

			format "maxframes: %" maxframes

			for curFrame=0 to maxframes do (
				-- move to right frame
				slidertime=curFrame
		
				-- get frame name	
				framename = animationName + curFrame as String
				append frames framename
			    format "    <frame name=\"%\">\n" framename to:outFile
	
				-- attach sockets
				-- todo
		
			    -- skip bones
				-- todo
			
				-- check texture
				if (getNumTVerts obj==0) then
				(
					format "No UV maps assigned to object: %\n" obj.name
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
							format " Tface[%]: % curVert: %\n" i Tface[h] curVert

						if (debug and vertTInfo[Tface[h]]==undefined) then
							format "Got one undefined \n"

						-- if the value is different we have a problem on Welded UV
						if (vertTInfo[Tface[h]]!=undefined and vertTInfo[Tface[h]]!=curVert) then (
							if (debug) then
								format "PROBLEM: REDEFINITION\n"
							append redefineFaces i
						)
						vertTInfo[Tface[h]]=curVert
					)
				)
		
				--for h in vertTInfo do
				--(
				--	format "vertTInfo Elem: % " h
				--)
	
				-- cycle on all TVerts of the object
				for i =1 to (getNumTVerts obj) do
				(
					-- skips undefined UV, is that an error?????
					-- the vert seems not used because is welded to another one
					if (vertTInfo[i]==undefined) then
					(
						format "      <v x=\"0\" y=\"0\" z=\"0\" u=\"0\" v=\"0\" /> ; vert not used \n" to:outFile
						continue;
					)

					-- get its 3 vertices as a point3
					-- export in XZY format
				    vert=vertTInfo[i]
					xvert = (vert.x * xscale) + xrelocate
					yvert = (vert.y * yscale) + yrelocate
					zvert = (vert.z * zscale) + zrelocate
		
					Tvert = getTVert obj i
		
					format "      <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> \n" xvert zvert yvert Tvert[1] (1-Tvert[2]) to:outFile
				)

				-- calculate displacement
				if (chkDispl.checked) then
				(
					-- after first frame
					if (curFrame>0) then
					(
						-- first foot
						if (curFrame<=middleFrame) then
						(
							format " frame: % posV1: %\n" curFrame posV1
							prevV1 = posV1
							posV1 = getVert obj vertex1
						    dist1 = distance posV1 prevV1
							format " dist: % \n" dist1
	
							append displDist dist1
						)
						-- second foot
						else if (curFrame>=(middleFrame+1)) then (
						  format " frame: % posV2: %\n" curFrame posV2
							prevV2 = posV2
							posV2 = getVert obj vertex2
						    dist2 = distance posV2 prevV2
							format " dist: % \n" dist2
	
							append displDist dist2
						)

					-- first frame
					) else (
						firstPosV1 = getVert obj vertex1
						posV1 = getVert obj vertex1
					)

					-- on last frame calc displ for first one
					if (curFrame==maxframes) then
					(
						posV1 = getVert obj vertex1
						dist1 = distance firstPosV1 posV1
						insertItem dist1 displDist 1
					)

					-- on middle frame store info for middle-next one
					if (curFrame==middleFrame) then
					(
						posV2 = getVert obj vertex2
					)
				)
	
				format "    </frame>\n\n" to:outFile
			)

			-- export action 
			format "    <action name=\"%\">" animationName to:outFile
			countF=1
			for f in frames do (
				if (chkDispl.checked) then
				(
					format" <f name=\"%\" displacement=\"%\" /> " f displDist[countF] to:outFile
				) else
					format" <f name=\"%\" delay=\"100\" /> " f to:outFile
				countF = countF + 1
			)
			format " </action> \n\n" to:outFile

			-- if action is walk then export backwalk also
			if (animationName == "walk") then
			(
				format "    <action name=\"backwalk\">" to:outFile
				countF = frames.count
				-- for with "by -1" crashes for unknown reasons, so we use a workaround
				for f=1 to countF do (
					if (chkDispl.checked) then
					(
						revIter = countF-f+1
						format " <f name=\"%\" displacement=\"%\" /> " frames[revIter] displDist[revIter]  to:outFile
					) else
						format " <f name=\"%\" delay=\"100\" /> " f to:outFile
				)
				format " </action> \n\n" to:outFile
			)

			-- cycle on all faces of object
			for i =1 to obj.numFaces do
			(
				faceVerts=getTVface obj i
				a = (faceVerts[1]-1) as Integer
				b = (faceVerts[3]-1) as Integer
				c = (faceVerts[2]-1) as Integer

				if (flipModel) then
					format "    <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" a c b to:outFile
				else
					format "    <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" a b c to:outFile
		
		    )
		
		format "      <smooth>yes</smooth>\n" to:outFile
		format "    </params>\n" to:outFile
		format "</meshfact>\n" to:outFile
		
		close outFile 
	
		if (redefinefaces.count!=0) then (
			format "\n\nFaces that need UV Unwield: %" redefinefaces
			message = "Some faces need UV Unwield, look at MaxScript Listener Window for faces #\n";
			message = message + "MODEL NEEDS FIXING!"
		) else
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
-- This means that some UV coords are shared on multiple vetexes
-- Go in the UVUnwrap function and Unweld all the vertexes of the face found
-------------------
