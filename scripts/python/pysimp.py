#!/usr/bin/env python

import types, string, re

try:
	from cspace import *
except:
    	print "WARNING: Failed to import module cspace"

def CreateRoom(matname):
	print 'Start creating polygons...'
	global system
	system = GetSystem()
	engine = system.Query_iEngine()
	room = engine.FindSector("room")
	walls = engine.CreateSectorWallsMesh(room,"walls")
	thingstate = walls.GetMeshObject().Query_iThingState()
	material=engine.FindMaterial(matname)

	poly=thingstate.CreatePolygon('floor')
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('ceiling')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w1')
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w2')
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w3')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w4')
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)
	thingstate.DecRef()
	print 'Finished!'

