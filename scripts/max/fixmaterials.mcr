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
-- Version 04

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
		    if (mat==undefined) then
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
		-- Main: program starts here
		-- ////////////////////////
	
		  materialsWrittenToWorld=#()

		for obj1 in objects do (
	
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
	
			for obj2 in objects do (
				if (obj1==obj2) then
					continue
	
				numMat2=0
				-- determine number of materials on object
				if (obj2.mat == undefined) then (
					format "   OBJECT HAS NO TEXTURE! % " obj2.name
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

					if (material1==material2) then
					(
					    format "Assigning material of obj % to obj %\n" obj1.name obj2.name
						obj2.mat = obj1.mat
					)
				) else if ((classOf obj1.mat)==Multimaterial and (classOf obj2.mat)==Multimaterial) then
				(
					-- only if checked
					if (not chkMulti.checked) then
						continue

					format "multimaterial found on obj % and obj % \n" obj1.name obj2.name
	
					jj = 1
					identical = true
					for subm1 in obj1.mat do (
						submat1 = getMatFilename obj1.mat[jj]
						submat2 = getMatFilename obj2.mat[jj]
	
						if (submat1!=submat2) then
						(
							format "found a difference % % at submat % \n" submat1 submat2 jj
							identical=false
							break
						) else
							format "submat equal % % at submat % \n" submat1 submat2 jj
						jj = jj + 1
					)
	
					if (identical) then
					(
					    format "Assigning material of obj % to obj %\n" obj1.name obj2.name
						obj2.mat = obj1.mat
					)
				)
	
			)
		)
	
		message = "ALL DONE!\nPlease save the scene and reload it.\n"
		messageBox message
	
	)
)

gw = newRolloutFloater "Fix Duplicate Materials" 300 220 
addRollout Test1 gw 

)

