#file:include_exclude.py
"""
purpose: to blablabla




"""

# -----------------------------------------
# Getting the header file list.



class IncludeExclude:
    """ reads a config file of header files which to include.
        
	TODO: There will probably be needed more config info, such as
	info about what to do to .h files.
    """

    def __init__(self, include_file_names, exclude_file_names):
        """ include_file_names - list of file names of header files to include.
	    exclude_file_names - list of file names of header files to exclude.

	    first line - base directory for the headers, relative to 
	                 CS/scripts/swig   eg ../../include/ for CS includes.
	    second line - base output directory.

	    All lines after the first will be header files relative to that 
	     base directory.
	"""

        self.includes_dict = {}
        self.excludes_dict = {}
	self.include_file_names = include_file_names
	self.exclude_file_names = exclude_file_names

    def GetFileNames(self, sortable = 0):
        """ Returns a dict of files to be parsed, and used in swig.
	     keyed by (base in directory, base out directory), valued
	      by a list of header files to process.

	    sortable - if set to 1, it will return a dictionary keyed by:
	      (<sort number>, (base in directory, base out directory))
	      valued by a list of header files sorted in order.

	    DONE_SORTABLE - not tested.
	"""

	# read in all the files.

        self._ReadAllXXcludes(self.includes_dict, self.include_file_names, sortable)
        self._ReadAllXXcludes(self.excludes_dict, self.exclude_file_names, sortable)

        # For each base (in, out) pair remove the excludes from the includes.
        return self._RidExcludesFromIncludes(self.includes_dict, 
	                                     self.excludes_dict, sortable)





    def _ReadAllXXcludes(self, xxclude_dict, file_names, sortable = 0):
        """ reads in the file names into xxclude_dict keyed by
	    (base in directory, base out directory)
	    valued by a list of headers.
	    file_names - to read the xxcludes from.

	    sortable - if 1, then the xxclude dict will be returned keyed by:
	      (<sort number>, (base in directory, base out directory))
	      and valued by a list of headers in order, that they are in file.


	    NOTE: xxcludes is short for includes, or excludes.

	    DONE_SORTABLE - not tested.
	"""
        sortable_counter = 0

	for x in file_names:
	    new_cludes = self._ReadXXcludes(x)

	    if sortable:
	        s_key = (sortable_counter, new_cludes[0])
	    else:
	        s_key = new_cludes[0]

	    if xxclude_dict.has_key( s_key ):
	        xxclude_dict[s_key] = (xxclude_dict[new_cludes[0]] + new_cludes[1] )
	    else:
	        xxclude_dict[s_key] = new_cludes[1]

            sortable_counter = sortable_counter + 1



    def _ReadXXcludes(self, file_name ):
        """ Reads in a file list of header files..
	    Returns ((base in directory, base out directory), [header names...])
	     header names are relative to the base directory.

	    DONE_SORTABLE - not tested.
	"""

	xxcludes= open(file_name).readlines()
        xxcludes = rid_new_lines(xxcludes)

	# get the base in directory.
	base_in = xxcludes[0]

	# get the base out directory.
	base_out = xxcludes[1]

        return ( (base_in, base_out),xxcludes[2:] )



    def _RidExcludesFromIncludes(self, includes, excludes, sortable = 0):
        """ Gets rid of the excludes from the includes.
	    includes, excludes - dicts keyed by base (in,out) directories, 
	                         valued by list of headers.

            sortable - if 1, includes and excludes keys have (<sorted_number>(base in,/out)).

	    TODO: fix this func.

	    FIXME: not sure if the sorted numbers in the keys to the "includes", and "excludes"
	           are going to match up.
		   If not then we need to remove them for the part where we compare the two.

		   I'm not sure it makes sense that we have a sorted order for includes and 
		   excludes any way.  just for includes would be good I think.
	"""

        #go through the base in, out dirs, and make dicts from the list,
	# keyed by header.

	for s_key in includes.keys():
	    # for each header in the excludes, remove it from the excludes.
            if sortable:
                sort_num, base_in, base_bout = s_key
	    else:
                base_in, base_bout = s_key


	    if excludes.has_key( s_key ):

		inc_headers = {}

		# used to map a header with sorted number, to the one without sorted number.
		sortable_map = {}

                s_counter = 0
	        for header in includes[s_key]:
		    if sortable:
			sortable_map[header] = (s_counter, header)
	            inc_headers[header] = None

		    s_counter = s_counter + 1
		
	        for header in excludes[s_key]:
		    if inc_headers.has_key(header):
		        del inc_headers[header]

	        # reverse the map, if we are in sortable mode.
		if sortable:
		    sort_list = []

		    for header in inc_headers.keys():
		        sort_list.append( sortable_map[header] )

		    sort_list.sort()
		sorted_list = []

		# remove the numbers at the front.
		for x in sort_list:
		    sorted_list.append( x[1] )


		includes[s_key] = sorted_list

        return includes






class TestIncludeExclude(unittest.TestCase):
    def testOneSet(self):
        """one include file and one exclude file."""
	inc_exc = fixheaders.IncludeExclude(["testfiles/includes.txt"], 
	                                    ["testfiles/excludes.txt"])
	file_names = inc_exc.GetFileNames()
	for x in file_names.keys(): file_names[x].sort()

	should_be={ ("testfiles/include/",
	             "testfiles/output/"):["csutil/csevcord.h",
                                           "csutil/csevent.h",
                                           "csutil/csinput.h"] }

	for x in should_be.keys(): should_be[x].sort()

        self.assertEqual(should_be, file_names)

    def testTwoSetsDifferentDirs(self):
        """ sorts two sets of includes/excludes. with different input/output
	      directories.
	"""

	inc_exc = fixheaders.IncludeExclude(["testfiles/includes.txt",
	                                     "testfiles/includes2.txt"], 
	                                    ["testfiles/excludes.txt",
	                                     "testfiles/excludes2.txt"])

	file_names = inc_exc.GetFileNames()
	for x in file_names.keys(): file_names[x].sort()

	should_be={ ("testfiles/include/",
	             "testfiles/output/"):["csutil/csevcord.h",
                                           "csutil/csevent.h",
                                           "csutil/csinput.h"],
	            ("testfiles/include2/",
	             "testfiles/output2/"):["csutil/csevcord.h",
                                           "csutil/csevent.h",
                                           "csutil/csinput.h"], }

	for x in should_be.keys(): should_be[x].sort()

        self.assertEqual(should_be, file_names)




