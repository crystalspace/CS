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
-- Version 06

macroScript Fix_Duplicate_Materials
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Fix Duplicate Materials" 
tooltip:"Fix Duplicate Materials" Icon:#("Maxscript",1)
(

rollout Test1 "Fix Duplicate Materials" width:222 height:142
(
	button btn2 "Fix!" pos:[35,97] width:152 height:24

	checkbox chkStd "Fix Standard Materials" pos:[35,19] width:174 height:27
	checkbox chkMulti "Fix Multi Materials" pos:[33,51] width:150 height:23

	on Test1 open do
	(
	   chkStd.checked = true
	   chkMulti.checked = true
	)

	on btn2 pressed do
	(
	
		-- functions declaration
		global tokenize
		global getMatFilename
	
		-- debug
		debug = false
		
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
			else
			(
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
		-- Main: program starts here
		-- ////////////////////////
	
		  materialsWrittenToWorld=#()

		for objcount1=1 to objects.count do (
			format "object: %\n" objcount1

			obj1=objects[objcount1]
	
			-- skip unwanted objects
			if ((classOf obj1)==Targetcamera or (classOf obj1)==Targetobject or (classOf obj1)==Point or (classOf obj1)==Omnilight or (classOf obj1)==Dummy) then
				continue

			numMat1 = 0
			-- determine number of materials on object
			if (obj1.mat == undefined) then (
				format "   OBJECT HAS NO TEXTURE! % " obj1.name
				continue
			)
			else if ((classOf obj1.mat)==Standardmaterial) then
				numMat1 = 1
			else
				numMat1 = obj1.mat.count
	
			if (numMat1==0) then
				continue

			for objcount2=objcount1 to objects.count do (
				obj2 = objects[objcount2]
				if (obj1==obj2) then
					continue

				-- skip unwanted objects
				if ((classOf obj2)==Targetcamera or (classOf obj2)==Targetobject or (classOf obj2)==Point or (classOf obj2)==Omnilight or (classOf obj2)==Dummy) then
					continue

				numMat2=0
				-- determine number of materials on object
				if (obj2.mat == undefined) then (
					format "   OBJECT HAS NO TEXTURE! % \n" obj2.name
					continue
				)
				else if ((classOf obj2.mat)==Standardmaterial) then
					numMat2 = 1
				else
					numMat2 = obj2.mat.count
	
				if (numMat2==0) then
					continue

				-- check if is already same material
				if (obj1.mat==obj2.mat) then
					continue

				-- compare materials
				if ((classOf obj1.mat)==Standardmaterial and (classOf obj2.mat)==Standardmaterial) then
				(
					-- only if checked
					if (not chkStd.checked) then
						continue

					material1 = getMatFilename obj1.mat
					material2 = getMatFilename obj2.mat
					if (debug) then
						format "obj1:% material1:% obj2:% material2:%\n" obj1.name obj2.name material1 material2

					if (material1==material2) then
					(
						if (debug) then
					    	format "Assigning material of obj % to obj %\n" obj1.name obj2.name

						obj2.mat = obj1.mat
					)
				) else if ((classOf obj1.mat)==Multimaterial and (classOf obj2.mat)==Multimaterial) then
				(
					-- only if checked
					if (not chkMulti.checked) then
						continue
					if (debug) then
						format "multimaterial found on obj % and obj % \n" obj1.name obj2.name
	
					-- check if number of submaterial is different
					if (obj1.mat.count!=obj2.mat.count) then
						continue

					jj = 1
					identical = true
					for subm1 in obj1.mat do (
						submat1 = getMatFilename obj1.mat[jj]
						submat2 = getMatFilename obj2.mat[jj]
	
						if (submat1!=submat2) then
						(
							if (debug) then
								format "found a difference % % at submat % \n" submat1 submat2 jj
							identical=false
							exit
						) else (
							if (debug) then
								format "submat equal % % at submat % \n" submat1 submat2 jj
						)
						jj = jj + 1
					)
	
					if (identical) then
					(
						if (debug) then
						    format "Assigning material of obj % to obj %\n" obj1.name obj2.name
						obj2.mat = obj1.mat
					)
				)
			
				objcount2 = objcount2 + 1
			)
			objcount1 = objcount1 + 1
		)

		message = "ALL DONE!\nPlease save the scene and reload it.\n"
		messageBox message
	
	)
)

gw = newRolloutFloater "Fix Duplicate Materials" 300 220 
addRollout Test1 gw 

)

