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

def TestMe(file):
	print 'Hello World'
	global system, scf
	system=GetSystem()
	scf=system.GetSCF()
	world=system.Query_iWorld()
	room=world.FindSector("room")
	poly=room.CreatePolygon('floor')
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.Vobj(0), poly.Vobj(1), 3)

