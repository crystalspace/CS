#!python
from wixlib import *
from optparse import OptionParser

import time
import datetime

from guidregistry import guid_registry as gr

options = OptionParser(usage="usage: %prog [options] list1 list2")
options.add_option("-o", "--out", dest="outfile",
                    help="Output file name", metavar="FILE")
options.add_option("--id", dest="msmid",
                    help="Merge module ID (used to look up GUID)", metavar="ID")
options.add_option("--version", dest="version",
                    help="Package version", metavar="VERSION")

(parsed_options, args) = options.parse_args()

if not parsed_options.outfile:
    options.error("missing --out")
if not parsed_options.msmid:
    options.error("missing --id")

(base, merge, dir) = generate_merge_module(id = parsed_options.msmid,
                                           path = [('TARGETDIR', 'SourceDir', None),
                                           (parsed_options.msmid + '_dir', None, None)],
                                           filename = parsed_options.outfile,
                                           manufacturer = "The Crystal Space Project",
                                           guid = gr[parsed_options.msmid],
                                           version = parsed_options.version)
                                           
f = Fragment(
    parent = base,
    id = parsed_options.msmid)

ref = DirectoryRef(
    parent = f,
    id = parsed_options.msmid + '_dir')

for list in args:
    generate_from_file_list_txt (list,
                                 parent = merge,
                                 prefix = parsed_options.msmid,
                                 dir = ref)
        
base.save()
