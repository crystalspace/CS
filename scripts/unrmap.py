#!/usr/bin/env python

import types
from string import *;

try:
	import cPickle
	pickle=cPickle
except:
	print "WARNING: Failed to import module cPickle, using slower pickle library"
	import pickle

try:
	import cspacec
	from cspace import *
except:
    	print "WARNING: Failed to import module cspace, can only pickle the map"
	class Vector3:
		def __init__(self, x, y, z):
			print x, y, z
	
class Actor:
	"""An actor object from Unreal"""
	def __init__(self, attrib):
	        self.attrib=attrib
	        self.stack=[]
	def Add(self, attrib):
		pass
	def Push(self, child):
		self.stack.append(child)
	def Load(self, room, tm):
		for x in self.stack:
			x.Load(room, tm)

def ListToVector3(list):
	for x in list:
		csVector3(x[0], x[1], x[2])

class Polygon(Actor):
    	"""A polygons"""
	def __init__(self, attrib):
	        self.attrib=attrib
	        self.veclist=[]
	def AddPoint(self, vec):
		self.veclist.append(float(vec[0]), float(vec[1]), float(vec[2]))
	def Load(self, room, tm):
		poly=room.NewPolygon(tm)
		for x in self.veclist:
			poly.AddVertex(x[0], x[1], x[2])
		poly.SetTextureSpace(poly.Vobj(0), poly.Vobj(1), 3);
		
class PolyList(Actor):
    	"""A list of polygons"""
	pass
		
class Brush(Actor):
    	"""A geometry object from Unreal"""
	pass
	
class Map(Actor):
	"""A pickleable map"""
	def Load(self):
#		loader=csLoader()
		system=GetSystem()
#		world=system.Query_csWorld()
#		room=world.NewSector()
		room=csSector(roomptr);
#		tm=loader.LoadTexture(world, 'stone', '/lib/std/stone4.gif')
		tm=csTextureHandle(tmptr)
		for x in self.stack:
			x.Load(room, tm)
		g3d=system.Query_iGraphics3D()
#		view=csView(world, g3d)
#		view.SetSector(room)
#		view.GetCamera().SetPosition(csVector3(0,5,0))
#		view.SetRectangle(2, 2, GetFrameWidth()-4, GetFrameHeight()-4)
#		world.view=view

class UnrealMap:
	"""Import an Unreal Map and Load it into Engine, or pickle it"""
	def Load(self, file):
		if(type(file)==types.StringType):
			file=open(file, 'r')
		self.stack=[]
		self.file=file;
		self.Loop()
		self.map=self.block
	def Save(self, file):
		if(type(file)==types.StringType):
			file=open(file, 'w+')
		pickle.dump(self.map, file)
	def Parse(self):
		self.NextLine()
		if(not len(self.line)):
			return 0
		self.words=split(self.line)
		if(self.words[0]=='Begin'):
			self.Push()
			self.Begin()
			return 1
		if(self.words[0]=='End'):
			self.Pop()
			return 0
		self.block.Add(self.Params(0))
		return 1
	def NextLine(self):
		self.line=self.file.readline()
	def Loop(self):
		while(self.Parse()):
			pass
	def Begin(self):
		if(self.words[1]=='Map'):
			attrib=self.Params(2)
			self.block=Map(attrib)
			print "Found Map", attrib
			self.Loop()
			return
		if(self.words[1]=='Actor'):
			self.Actor()
			return
		if(self.words[1]=='Brush'):
			self.block=Brush(self.Params(2))
			self.Loop()
			return
		if(self.words[1]=='PolyList'):
			self.block=PolyList(self.Params(2))
			self.Loop()
			return
		if(self.words[1]=='Polygon'):
			self.Polygon()
			return
		print "WARNING: Ignoring Unknown: ", self.words[1]
		self.block=None
		self.Loop()
	def Polygon(self):
		self.block=Polygon(self.Params(2))
		for i in range(1,4):
			self.NextLine();
		for i in range(1,4):
			self.PolyAdd()
		self.Loop()
	def PolyAdd(self):
		self.NextLine();
		self.words=split(self.line)
		vecs=split(self.words[1], ',')
		self.block.AddPoint(vecs)
	def Actor(self):
		attrib=self.Params(2)
		self.block=Actor(attrib)
		self.Loop()
	def Pop(self):
		if(len(self.stack)):
#			print "Pop: ", self.stack[0], self.block
			self.stack[0].Push(self.block)
			self.block=self.stack[0]
			del self.stack[0]
	def Push(self):
		if(hasattr(self, 'block')):
			self.stack.insert(0, self.block)
#			print "Push: ", self.block
	def Params(self, index):
		params={}
		for x in self.words[index:]:
			word=split(x, '=')
			if(len(word)==2):
				params[word[0]]=word[1]
		return params

def Load(file):
    	try:
    		print 'Unrmap: Opening '+file+'.pym'
		a=pickle.load(open(file+'.pym','r'))
	except:
		print 'Unrmap: file not openable, attempting conversion of '+file+'.t3d'
	 	m=UnrealMap()
		m.Load(file+'.t3d')
		m.Save(file+'.pym')
		a=m.map
	print 'Unrmap: Map loaded, creating...'
	a.Load()
				
#Load('entry')
