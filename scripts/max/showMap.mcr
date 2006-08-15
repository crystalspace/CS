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
macroScript ShowMaps
category:"PlaneShift"
internalcategory:"PlaneShift"
ButtonText:"Show Texture Maps" 
tooltip:"Show Texture Maps" Icon:#("Maxscript",1)
(

rollout Test1 "Show Maps" width:150 height:101
(
	button btn1 "Set" pos:[24,57] width:110 height:22


	radiobuttons rdo1 "Show Maps" pos:[24,9] width:94 height:30 labels:#("On", "Off")

	on btn1 pressed do
	(
		if (rdo1.state==1) then
			showTextures = true
		else
			showTextures = false
	
		for obj in objects do (
		    m = obj.material
		    if (m==undefined) then
				continue;
		
		    -- handle Multi/materials
			if ((classOf m)==Multimaterial) then (
				for subm in m do (
					showTextureMap subm subm.diffuseMap showTextures 
				)
			)
		
		    -- handle Standardmaterials
			else if ((classOf m)==Standardmaterial) then (
				showTextureMap m m.diffuseMap showTextures 
			)
		)
	
	)
)


gw = newRolloutFloater "Show Texture Maps" 200 160 
addRollout Test1 gw 


)

