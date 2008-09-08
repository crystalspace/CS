#!/usr/bin/env python
from optparse import OptionParser
from xml.etree.ElementTree import *

import sys

options = OptionParser(usage="usage: %prog [options] csplugin-file csplugin-file")
options.add_option("-o", "--out", dest="outfile",
                    help="Output file name", metavar="FILE")

(parsed_options, args) = options.parse_args()

cspluginNode = Element("plugin")
cspluginTree = ElementTree (cspluginNode)

outClasses = None

for cspluginFile in args:
	xml = file(cspluginFile, "r")
	inTree = ElementTree (None, xml)
	inRoot = inTree.getroot()
	if inRoot.tag != "plugin":
		print "Root element for %s is not <plugin>" % cspluginFile
		continue
	scfNode = inRoot.find("scf")
	if scfNode:
		classesNode = scfNode.find("classes")
		if classesNode:
			if not outClasses:
				outSCF = SubElement (cspluginNode, "scf")
				outClasses = SubElement (outSCF, "classes")
			for children in classesNode:
				outClasses.append(children)
				
if parsed_options.outfile:
	out = file (parsed_options.outfile, "w")
else:
	out = sys.stdout
cspluginTree.write(out, "utf-8")
