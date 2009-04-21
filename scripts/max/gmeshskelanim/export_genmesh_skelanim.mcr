macroScript Export_GenMesh_Skelanim2_CS
category:"Crystal Space"
internalcategory:"Crystal Space"
ButtonText:"Export GenMesh Skelanim 2 CS" 
tooltip:"Export GenMesh Skelanim 2 CS" Icon:#("Maxscript",1)
(

rollout Test1 "Export GenMesh Skelanim 2 to CS" width:238 height:445
(
	label lbl1 "Export GenMesh Model To Directory:" pos:[21,7] width:200 height:20
	edittext edt3 "" pos:[17,22] width:192 height:21

	button btn2 "Export model!" pos:[70,210] width:135 height:24

	edittext edtScale "" pos:[50,68] width:50 height:22
	label lbl2 "Scale:" pos:[20,70] width:30 height:22

	checkbox chkParentDummyMovable "Parent dummy movable" pos:[150, 80] height:20 checked:true

	checkbox chkposx "PosX" pos:[150, 95] height:20
	checkbox chkposy "PosY" pos:[150, 110] height:20
	checkbox chkposz "PosZ" pos:[150, 125] height:20

	checkbox chkrotx "RotX" pos:[200, 95] height:20
	checkbox chkroty "RotY" pos:[200, 110] height:20
	checkbox chkrotz "RotZ" pos:[200, 125] height:20

	on Test1 open do
	(
		edt3.text = "E:\\"
		edtScale.text = "1"
	)

	global err_counter
	global open_edges
	struct tvedge (v1, v2, f1, f2)
	global tvedge

	global vert_info
	struct vert_info (v_idx, u, v)

	global object_is_bone
	fn object_is_bone obj =
	(
		result = false
		if (
			(obj.name != "Bip01 Footsteps") and (classOf obj == Biped_Object or classOf obj == BoneGeometry or (classOf obj) == Bone) or (classOf obj == Dummy and (obj.name == "root"))
		) then
		      result = true
		result
	)

	on btn2 pressed do
	(
		filename = edt3.text + "genmesh"
		filename_skeleton = edt3.text + "genmesh_skeleton"
		outFile = createFile filename
		outFile_skeleton = createFile filename_skeleton
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
		struct biped_transform (biped_name, tr)

		global bones_list
		bones_list = #()

		global vert2bones
		struct vert2bones (v_idx, the_bones)
		global bone_weight
		struct bone_weight (b_id, b_weight, b_name)

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

		global object_is_socket
		fn object_is_socket obj =
		(
			result = false
			if (
				(classOf obj == Dummy) and (findString obj.name "_socket_" != undefined)
			) then
			      result = true
			result
		)

		global fill_bones_list
		fn fill_bones_list bones_list bone =
		(
			append bones_list bone

			for child in bone.children do
			(
				if (object_is_bone child) then
				(
					fill_bones_list bones_list child
				)
			)
		)

		fn export_bones_transform obj bone_obj spd bones_verts outFile=
		(
			element_type = "bone"
			if (object_is_socket bone_obj) then
			(
				element_type = "socket"
			)
			format "%<% name=\"%\">\n" spd element_type bone_obj.name to:outFile

			help_tr = undefined
			if (bone_obj.parent != undefined and bone_obj != root_bone/*and bone_obj.name != "helper_bone"*/) then
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

				--format "%    helper\n" spd to:outFile
				format "%    <matrix>\n" spd to:outFile
				format "%      <rotx>%</rotx>\n" spd rotx to:outFile
				format "%      <roty>%</roty>\n" spd roty to:outFile
				format "%      <rotz>%</rotz>\n" spd rotz to:outFile
				format "%    </matrix>\n" spd to:outFile
				format "%  </move>\n" spd to:outFile
			)

			bone_id = findItem bones_list bone_obj
			if ((bone_id > 0) and bones2verts[bone_id] != undefined) then
			(
				min_x = 0
				min_y = 0
				min_z = 0
				max_x = 0
				max_y = 0
				max_z = 0
				for v in bones2verts[bone_id] do
				(
					transformed_vertex = v*(inverse bone_obj.transform)
					if transformed_vertex.x < min_x then
					(
						min_x = transformed_vertex.x
					)
					if transformed_vertex.y < min_y then
					(
						min_y = transformed_vertex.y
					)
					if transformed_vertex.z < min_z then
					(
						min_z = transformed_vertex.z
					)
					if transformed_vertex.x > max_x then
					(
						max_x = transformed_vertex.x
					)
					if transformed_vertex.y > max_y then
					(
						max_y = transformed_vertex.y
					)
					if transformed_vertex.z > max_z then
					(
						max_z = transformed_vertex.z
					)
				)

				format "%  <skinbox>\n" spd to:outFile
				format "%    <min x=\"%\" y=\"%\" z=\"%\"/>\n" spd min_x min_z min_y to:outFile
				format "%    <max x=\"%\" y=\"%\" z=\"%\"/>\n" spd max_x max_z max_y to:outFile
				format "%  </skinbox>\n" spd to:outFile

				ragdoll = getUserProp bone_obj "RAGDOLL"
				if (ragdoll == "disabled") then
				(
					format "%  <ragdoll>\n" spd to:outFile
					format "%    <disabled/>\n" spd to:outFile
					format "%  </ragdoll>\n" spd to:outFile
				)
				else if (ragdoll == "parent") then
				(
					format "%  <ragdoll>\n" spd to:outFile
					format "%    <parent/>\n" spd to:outFile
					format "%  </ragdoll>\n" spd to:outFile
				)
				else
				(
					format "%  <ragdoll>\n" spd to:outFile
					format "%    <geom name=\"%\">\n" spd bone_obj.name to:outFile
					format "%      <box><skinbox/></box>\n" spd to:outFile
					format "%      <friction>10</friction>\n" spd to:outFile
					format "%      <elasticity>0.1</elasticity>\n" spd to:outFile
					format "%      <softness>0.2</softness>\n" spd to:outFile
					format "%      <slip>0.3</slip>\n" spd to:outFile
					format "%    </geom>\n" spd to:outFile
					format "%    <body name=\"%\">\n" spd bone_obj.name to:outFile
					format "%      <mass>1</mass>\n" spd to:outFile
					format "%      <gravmode>1</gravmode>\n" spd to:outFile
					format "%    </body>\n" spd to:outFile
					if (bone_obj.parent != undefined) then
					(
						format "%    <joint name=\"% -> %\">\n" spd bone_obj.parent.name bone_obj.name to:outFile
						format "%      <rotconstraints>\n" spd to:outFile
						format "%        <min x=\"0\" y=\"-0.3\" z=\"0\"/>\n" spd to:outFile
						format "%        <max x=\"0\" y=\"0.7\" z=\"0\"/>\n" spd to:outFile
						format "%      </rotconstraints>\n" spd to:outFile
						format "%      <transconstraints>\n" spd to:outFile
						format "%        <min x=\"0\" y=\"0\" z=\"0\"/>\n" spd to:outFile
						format "%        <max x=\"0\" y=\"0\" z=\"0\"/>\n" spd to:outFile
						format "%      </transconstraints>\n" spd to:outFile
						format "%    </joint>\n" spd to:outFile
						format "%  </ragdoll>\n" spd to:outFile
					)
				)
			)


			for child in bone_obj.children do
			(
				if (child != undefined) then
				(
					if ((object_is_bone child) or (object_is_socket child)) then
					(
						export_bones_transform obj child (spd + "  ") bones_verts outFile
					)
				)
			)
			format "%</%>\n" spd element_type to:outFile
		)

		obj = undefined
		slidertime = 0

		main_object_index = 0

		global root_bone = undefined
		for i=1 to objects.count do 
		(
			if (classOf objects[i] == Editable_Mesh /*and objects[i].name == "objdefault"*/) then
			(
				obj = objects[i]
			)
		)

		--format "ROOT BONE %\n" root_bone to:outFile
		for i=1 to objects.count do 
		(
			if (object_is_bone  objects[i]) then
			(
				
				root_bone = objects[i]
				fill_bones_list bones_list root_bone
				exit
			)
		)

		-------------------------------------GENMESH--------------------------------------

		global bones2verts = #(bones_list.count)
		global orig_vertex_map = #()
		bones2verts[1] = undefined

		if (getNumTVerts obj==0) then
		(
			message = "Export aborted: No UV maps assigned to genmesh object: " + obj.name
			messageBox message
			close outFile 
			return 1
		)

		fn export_material_mesh obj mat_id outFile spd close_factory =
		(
			vertTInfo = #(getNumTVerts obj)
			vertTInfo[1] = undefined
			vertTInfoIndeces = #()
			vertTInfoTVIndeces = #(getNumTVerts obj)
			vertTInfoTVIndeces[1] = undefined
			struct vertex_convert_data (vtx_id, original_vtx_id)

			tvface_verts = #()
			mat_faces = #()
			for i=1 to obj.numfaces do
			(
				matID = getFaceMatID obj i
				if (matID == mat_id) then
				(
					append mat_faces i
					tvface = getTVFace obj i
					face = getface obj i
					for i = 1 to 3 do
					(
						curVert=getVert obj face[i]
						vertTInfo[tvface[i]] = curVert
						vertTInfoTVIndeces[tvface[i]] = face[i]
						v_idx = findItem tvface_verts tvface[i]
						if (v_idx == 0) then
						(
							append tvface_verts tvface[i]
						)
					)
				)
			)

			sort tvface_verts
			obj_name = obj.material[mat_id].name
			image = obj.material[mat_id].maps[2].filename
			indx = tokenize image "\\"
			materialname = indx[indx.count]

			format "%  <meshfact name=\"%\">\n" spd (obj.name + "_" +(mat_id as string)) to: outFile
			format "%    <plugin>crystalspace.mesh.loader.factory.genmesh</plugin>\n" spd to: outFile
			format "%    <params>\n" spd to: outFile
			format "%      <material>%</material>\n" spd materialname to: outFile
			format "%      <numvt>%</numvt>\n" spd (tvface_verts.count) to: outFile
			format "%      <numtri>%</numtri>\n" spd (mat_faces.count) to:outFile

			for i =1 to tvface_verts.count do
			(
				-- get its 3 vertices as a point3
				-- export in XZY format
				vert=vertTInfo[tvface_verts[i]]
				xvert = (vert.x * xscale) + xrelocate
				yvert = (vert.y * yscale) + yrelocate
				zvert = (vert.z * zscale) + zrelocate

				Tvert = getTVert obj tvface_verts[i]
				append vertTInfoIndeces (vertex_convert_data i vertTInfoTVIndeces[tvface_verts[i]])

				format "%      <v x=\"%\" y=\"%\" z=\"%\" u=\"%\" v=\"%\" /> \n" spd xvert zvert yvert Tvert[1] (1-Tvert[2]) to:outFile
			)

			format "\n" to:outFile
			for i =1 to mat_faces.count do
			(
				faceVerts=getTVface obj mat_faces[i]
				idx_a = findItem tvface_verts faceVerts[1]
				idx_b = findItem tvface_verts faceVerts[2]
				idx_c = findItem tvface_verts faceVerts[3]
				a = (idx_a - 1) as Integer
				b = (idx_b - 1) as Integer
				c = (idx_c - 1) as Integer
				format "%      <t v1=\"%\" v2=\"%\" v3=\"%\" />\n" spd b a c to:outFile
			)
			format "%      <autonormals />\n" spd to:outFile


			--------------------------------------------------
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
			vertices2bones_weights = #()
			vertices2bones_indices = #()
			vert_bones = #()
			struct original_vertex_bone_data (indices, weights)
			original_vertices_bone_data = #(obj.numverts)
			for v=1 to vertTInfoIndeces.count do 
			(
				original_vtx_id = vertTInfoIndeces[v].original_vtx_id
				v2bw = vert2bones v #(0.0, 0.0, 0.0, 0.0)
				v2bi = vert2bones v #(0, 0, 0, 0)

				append vertices2bones_weights v2bw
				append vertices2bones_indices v2bi

				if (obj_physique != undefined) then
				(
					bones_weights = #()
					for b=1 to physiqueOps.getVertexBoneCount obj original_vtx_id do
					(
						bone = physiqueOps.getVertexBone obj original_vtx_id b
						vert_bone = findItem vert_bones bone
						if (vert_bone == 0) then
						(
							append vert_bones bone
						)

						weight = physiqueOps.getVertexWeight obj original_vtx_id b
						bone_id = findItem vert_bones bone
						global_bone_id = findItem bones_list bone
						if weight > 0 then
						(
							if (bones2verts[global_bone_id] == undefined) then
							(
								bones2verts[global_bone_id] = #()
							)
							append bones2verts[global_bone_id] (getVert obj original_vtx_id)
						)

						bw = bone_weight bone_id weight bone_name
						b_pos = 0
						for k = 1 to bones_weights.count do
						(
							if bw.b_weight > bones_weights[k].b_weight then
							(
								b_pos = k
								exit
							)
						)

						if b_pos > 0 then
						(
							insertItem bw bones_weights b_pos
						)
						else
						(
							append bones_weights bw
						)

						--if (b_found == false) then
						--(
							--append bones_vertices (bone_vertex_weight bone (v-1) weight )
						--)
					)
					bwcount = bones_weights.count
					if bwcount > 4 then
					(
						bwcount = 4
					)
					for ab = 1 to bwcount do
					(
						b_id = bones_weights[ab].b_id
						if (b_id > 0) then
						(
							v2bi.the_bones[ab] = ((bones_weights[ab].b_id - 1) as integer)
							v2bw.the_bones[ab] = bones_weights[ab].b_weight
						)
					)
				)
				else
				if (obj_skin != undefined) then
				(
					affect_bones_count = skinOps.GetVertexWeightCount obj_skin original_vtx_id

					bones_weights = #()

					for ab = 1 to affect_bones_count do
					(
						weight = skinOps.GetVertexWeight obj_skin original_vtx_id ab
						bone_index = skinOps.GetVertexWeightBoneID obj_skin original_vtx_id ab
						bone_name = skinOps.GetBoneName obj_skin bone_index 0
						bone = getnodebyname bone_name
						vert_bone = findItem vert_bones bone
						if (vert_bone == 0) then
						(
							append vert_bones bone
						)

						--bone_id = findItem bones_list bone
						bone_id = findItem vert_bones bone

						bw = bone_weight bone_id weight bone_name
						b_pos = 0
						for k = 1 to bones_weights.count do
						(
							if bw.b_weight > bones_weights[k].b_weight then
							(
								b_pos = k
								exit
							)
						)

						if b_pos > 0 then
						(
							insertItem bw bones_weights b_pos
						)
						else
						(
							append bones_weights bw
						)

						b_found = false
						for bv in bones_vertices do
						(
							if ((bv.bone == bone) and (bv.v_idx == (v - 1))) then
							(
								bv.weight = bv.weight + weight
								b_found = true
							)
						)

						--if (b_found == false) then
						--(
							--append bones_vertices (bone_vertex_weight bone (v-1) weight )
						--)
					)

					bwcount = bones_weights.count
					if bwcount > 2 then
					(
						bwcount = 2
					)
					for ab = 1 to bwcount do
					(
						b_id = bones_weights[ab].b_id
						if (b_id > 0) then
						(
							v2bi.the_bones[ab] = ((bones_weights[ab].b_id - 1) as integer)
							v2bw.the_bones[ab] = bones_weights[ab].b_weight
						)
					)
				)
			)

			format "      <renderbuffer name=\"bones indices\" components=\"4\" type=\"short\">\n" to:outFile
			for b=1 to vertices2bones_weights.count do
			(
				idx_1 = vertices2bones_indices[b].the_bones[1]
				idx_2 = vertices2bones_indices[b].the_bones[2]
				idx_3 = vertices2bones_indices[b].the_bones[3]
				idx_4 = vertices2bones_indices[b].the_bones[4]
				
				format "        <e c0=\"%\" c1=\"%\" c2=\"%\" c3=\"%\" />\n" idx_1 idx_2 idx_3 idx_4 to:outFile
			)
			format "      </renderbuffer>\n\n" to:outFile
			
			format "      <renderbuffer name=\"bones weights\" components=\"4\" type=\"float\">\n" to:outFile
			for b=1 to vertices2bones_weights.count do
			(
				w_1 = vertices2bones_weights[b].the_bones[1] as float
				w_2 = vertices2bones_weights[b].the_bones[2] as float
				w_3 = vertices2bones_weights[b].the_bones[3] as float
				w_4 = vertices2bones_weights[b].the_bones[4] as float
				
				format "        <e c0=\"%\" c1=\"%\" c2=\"%\" c3=\"%\" />\n" w_1 w_2 w_3 w_4 to:outFile
			)
			format "      </renderbuffer>\n\n" to:outFile
			--------------------------------------------------

			format "\n" to:outFile
			if (mat_id == 1) then
			(
				format "    <animcontrol plugin=\"crystalspace.mesh.anim.skeleton2\">\n" to:outFile
				format "      <skelfile>/models/riddick/riddick_skeleton.xml</skelfile>\n" to:outFile
				format "      <use_bones>\n" to:outFile
				for bone in vert_bones do
				(
					if (bones_list != undefined) then
					(
						bone_id = findItem bones_list bone
						bone_id = bone_id - 1
						format "        <bone>%</bone>\n" bone_id to:outFile
					)
				)
				format "      </use_bones>\n" to:outFile
				format "    </animcontrol>\n" to:outFile
			)
			else
			(
				format "    <animcontrol plugin=\"crystalspace.mesh.anim.skeleton2\">\n" to:outFile
				format "      <skelfact>\n" to:outFile
				format "        <use_parent/>\n" to:outFile
				format "      </skelfact>\n" to:outFile
				format "      <use_bones>\n" to:outFile
				for bone in vert_bones do
				(
					if (bones_list != undefined) then
					(
						bone_id = findItem bones_list bone
						bone_id = bone_id - 1
						format "        <bone>%</bone>\n" bone_id to:outFile
					)
				)
				format "      </use_bones>\n" to:outFile
				format "    </animcontrol>\n" to:outFile
			)

			format "%    </params>\n" spd to: outFile

			if (close_factory == true) then
			(
				format "%  </meshfact>\n" spd to: outFile
			)
		)

		if (classOf obj.mat == Multimaterial) then
		(
			mat_id = undefined
			i = 1
			while (mat_id == undefined) do
			(
				if (obj.mat[i] != undefined) then
				(
					mat_id = i
				)
			)
			
			if (mat_id != undefined) then
			(
				export_material_mesh obj mat_id outFile "" false
				for i = (mat_id + 1) to obj.mat.count do
				(
					if (obj.mat[i] != undefined) then
					(
						export_material_mesh obj i outFile "  " true
					)
				)
				format "  </meshfact>\n" to: outFile
			)
		)
		else
		(
			--TODO - standard material
		)

		format "\n" to:outFile
		skel_name = obj.name + "_skelfact"
		format "    <skelfact name=\"%\">\n" skel_name to:outFile_skeleton

		export_bones_transform obj root_bone "      " bones_vertices outFile_skeleton

		format "\n" to:outFile
		format "      <script name=\"main\">\n" to:outFile_skeleton
		format "        <loop/>\n" to:outFile_skeleton

		----------------------ANIMATION SCRIPT---------------------------
		format "\n" to:outFile

		key_frame_times = #()
		for t in objects do
		(
			if (object_is_bone t) then
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
		root_biped_transforms = #()
		root_dummy_transforms = #()
		initial_biped_transform = undefined
		initial_dummy_transform = undefined
		sort key_frame_times
		--slidertime = key_frame_times[2]
		for t in objects do
		(
			if (object_is_bone t) then
			(
				if (t.parent != undefined and t != root_bone) then
				(
					biped_rotation = (t.transform*(inverse t.parent.transform))as Quat
					biped_position = t.transform.pos *(inverse t.parent.transform)
					rotvalues = quattoeuler biped_rotation order:2
					base_rotx = rotvalues.x
					base_roty = rotvalues.y
					base_rotz = rotvalues.z
					
					append biped_transforms (biped_transform t.name t.transform)
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
					append biped_transforms (biped_transform t.name t.transform)

					if ((classOf t) == Dummy) then
					(
						initial_dummy_transform = t.transform
						append root_dummy_transforms (biped_transform t.name t.transform)
						child_obj = t.children[1]
						biped_rotation = child_obj.transform as Quat
						biped_position = child_obj.transform
						rotvalues = quattoeuler biped_rotation order:2
						base_rotx = rotvalues.x
						base_roty = rotvalues.y
						base_rotz = rotvalues.z
						initial_biped_transform = child_obj.transform
						append root_biped_transforms (biped_transform child_obj.name child_obj.transform)
					)
				)
			)
		)

		for kf=2 to key_frame_times.count do
		(
			slidertime = key_frame_times[kf]
			duration = 1
			if (kf > 1) then
			(
				--duration = ((key_frame_times[kf] as integer - key_frame_times[kf - 1] as integer))/4 as integer
				t1 = key_frame_times[kf] as string
				t1[t1.count] = ""
				t2 = key_frame_times[kf - 1] as string
				t2[t2.count] = ""
				duration = ((t1 as float) - (t2 as float))*40
			)
			biped_animation_found = false
			format "        <frame duration=\"%\">\n" (duration as integer) to:outFile_skeleton

			dummy_rotation = undefined
			dummy_position = undefined
			old_dummy_rotation = undefined
			old_dummy_position = undefined

			root_biped_rotation = undefined
			root_biped_position = undefined
			root_biped_transform = undefined
			old_root_biped_rotation = undefined
			old_root_biped_position = undefined
			for t in objects do
			(
				if (object_is_bone t) then
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

					if (t.parent != undefined and t != root_bone) then
					(
						biped_rotation = (t.transform*(inverse t.parent.transform)) as Quat
						biped_position = t.transform.pos *(inverse t.parent.transform)
						old_biped_rotation = (biped_transforms[bp_index].tr*(inverse t.parent.transform)) as Quat
						old_biped_position = biped_transforms[bp_index].tr.pos *(inverse t.parent.transform)
					)
					else
					(
						b_transform = t.transform
						biped_rotation = (b_transform) as Quat
						biped_position = b_transform.pos
						old_biped_rotation = biped_transforms[bp_index].tr as Quat
						old_biped_position = biped_transforms[bp_index].tr.pos
						if ((classOf t) == Dummy) then
						(
							dummy_rotation = (t.transform) as Quat
							dummy_position = t.transform.pos
							old_dummy_rotation = root_dummy_transforms[bp_index].tr as Quat
							old_dummy_position = root_dummy_transforms[bp_index].tr.pos

							root_biped_obj = t.children[1]
							
							root_biped_rotation = (root_biped_obj.transform) as Quat
							root_biped_position = root_biped_obj.transform.pos
							root_biped_transform = root_biped_obj.transform
							old_root_biped_rotation = root_biped_transforms[bp_index].tr as Quat
							old_root_biped_position = root_biped_transforms[bp_index].tr.pos
						)
					)

					write_x_rot = false
					write_y_rot = false
					write_z_rot = false

					epsilon = 0.0000001

					new_rotvalues = quattoeuler biped_rotation order:2
					diff_rotvalues = quattoeuler (biped_rotation*(inverse old_biped_rotation)) order:2

					if ((t.parent != undefined) and t != root_bone)then
					(
						--if ((write_x_rot or write_y_rot or write_z_rot) or (t.parent == undefined)) then
						(
							rotx = (new_rotvalues.x * pi)/180
							roty = -(new_rotvalues.y * pi)/180
							rotz = (new_rotvalues.z * pi)/180
							xMove = (biped_position.x * xscale) + xrelocate
							yMove = (biped_position.y * yscale) + yrelocate
							zMove = (biped_position.z * zscale) + zrelocate

							if (((classOf t.parent) == Dummy) and (not chkParentDummyMovable.checked)) then
							(
								pos = initial_biped_transform.pos *(inverse initial_dummy_transform)
								if (chkposx.checked == true) then
								(
									xMove = pos.x
								)

								if (chkposy.checked == true) then
								(
									yMove = pos.y
								)

								if (chkposx.checked == true) then
								(
									zMove = pos.z
								)

								initial_rotation = quattoeuler (initial_biped_transform as Quat) order:2

								if (chkrotx.checked == true) then
								(
									rotx = (initial_rotation.x * pi)/180
								)

								if (chkrotz.checked == true) then
								(
									roty = -(initial_rotation.y * pi)/180
								)

								if (chkroty.checked == true) then
								(
									rotz = (initial_rotation.z * pi)/180
								)

								
							)

							format "          <bone name=\"%\">" t.name to:outFile_skeleton
							format "<move>"  to:outFile_skeleton
							format "<v x=\"%\" y=\"%\" z=\"%\" />"  xMove zMove yMove to:outFile_skeleton
							format "<matrix>"  to:outFile_skeleton
							format "<rotx>%</rotx>" rotx to:outFile_skeleton
							format "<roty>%</roty>" roty to:outFile_skeleton
							format "<rotz>%</rotz>" rotz to:outFile_skeleton
							format "</matrix>"  to:outFile_skeleton
							format "</move>" to:outFile_skeleton
							format "</bone>\n" to:outFile_skeleton

						)
					)
					else
					(
						if (chkParentDummyMovable.checked == false ) then
						(
							diff_rotvalues = quattoeuler (root_biped_rotation*(inverse old_root_biped_rotation)) order:2
							rotx = 0
							roty = 0
							rotz = 0

							if (chkrotx.checked == true) then
							(
								rotx = (diff_rotvalues.x * pi)/180
							)

							if (chkrotz.checked == true) then
							(
								roty = -(diff_rotvalues.y * pi)/180
							)

							if (chkroty.checked == true) then
							(
								rotz = (diff_rotvalues.x * pi)/180
							)

							xMove = 0
							yMove = 0
							zMove = 0

							if (chkposx.checked == true) then
							(
								xMove = ((root_biped_position.x - old_root_biped_position.x) * xscale) + xrelocate
							)

							if (chkposy.checked == true) then
							(
								yMove = ((root_biped_position.y - old_root_biped_position.y) * yscale) + yrelocate
							)

							if (chkposz.checked == true) then
							(
								zMove = ((root_biped_position.z - old_root_biped_position.z) * zscale) + zrelocate
							)

							format "          <bone name=\"%\"><relative/>" t.name to:outFile_skeleton
							format "<move>"  to:outFile_skeleton
							format "<v x=\"%\" y=\"%\" z=\"%\" />"  xMove zMove yMove to:outFile_skeleton
							format "<matrix>"  to:outFile_skeleton
							format "<rotx>%</rotx>" rotx to:outFile_skeleton
							format "<roty>%</roty>" roty to:outFile_skeleton
							format "<rotz>%</rotz>" rotz to:outFile_skeleton
							format "</matrix>"  to:outFile_skeleton
							format "</move>" to:outFile_skeleton
							format "</bone>\n" to:outFile_skeleton

							root_biped_transforms[bp_index].tr = root_biped_transform
						)
						else
						(
							relative = false
							if (relative == true) then
							(
								rotx = (diff_rotvalues.x * pi)/180
								roty = -(diff_rotvalues.y * pi)/180
								rotz = (diff_rotvalues.z * pi)/180
								local xMove = ((biped_position.x - old_biped_position.x) * xscale) + xrelocate
								local yMove = ((biped_position.y - old_biped_position.y) * yscale) + yrelocate
								local zMove = ((biped_position.z - old_biped_position.z) * zscale) + zrelocate
								format "          <bone name=\"%\"><relative/>" t.name to:outFile_skeleton
							)
							else
							(
								rotx = (new_rotvalues.x * pi)/180
								roty = -(new_rotvalues.y * pi)/180
								rotz = (new_rotvalues.z * pi)/180
								local xMove = (biped_position.x * xscale) + xrelocate
								local yMove = (biped_position.y * yscale) + yrelocate
								local zMove = (biped_position.z * zscale) + zrelocate
								format "          <bone name=\"%\">" t.name to:outFile_skeleton
							)
							format "<move>"  to:outFile_skeleton
							format "<v x=\"%\" y=\"%\" z=\"%\" />"  xMove zMove yMove to:outFile_skeleton
							format "<matrix>"  to:outFile_skeleton
							format "<rotx>%</rotx>" rotx to:outFile_skeleton
							format "<roty>%</roty>" roty to:outFile_skeleton
							format "<rotz>%</rotz>" rotz to:outFile_skeleton
							format "</matrix>"  to:outFile_skeleton
							format "</move>" to:outFile_skeleton
							format "</bone>\n" to:outFile_skeleton
						)
					)
					biped_transforms[bp_index].tr = t.transform
				)
			)
			format "        </frame>\n\n" to:outFile_skeleton
		)

		format "\n" to:outFile_skeleton
		-----------------------------------------------------------------

		format "      </script>\n" to:outFile_skeleton
		format "    </skelfact\n" to:outFile_skeleton

-------------------------------------------------------------------------------
		close outFile 
		close outFile_skeleton
		message = "Done!"
		messageBox message
	
	)
)

gw = newRolloutFloater "Export GenMesh Animation 2" 300 270 
addRollout Test1 gw 
)