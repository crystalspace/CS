#!/usr/bin/env python

import types, string, re

try:
	import cPickle
	pickle=cPickle
except:
	print "WARNING: Failed to import module cPickle, using slower pickle library"
	import pickle

try:
	from cspace import *
except:
    	print "WARNING: Failed to import module cspace, can only pickle the map"

def CreateRoom(matname):
	print 'Start creating polygons...'
	global system
	system=GetSystem()
	world=system.Query_iWorld()
	room=world.FindSector("room")
	polyset=room.Query_iPolygonSet()
	material=world.FindMaterial(matname)

	poly=polyset.CreatePolygon('floor')
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=polyset.CreatePolygon('ceiling')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=polyset.CreatePolygon('w1')
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=polyset.CreatePolygon('w2')
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=polyset.CreatePolygon('w3')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=polyset.CreatePolygon('w4')
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)
	print 'Finished!'

