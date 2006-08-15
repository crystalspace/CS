macroScript Export_GenMesh_Skelanim_CS
category:"Crystal Space"
internalcategory:"Crystal Space"
ButtonText:"Export GenMesh Skelanim CS" 
tooltip:"Export GenMesh Skelanim CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export GenMesh Skelanim to CS" width:238 height:345
(
	edittext edt3 "" pos:[17,32] width:192 height:21
	label lbl1 "Export GenMesh Skelanim To:" pos:[21,7] width:145 height:20
	button btn2 "Export!" pos:[42,110] width:152 height:24

	edittext edtScale "" pos:[57,68] width:50 height:27
	label lbl2 "Scale:" pos:[20,70] width:40 height:22

	on Test1 open do
	(
		edt3.text = "D:\\genmesh"
		edtScale.text = "1"
	)

	on btn2 pressed do
	(
		filename = edt3.text
		outFile = createFile filename
		debug=false
		global xscale = edtScale.text as Float
		global yscale = edtScale.text as Float
		global zscale = edtScale.text as Float
	
		global xrelocate = 0
		global yrelocate = 0
		global zrelocate = 0
	
		-- functions declaration
		global tokenize
		global find_object_by_name
		global export_bones_transform
		struct bone_vertex_weight (bone, v_idx, weight)
		struct vertex_convert_data (vtx_id, original_vtx_id)
		struct biped_transform (biped_name, pos, rotx, roty, rotz)

		-- Tokenize utility function
		fn tokenize instring sep = 
		(
			outarray = #()
			temp=copy instring
			i = 1
			while (true) do 
			(
				index = findstring temp sep
				if (index==undefined) then
				(
					outarray[i]=temp
					exit
				)
				else 
				(
					outarray[i] = substring temp 1 (index-1)
					temp = substring temp (index+1) -1
				)
				i = i + 1
			)
			outarray
		)

		fn find_object_by_name obj_name = 
		(
			result = undefined
			for o in objects do
			(
				if (o.name == obj_name) then
				(
					result = o
					exit
				)
			)
			result
		)

		fn export_bones_transform obj bone_obj spd bones_verts outFile=
		(
			format "%<bone name=\"%\">\n" spd bone_obj.name to:outFile

			help_tr = undefined
			if (bone_obj.parent != undefined) then
			(
				local_rotation = (bone_obj.transform*(inverse bone_obj.parent.transform))as Quat
				local_position = bone_obj.transform.pos *(inverse bone_obj.parent.transform)
				rotvalues = quattoeuler local_rotation order:2
			
				local xMove = (local_position.x * xscale) + xrelocate
				local yMove = (local_position.y * yscale) + yrelocate
				local zMove = (local_position.z * zscale) + zrelocate
				format "%  <move>\n" spd to:outFile
				format "%    <v x=\"%\" y=\"%\" z=\"%\" />\n" spd xMove zMove yMove to:outFile

				rotx = (rotvalues.x * pi)/180
				roty = -((rotvalues.y) * pi)/180
				rotz = (rotvalues.z * pi)/180

				format "%    <matrix>\n" spd to:outFile
				format "%      <rotx>%</rotx>\n" spd rotx to:outFile
				format "%      <roty>%</roty>\n" spd roty to:outFile
				format "%      <rotz>%</rotz>\n" spd rotz to:outFile
				format "%    </matrix>\n" spd to:outFile
				format "%  </move>\n" spd to:outFile

				for bv in bones_verts do
				(
					if (bv.bone == bone_obj) then
						format "%  <vertex idx=\"%\" weight=\"%\"/>\n" spd bv.v_idx bv.weight to:outFile
				)
			)
			else
			(
				position = bone_obj.transform.pos
				local xMove = (position.x * xscale) + xrelocate
				local yMove =(position.y * yscale) + yrelocate
				local zMove = (position.z * zscale) + zrelocate

				format "%  <move>\n" spd to:outFile
				format "%    <v x=\"%\" y=\"%\" z=\"%\" />\n" spd xMove zMove yMove to:outFile

				rotation = bone_obj.transform as Quat
				rotvalues = quattoeuler rotation order:2
				rotx = (rotvalues.x * pi)/180
				roty = -((rotvalues.y) * pi)/180
				rotz = (rotvalues.z * pi)/180

				format "%    <matrix>\n" spd to:outFile
				format "%      <rotx>%</rotx>\n" spd rotx to:outFile
				format "%      <roty>%</roty>\n" spd roty to:outFile
				format "%      <rotz>%</rotz>\n" spd rotz to:outFile
				format "%    </matrix>\n" spd to:outFile
				format "%  </move>\n" spd to:outFile

				for bv in bones_verts do
				(
					if (bv.bone == bone_obj) then
						format "%  <vertex idx=\"%\" weight=\"%\"/>\n" spd bv.v_idx bv.weight to:outFile
				)
			)

			for child in bone_obj.children do
			(
				export_bones_transform obj child (spd + "  ") bones_verts outFile
			)
			format "%</bone>\n" spd to:outFile
		)

		obj = undefined
		slidertime = 0

		main_object_index = 0

		root_bone = undefined
		for i=1 to objects.count do 
		(
			if ((classOf objects[i] == Biped_Object or classOf objects[i] == BoneGeometry or (classOf objects[i]) == Bone) and (objects[i].parent == undefined)) then
				root_bone = objects[i]
			if (classOf objects[i] == Editable_Mesh) then
				obj = objects[i]
		)

		--m = root_bone.transform.controller.figureMode = true
		-------------------------------------GENMESH--------------------------------------

		objectname = obj.name
		--image = obj.material.maps[2].filename
		--indx = tokenize image "\\"
		--materialname = indx[indx.count]
		materialname = "JPG"

		-- output name of object and some info
		format "\n\nFound Object Name: % Faces: %\n" objectname obj.numfaces

		-- write header
		format "  <meshfact name=\"%\">\n"objectname to: outFile
		format "    <plugin>crystalspace.mesh.loader.factory.genmesh</plugin>\n" to: outFile
		format "    <params>\n" to: outFile
		format "      <numvt>%</numvt>\n" (getNumTVerts obj) to: outFile
		format "      <numtri>%</numtri>\n" obj.numfaces to:outFile

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
		vertTInfoIndeces = #()
		vertTInfoTVIndeces = #(getNumTVerts obj)
		vertTInfoTVIndeces[1] = undefined

		-- cycle on all Faces of the object to get Tverts x,y,z positions
		for i=1 to obj.numfaces do
		(
			-- get face
			Tface = getTVFace obj i
			face = getFace obj i

			-- get its 3 vertices
			for h=1 to 3 do 
			(
				curVert=getVert obj face[h]
				
				if (curVert==undefined) then
					format "\n\nUNDEF: %\n\n" Tface[h]
				if (debug) then
					format " face: % curVert %: %\n" i Tface[h] curVert

				-- if the value is different we have a problem on Welded UV
				if (vertTInfo[Tface[h]]!=undefined and vertTInfo[Tface[h]]!=curVert) then 
				(

					message = "PROBLEM on object " + obj.name + ": welded UV on vertex " + (Tface[h] as String)
					messageBox message
					close outFile
					return 1
				)
				vertTInfo[Tface[h]]=curVert
				vertTInfoTVIndeces[Tface[h]] = face[h]
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
			append vertTInfoIndeces (vertex_convert_data i vertTInfoTVIndeces[i])

			--format "        <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> %\n" xvert zvert yvert Tvert[1] (1-Tvert[2]) vertTInfoTVIndeces[i] to:outFile
			format "        <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> \n" xvert zvert yvert Tvert[1] (1-Tvert[2]) to:outFile
		)

		format "\n" to:outFile

		-- cycle on all faces of object
		for i =1 to obj.numFaces do
		(
			faceVerts=getTVface obj i
			a = (faceVerts[1]-1) as Integer
			b = (faceVerts[3]-1) as Integer
			c = (faceVerts[2]-1) as Integer

			format "        <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" b c a to:outFile
			--format "        <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" c b a to:outFile
		)
		
		format "    <autonormals />\n" to:outFile

		----------------------------------------------------------------------------------

		obj_physique = undefined
		obj_skin = undefined
		for i = 1 to obj.modifiers.count do
		(
			if (classOf obj.modifiers[i] == Physique) then
			(
				obj_physique = obj.modifiers[i]
			)
		)

		if (obj_physique == undefined) then
		(
			for i = 1 to obj.modifiers.count do
			(
				if (classOf obj.modifiers[i] == Skin) then
				(
					obj_skin = obj.modifiers[i]
				)
			)
		)

		bones_vertices =#()
		for v=1 to (getNumTVerts obj) do 
		(
			original_vtx_id = undefined
			for vtx_data in vertTInfoIndeces do
			(
				if (v == vtx_data.vtx_id) then
					original_vtx_id = vtx_data.original_vtx_id
			)

			if (obj_physique != undefined) then
			(
				for b=1 to physiqueOps.getVertexBoneCount obj original_vtx_id do
				(
					bone = physiqueOps.getVertexBone obj original_vtx_id b
					weight = physiqueOps.getVertexWeight obj original_vtx_id b
					b_found = false
					for bv in bones_vertices do
					(
						if ((bv.bone == bone) and (bv.v_idx == (v - 1))) then
						(
							bv.weight = bv.weight + weight
							b_found = true
						)
					)

					if (b_found == false) then
					(
						append bones_vertices (bone_vertex_weight bone (v-1) weight )
					)
				)
			)
			else
			if (obj_skin != undefined) then
			(
				affect_bones_count = skinOps.GetVertexWeightCount obj_skin original_vtx_id
				for ab = 1 to affect_bones_count do
				(
					weight = skinOps.GetVertexWeight obj_skin original_vtx_id ab
					bone_index = skinOps.GetVertexWeightBoneID obj_skin original_vtx_id ab
					bone = find_object_by_name (skinOps.GetBoneName obj_skin bone_index 0)

					b_found = false
					for bv in bones_vertices do
					(
						if ((bv.bone == bone) and (bv.v_idx == (v - 1))) then
						(
							bv.weight = bv.weight + weight
							b_found = true
						)
					)

					if (b_found == false) then
					(
						append bones_vertices (bone_vertex_weight bone (v-1) weight )
					)
				)
			)
		)

		format "\n" to:outFile
		format "    <animcontrol plugin=\"crystalspace.mesh.anim.skeleton\">\n" to:outFile

		export_bones_transform obj root_bone "      " bones_vertices outFile

		format "\n" to:outFile
		format "      <script name=\"main\">\n" to:outFile

		----------------------ANIMATION SCRIPT---------------------------
		format "\n" to:outFile

		key_frame_times = #()
		for t in objects do
		(
			if ((classOf t) == Biped_Object or (classOf t) == BoneGeometry or (classOf t) == Bone) then
			(
				try
				(
					bip = t.transform.controller
					for t in bip.keys do
					(
						found = false
						for fr in key_frame_times do
						(
							if (t.time == fr) then
							(
								found = true
							)
						)
						if (found == false) then
						(
							append key_frame_times t.time
						)
					)
				)
				catch
				(
				)

				try
				(
					bip = t.rotation.controller
					for t in bip.keys do
					(
						found = false
						for fr in key_frame_times do
						(
							if (t.time == fr) then
							(
								found = true
							)
						)
						if (found == false) then
						(
							append key_frame_times t.time
						)
					)
				)
				catch
				(
				)

				try
				(
					bip = t.position.controller
					for t in bip.keys do
					(
						found = false
						for fr in key_frame_times do
						(
							if (t.time == fr) then
							(
								found = true
							)
						)
						if (found == false) then
							append key_frame_times t.time
					)
				)
				catch
				(
				)

			)
		)

		biped_transforms = #()
		sort key_frame_times

		for t in objects do
		(
			if ((classOf t) == Biped_Object or (classOf t) == BoneGeometry or (classOf t) == Bone) then
			(
				if (t.parent != undefined) then
				(
					biped_rotation = (t.transform*(inverse t.parent.transform))as Quat
					biped_position = t.transform.pos *(inverse t.parent.transform)
					rotvalues = quattoeuler biped_rotation order:2
					base_rotx = rotvalues.x
					base_roty = rotvalues.y
					base_rotz = rotvalues.z
					
					append biped_transforms (biped_transform t.name biped_position base_rotx base_roty base_rotz)
				)
				else
				(
					b_transform = t.transform
					biped_rotation = (b_transform) as Quat
					biped_position = b_transform.pos
					rotvalues = quattoeuler biped_rotation order:2
					base_rotx = rotvalues.x
					base_roty = rotvalues.y
					base_rotz = rotvalues.z
					append biped_transforms (biped_transform t.name biped_position base_rotx base_roty base_rotz)
				)
			)
		)

		--m = root_bone.transform.controller.figureMode = false

		for kf=2 to key_frame_times.count do
		(
			slidertime = key_frame_times[kf]
			duration = ((key_frame_times[kf] as integer - key_frame_times[kf - 1] as integer))/4 as integer
			biped_animation_found = false
			for t in objects do
			(
				--if ((classOf t) == Biped_Object and t.name == "Bip01 L Thigh") then
				if ((classOf t) == Biped_Object or (classOf t) == BoneGeometry or (classOf t) == Bone) then
				(
					biped_base_transform = undefined

					bp_index = 0
					for i=1 to biped_transforms.count do
					(
						if (biped_transforms[i].biped_name == t.name) then
						(
							bp_index = i
						)
					)

					biped_rotation = undefined
					biped_position = undefined
					if (t.parent != undefined) then
					(
						biped_rotation = (t.transform*(inverse t.parent.transform)) as Quat
						biped_position = t.transform.pos *(inverse t.parent.transform)
					)
					else
					(
						b_transform = t.transform
						biped_rotation = (b_transform) as Quat
						biped_position = b_transform.pos
					)

					write_x_rot = false
					write_y_rot = false
					write_z_rot = false

					new_rotvalues = quattoeuler biped_rotation order:2

					if abs(new_rotvalues.x - biped_transforms[bp_index].rotx) > 0.0001 then
						write_x_rot = true;
					if abs(new_rotvalues.y - biped_transforms[bp_index].roty) > 0.0001 then
						write_y_rot = true;
					if abs(new_rotvalues.z - biped_transforms[bp_index].rotz) > 0.0001 then
						write_z_rot = true;

					if write_x_rot or write_y_rot or write_z_rot then
					(
						biped_animation_found = true
						final_rotx = (new_rotvalues.x * pi)/180
						final_roty = -(new_rotvalues.y * pi)/180
						final_rotz = (new_rotvalues.z * pi)/180
						format "        <rot bone=\"%\" duration=\"%\" x=\"%\" y=\"%\" z=\"%\"/>\n" t.name duration final_rotx final_roty final_rotz to:outFile

					)

					write_x_rot = false
					write_y_rot = false
					write_z_rot = false
					if (abs(biped_position.x - biped_transforms[bp_index].pos.x) > 0.0001) then
						write_x_rot = true
					if (abs(biped_position.y - biped_transforms[bp_index].pos.y) > 0.0001) then
						write_y_rot = true
					if (abs(biped_position.z - biped_transforms[bp_index].pos.z) > 0.0001) then
						write_z_rot = true

					--if (t.parent != undefined) and (write_x_rot or write_y_rot or write_z_rot) then
					if write_x_rot or write_y_rot or write_z_rot then
					(
						biped_animation_found = true
						local xMove = (biped_position.x * xscale) + xrelocate
						local yMove = (biped_position.z * yscale) + yrelocate
						local zMove = (biped_position.y * zscale) + zrelocate
						format "        <move bone=\"%\" duration=\"%\" x=\"%\" y=\"%\" z=\"%\"/>\n" t.name duration xMove yMove zMove to:outFile
					)

					biped_transforms[bp_index].pos = biped_position
					biped_transforms[bp_index].rotx = new_rotvalues.x
					biped_transforms[bp_index].roty = new_rotvalues.y
					biped_transforms[bp_index].rotz = new_rotvalues.z
				)
			)
			if (biped_animation_found == true) then
				format "        <delay time=\"%\" />\n\n" duration to:outFile
		)

		format "\n" to:outFile
		-----------------------------------------------------------------

		format "        <repeat />\n" to:outFile
		format "      </script>\n" to:outFile
		format "      <run script=\"main\"/>\n" to:outFile
		format "    </animcontrol>\n" to:outFile
		format "  </params>\n" to:outFile
		format "</meshfact>\n" to:outFile

-------------------------------------------------------------------------------
		close outFile 
		message = "Done!"
		messageBox message
	
	)
)

gw = newRolloutFloater "Export GenMesh Animation" 300 170 
addRollout Test1 gw 
)