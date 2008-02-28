# +---------------------------------------------------------+
# | Copyright (c) 2005 caedes@sindominio.net                |
# | http://b2cs.delcorp.org                                 |
# | Created on May 01, 2006                                 |
# | Released under the General Public License (GPL)         |
# |  ++-+-++++++++++++++++++++-+-+---++-------+-+-+-+--+-+  |
# | creates the output for skeleton armatures                |
# +---------------------------------------------------------+

"""
        blender2crystal module
        type: armature
        name: skeleton
        description: Skeleton export
"""

import Blender
from Blender import Mathutils
from Blender.Armature import Bone
from b2cs.csxml import *
import math
import b2cs

global all_skeleton_factories
global created_skeleton_factories

def print_info():
	print "type skeleton2 plugin"
	
def start_export(global_keys):
	global all_skeleton_factories
	global created_skeleton_factories
	all_skeleton_factories = {}
	created_skeleton_factories = []
	
	# get the matrix for a bone
def get_bone_matrix(bone,armature):
	rot = b2cs.util.getPropertyRotationMatrix(armature)
	if not bone.hasParent():
		if rot:
			bone_matrix = bone.matrix["ARMATURESPACE"]*rot
		else:
			bone_matrix = bone.matrix["ARMATURESPACE"]
	else:
		parent_imatrix = Mathutils.Matrix(bone.parent.matrix["ARMATURESPACE"]).invert()
		bone_matrix = bone.matrix["ARMATURESPACE"]*parent_imatrix
	return (bone_matrix.translationPart(),bone_matrix.rotationPart())
	
	# get the matrix for a bone
def get_bone_posematrix(bone,armature_obj):
	pose = armature_obj.getPose()
	pos_b = pose.bones[bone.name]
	mat_b = pos_b.poseMatrix
	return mat_b

def get_bone_cmatrix(bone,armature_obj):
	pose = armature_obj.getPose()
	mat_b = get_bone_posematrix(bone,armature_obj)
	rot = b2cs.util.getPropertyRotationMatrix(armature_obj)
	if not bone.hasParent():
		if rot:
			bone_matrix = mat_b*rot
		else:
			bone_matrix = mat_b
	else:
		pare = bone.parent
		pos_p = pose.bones[pare.name]
		mat_p = pos_p.poseMatrix
		mat_c = Mathutils.Matrix(mat_p)
		mat_c.invert()
		bone_matrix = mat_b * mat_c
	return (bone_matrix.translationPart(),bone_matrix.rotationPart())

def get_socket_matrix(bone,armature,socket_obj):
	bone_matrix = get_bone_posematrix(bone,armature)
	bone_imatrix = Mathutils.Matrix(bone_matrix).invert()
	skel_imatrix = Mathutils.Matrix(armature.getMatrix()).invert()
	obj_matrix = socket_obj.getMatrix()*skel_imatrix*bone_imatrix
	return (obj_matrix.translationPart(),obj_matrix.rotationPart())
	
def export_socket(file,armature,bone,object_prop,socket_obj):
	file.write('<socket name="%s">\n'%(socket_obj.getName()))
	size = armature.getSize()
	(pos,rotmat) = get_socket_matrix(bone,armature,socket_obj)
	euler=rotmat.toEuler()
	c = math.pi/180.0
	rot = [euler[0]*c,euler[1]*c,euler[2]*c]
	file.write('  <move>\n')
	file.write('  <v x="%s" y="%s" z="%s" />\n' % (size[0]*pos[0],size[2]*pos[2],size[1]*pos[1]))
	file.write('  <matrix>\n')
	if rot[0]:
		file.write('  <rotx>%s</rotx>\n'%(rot[0]))
	if rot[1]:
		file.write('  <rotz>%s</rotz>\n'%(rot[1]))
	if rot[2]:
		file.write('  <roty>%s</roty>\n'%(-rot[2]))
	file.write('  </matrix>\n')
	file.write('  </move>\n')

	file.write('</socket>\n')
def export_bone(file,armature,bone,object_prop,bonelist,socketlist):
        # for the moment export all bones
        #if bone.getBoneclass()==Bone.UNSKINNABLE:
        #	return
        #if bone.hasIK()
	bonelist.append(bone.name)
	file.write('<bone name="'+bone.name+'">\n')
	#!!!("+bone.getBoneclass()+")---"
	# get matrix values
	(pos,rotmat)=get_bone_matrix(bone,armature)
	euler=rotmat.toEuler()
	quat=rotmat.toQuat()
	#print "quaternion",quat,quat.w
	from cspace import csQuaternion,csVector3
	from b2cs.b2cstool import *
	csmat = bmat2cs(rotmat)
	#print csmat
	csquat = csQuaternion()
	c = math.pi/180.0
	rot = [euler[0]*c,euler[1]*c,euler[2]*c]
	csquat.SetEulerAngles(csVector3(rot[0],-rot[2],rot[1]))
	#print "csquaternion from euler",csquat,csquat.w
	csquat.SetMatrix(csmat)
	#print "csquaternion from matrix",csquat,csquat.w
	#pos =bone_matrix.translationPart()
	# XXX should this be used?
	# bone.getWeight()
	# write bone transform
	size=armature.getSize()
	#file.write('<move>\n')
	file.write('<position x="%s" y="%s" z="%s" />\n' % (size[0]*pos[0],size[2]*pos[2],size[1]*pos[1]))
	file.write('<rotation x="%s" y="%s" z="%s" w="%s" />\n' % (quat.x,quat.z,quat.y,quat.w))
	#file.write('</matrix>\n')
	#file.write('</move>\n')
	# XXX skinbox
	# XXX ragdoll <body,geom,joint>
	# export children bones
	if bone.name in socketlist.keys():
		for socket_obj in socketlist[bone.name]:
			print " * socket:",socket_obj.getName()
			export_socket(file,armature,bone,object_prop,socket_obj)
	if bone.children:
		for a in bone.children:
			if a.parent.name == bone.name:
				export_bone(file,armature,a,object_prop,bonelist,socketlist)
				# export vertex influences
				#export_influences(file,mesh,bone.name,object_prop)
				# end bone node
	file.write('</bone>\n')
	object_prop["exported_bones"].append(bone.name)
	
def get_bone(bonename,armature):
	bones = armature.bones
	for bbonename in bones.keys():
		bone = bones[bbonename]
		if str(bone.name)==str(bonename):
			return bone
	return None
	
def parse_properties(object,object_prop,global_keys):
	b2cs.util.doMapPropertiesDict(object,b2cs.prop.allproperties["skeleton"],object_prop)
	object_prop["factory"]="skel"+object.getData().name
	
def export_armature(file,armature_obj,object_prop,global_keys):
	if "skeldict" not in global_keys.keys():
		global_keys["skeldict"]={}
	if "skelanim" not in global_keys.keys():
		global_keys["skelanim"]={}
	armature=armature_obj.getData()
	# save and unset rest position, otherwise poses cant be exported.
	restpos = armature.restPosition
	armature.restPosition = False
	
	# start export
	skelname = armature.name
	global_keys["skeldict"][skelname]=[]
	global_keys["skelanim"][skelname]=[]
	bonelist = global_keys["skeldict"][skelname]
	file.write('<skeletonfactory name="%s">\n'%(skelname))
	bones = armature.bones
	object_prop["exported_bones"]=[]
	initframe = Blender.Get('curframe')
	# find sockets
	socketlist = {}
	armature_childs = b2cs.util.doGetChildren(armature_obj)
	for child in armature_childs:
		if b2cs.util.doGetType(child) == "socket":
			parentname = child.getParentBoneName()
			if parentname:
				if parentname not in socketlist.keys():
					socketlist[parentname] = []
				socketlist[parentname].append(child)
	file.write("<bone name='StaticRootBone' numbones='%d'>\n"%(len(bones.keys())+1))
	# bone structure
	for bonename in bones.keys():
		bone = bones[bonename]
		if not bone.hasParent():
			export_bone(file,armature_obj,bone,object_prop,bonelist,socketlist)
	file.write("</bone>")
	# actions
	actions= Blender.Armature.NLA.GetActions()
	initialaction = armature_obj.getAction()
	firstaction = None
	for action in actions:
		cyclic_action = False
		if "actions" in object_prop.keys():
			if not action in object_prop["actions"]:
				continue
		# parallel
		print "action:", action
		actions[action].setActive(armature_obj)
		# normal
		iposdict = actions[action].getAllChannelIpos()
		# get keyframes
		frames={}
		for bonename in iposdict:
			if bonename in object_prop["exported_bones"] and iposdict[bonename]:
				ipo=iposdict[bonename]
				curves=ipo.getCurves()
				for curve in curves:
					if (curve.getExtrapolation() == "Cyclic"):
						cyclic_action=True
					points= curve.getPoints()
					for p in points:
						xy=p.getPoints()
						if xy[0] in frames.keys() and bonename not in frames[xy[0]]:
							frames[xy[0]].append(bonename)
						elif xy[0] not in frames.keys():
							frames[xy[0]]=[bonename]
							# no frames? then this is not for this skeleton
		if len(frames) == 0:
			continue
			# set the first action as default action
			#if firstaction==None and object_prop["activatefirst"]:
			#	firstaction=action
		global_keys["skelanim"][skelname].append(action)
		file.write('  <animation name="'+action+'">\n')
		#if cyclic_action:
		#	file.write('  <loop/>\n')
			# export animation frames
		allkeys=frames.keys()
		keytime=None
		# find smallest keytime
		for key in allkeys:
			if not keytime or key < keytime:
				keytime = key
		allkeys.sort()
		rotnames=["QuatW","QuatX","QuatY","QuatZ"]
		locnames=["LocX","LocY","LocZ"]
		for bonename in iposdict:
			if bonename == "Object":
				continue
			bone = get_bone(bonename,armature)
			if not bone:
				continue
			file.write('    <channel name="'+bonename+'">\n')
			for key in allkeys:
				Blender.Set('curframe',int(key))
				time=key-keytime
				fps=24.0
				keytime=key
				cstime = ((key - allkeys[0])/fps)*1000.0
				file.write('      <frame time="'+str(int(cstime))+'">\n')
				if bonename == "Object":
					continue
				bone = get_bone(bonename,armature)
				if not bone:
					continue
				export_rot=1
				export_loc=1
				if export_rot:
				        # now find the real rotation
					(pos,matrot)=get_bone_cmatrix(bone,armature_obj)
					quat=matrot.toQuat()
					#euler=matrot.toEuler()
					#c = math.pi/180.0
					#disp = [euler[0]*c,euler[1]*c,euler[2]*c]
				# export anims if any
				size=armature_obj.getSize()
				if export_rot or export_loc:
					file.write('        <position x="%s" y="%s" z="%s"/>\n'%(size[0]*pos[0],size[2]*pos[2],size[1]*pos[1]))
					file.write('        <rotation x="%s" y="%s" z="%s" w="%s" />\n' % (quat.x,quat.z,quat.y,quat.w))

				file.write('      </frame>\n')
			
			# end frame
			file.write('    </channel>\n')
			# end script
		file.write('  </animation>\n')
	if initialaction:
		initialaction.setActive(armature_obj)
	Blender.Set('curframe',initframe)
	file.write('</skeletonfactory>\n')
	armature.restPosition = restpos
	
def export(object,object_prop,global_keys):
	global created_skeleton_factories
	if object_prop["factory"] not in created_skeleton_factories:
		fact_file=file(os.path.join(global_keys["temp_dir"],"factories",object_prop["factory"]),"w")
		export_armature(fact_file,object,object_prop,global_keys)
		fact_file.close()
		if not global_keys["direxport"]:
			b2cs.csxml.zip_file(os.path.join(global_keys["temp_dir"],"factories",object_prop["factory"]),"factories",global_keys["autodestroyfactories"])
		created_skeleton_factories.append(object_prop["factory"])
		
