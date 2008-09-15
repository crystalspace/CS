#!python
from wixlib import *
from optparse import OptionParser

import time
import datetime
import uuid

from guidregistry import guid_registry as gr

options = OptionParser(usage="usage: %prog [options] list1 list2")
options.add_option("-o", "--out", dest="outfile",
                    help="Output file name", metavar="FILE")
options.add_option("--id", dest="msmid",
                    help="Merge module ID (used to look up GUID)", metavar="ID")
options.add_option("--subdirmap", dest="subdirmap",
                    help="Subdirectory map to place files in. (Multiple allowed)", 
                    action="append", metavar="LIST:DIR")
options.add_option("--version", dest="version",
                    help="Package version", metavar="VERSION")

(parsed_options, args) = options.parse_args()

if not parsed_options.outfile:
    options.error("missing --out")
if not parsed_options.msmid:
    options.error("missing --id")

subdirmap = {}
if parsed_options.subdirmap:
    for s in parsed_options.subdirmap:
        (list, subdirs) = s.split (':', 1)
        subdirmap[list] = subdirs

if gr.has_key(parsed_options.msmid):
    msm_guid = gr[parsed_options.msmid]
else:
    # WiX docs say it's an autogen guid, but using * gives error CNDL0009
    # when running candle
    msm_guid = str(uuid.uuid1())
     
(base, merge, dir) = generate_merge_module(id = parsed_options.msmid,
                                           path = [('TARGETDIR', 'SourceDir', None),
                                           (parsed_options.msmid + '_dir', None, None)],
                                           filename = parsed_options.outfile,
                                           manufacturer = "The Crystal Space Project",
                                           guid = msm_guid,
                                           version = parsed_options.version)
                                           
f = Fragment(
    parent = base,
    id = parsed_options.msmid)

dirref = DirectoryRef(
    parent = f,
    id = parsed_options.msmid + '_dir')

for list in args:
    listid = os.path.splitext(os.path.basename(list))[0]
    thisdir = dirref
    if subdirmap.has_key(listid):
        dirid = parsed_options.msmid
        list_subdir = subdirmap[listid]
        for subdir in list_subdir.split('/'):
            dirid = dirid + '.' + subdir
            thisdir = Directory(
                parent = thisdir,
                id = dirid + '_d',
                name = subdir)
    generate_from_file_list_txt (list,
                                 parent = merge,
                                 prefix = parsed_options.msmid + '.' + listid,
                                 dir = thisdir)
        
base.save()
