#file: fixheaders.py
#purpose: to make the header files from cs header files so that swig can handle
#  them better.

"""

Three main parts to this module.

  1. getting a list of the header files to include/exclude, and in what order.
    - there will be a config file, to determine which files to read for
      include/exclude so that other projects can add their own stuff.
  2. parsing the header files, and writing the new ones.
  3. Creating the cs.i file, with all the includes.
      - the #include section needs to be made up, so that all headers used 
        are in there.
        - Can we use some sort of %include to include the headers??  can't 
	  use %include...

"""

import string

from string import replace
from string import join
from string import split

import sys
import os


from UserList import UserList


class HeaderList(UserList):
    """ A list representing the header file names, and the base in and base out directories.
    """
    def __init__(self, include_file_name):
        """
	"""
	UserList.__init__(self)

        lines = rid_new_lines( open(include_file_name).readlines() )

	self.base_in = lines[0]
	self.base_out = lines[1]
	self.extend( lines[2:] )

    



def get_include_file_names(file_list):
    """ Returns a list of include_lists to be parsed, and used in swig.

        These include_lists contain the file names of the include files.
	The include_lists also have methods to GetBaseIn, and GetBaseOut.
        

        file_list - list of file names of header files to include.

	The files in file_list are formated like so:
	  <base in>\n
	  <base out>\n
	  <include file name, relative to the base in directory.>
	  <...>
	  <last include file name>
	    base in - base directory for the headers, relative to 
	                 CS/scripts/swig   eg ../../include/ for CS includes.
	    base out - base output directory for processed files.
    """

    # open each of the files in file_list, and make an HeaderList out of each one.
    return map(HeaderList, file_list)




# -----------------------------------------------
# filtering the header files.

import re
"""
NOTES:

Here will be a bunch of filters which will be used to make header files more
  friendly to swig.

Using lists of lines as it makes it more efficient to run multiple filters.  Make the 
  filters be in place for speed.
"""


def expand_scf_version(header_file_line_list, other_info = []):
    """ this filter expands the SCF_VERSION macro.
        header_file_line_list - list of lines for a header file.  
	                          No new lines please.

	  SCF_VERSION(Name,Major,Minor,Micro)				
             const int VERSION_##Name = SCF_CONSTRUCT_VERSION (Major, Minor, Micro)
          SCF_CONSTRUCT_VERSION(Major,Minor,Micro)			\
            ((Major << 24) | (Minor << 16) | Micro)

	  SCF_VERSION(iKeyboardDriver, 0,0,1)

	  TODO: make this filter return the VERSION_<name> parts.
	    So this can be used to make the object registry 
    """
    if header_file_line_list[0][:-1] == "\n":
        raise "I said no new lines!"

    data_matcher = re.compile(r"""(.*)SCF_VERSION\((.*),(.*),(.*),(.*)\)(.*)""")

    lines = []

    for line in header_file_line_list:

        data= data_matcher.search(line, 0)
        if(data):
	    start_line, name, Major, Minor, Micro, end_line = data.groups()
	    ver_num = ((int(Major) << 24) | (int(Minor) << 16) | int(Micro))

	    lines.append( "%sconst int VERSION_%s = %s%s" %( start_line, 
	                                                     name, ver_num, end_line) )
	else:
	    lines.append(line)

	    
    return lines






# --------------------------------------------
#  Writing the header files.


def make_directories_for_headers(base_dir, header_list):
    """Function takes a list of relative path header file names, and makes 
       directories for them.

       base_dir - directory to make the header files under.
       header_list - header file names using '/' seperator.
    """

    # break the header file names up into a dict keyed by a list of directories.
    header_dict = {}

    for x in header_list:
        split_list = string.split(x, "/")

	# get rid of the file name.
	directories = split_list[:-1]

	# join the directories together again to make deeper directories.
	header_dict[string.join(directories, "/")] = None


    directories = header_dict.keys()

    # Make the directories.

    def make_sub_dirs(base_dir, list_of_dirs):
        """ creates directories from a list of directories.
	     eg to make "/usr/local/bin/", "/usr/local", and "/usr"
	        base_dir = "/"
	        list_of_dirs = ["usr", "local", "bin"]
	"""
	# used to build up a list of lower level dirs.
	deeper_dirs = [base_dir]

	for current_dir in list_of_dirs:
	    the_dir = apply(os.path.join, deeper_dirs + [current_dir])

	    if not os.path.isdir( the_dir ):
	        os.system("mkdir %s" % the_dir)

	    # add directory so that the next one will be made in current_dir.
	    deeper_dirs.append(current_dir)



    for a_dir in directories:
        dirs = string.split(a_dir, "/")
	# see if directories are there, if not make them.

        make_sub_dirs(base_dir, dirs)




class FilterConfig:
    """ For getting filter function options for header files.

        TODO: this class needs to be re written to actually read in which 
	      filters are to be used.

    """

    def __init__(self, includes_lists,
                       default_function_names = [], 
                       excluded_functions = {}, 
		       included_functions = {},
		       filter_functions = []):
        """  TODO: a config for the filter functions.
	     includes_lists- a list of HeaderLists containing header files.

	     default_function_names - used on all headers.
	     excluded_functions - keyed by header, valued function_name.
	     included_functions - keyed by header, valued by function_name.
	     filter_functions - a list of functions, which takes a list of lines,
	                         and returns a processed list of lines.  They may
				 use default arguments to return other info.
        """
	self.includes_lists = includes_lists

	self.default_function_names = default_function_names
	self.included_functions = included_functions
	self.filter_functions = filter_functions
	self.excluded_functions = excluded_functions



    def MakeFilterDict(self):
        """  FIXME: see todo in init.
             For now it just returns all headers with the one function.
             Returns filter_dict[base directory(in,out)]  = header_dict
	                                                    header_dict[header] = function list.

        """
	r = {}

	for include_list in self.includes_lists:
	    base_in_out = (include_list.base_in, include_list.base_out)

	    d = {}

	    for header in include_list:
	        d[header] = [expand_scf_version]

	    r[base_in_out] = d

	    
	return r




def create_iObjectRegistry(scf_interfaces):
    """ Returns a string containing an iObjectRegistry object for swig 
        which contains a heap of Query_<SCF interface> methods.

	NOTE: A python object which would have a generic query object, 
	      which calls one of the other methods.

        scf_interfaces - a list of scf_interfaces, 
    """

    """ Example:

	//****** System Interface
	struct iObjectRegistry:public iBase
	{
	public:
	  %addmethods
	  {
	    iEngine* Query_iEngine()
	    {
	      iEngine* en = CS_QUERY_REGISTRY (self, iEngine);
	      en->DecRef ();
	      return en;
	    }
	    void Print(int mode, const char* format) {
	      printf (format);
	    }
	  }
	};
    """

    def create_method(interface_name):
        """ returns a method in a string for querying the iObjectRegistry"""
	method_list = []
	mapp = method_list.append
        i = interface_name
	mapp( """    %s* Query_%s()""" % (i, i) )
	mapp( """    {""" )
	mapp( """      %s* rval_%s = CS_QUERY_REGISTRY (self, %s);""" % (i, i, i) )
	mapp( """      rval_%s->DecRef ();""" % (i) )
	mapp( """      return rval_%s;""" % (i) )
	mapp( """    }""" )

	return string.join(method_list, "\n")

    # Create the top bit, then all the methods from the scf interfaces.
    # Finally create the bottom part.
    
    parts = []

    parts.append("""\
//****** System Interface
struct iObjectRegistry:public iBase
{
public:
  %addmethods
  {""")

    for scf_interface in scf_interfaces:
        parts.append( create_method(scf_interface) )


    parts.append("""\

    void Print(int mode, const char* format) {
      printf (format);
    }
  }
};""")

    return string.join(parts, "\n")



class HeaderProcessor:
    

    def __init__(self, header_lists, filter_dict):

        self.header_lists = header_lists
	self.filter_dict = filter_dict

        # place for filters to place info.
        # keyed by header name, valued by dict returned from filter for that header.
        self.extra_info = {}


    def _ReadProcessFile(self, base_dir, header_file_name, filters):
        """ base_dir - directory to look in for headers.
	    header_file_name - path to header.
            filters - functions which take a list of lines, and return a list of 
              processed lines.  They are applied in the order they are given.
		These functions must also take a list which it can use to place other info 
		 in if they wish.
	      
	    Returns [processed file as string], {<function name>:<other values returned>}]
	"""

        full_header_file_name = os.path.join(base_dir, header_file_name)

	# have to have no new line characters.
	lines = open(full_header_file_name).readlines()


        new_lines = rid_new_lines(lines)


	extra_info = {}

	for filter in filters:
	    extra_info[filter.__name__] = []
	    new_lines = filter(new_lines, extra_info[filter.__name__])

        self.extra_info[header_file_name] = extra_info

	return string.join(new_lines, "\n")





    def WriteHeaderFiles(self, filter_dict):
	""" filter_dict[base directory(in,out)]  = header_dict
	                                           header_dict[header] = function list.
	"""

	# Make the directories for the headers.
	for base_in_dir, base_out_dir in filter_dict.keys():

            make_directories_for_headers(base_out_dir, filter_dict[(base_in_dir, base_out_dir)].keys())

	    print "looping"
	    for header_file_name in filter_dict[(base_in_dir, base_out_dir)].keys():

	        processed_string = self._ReadProcessFile(base_in_dir,
	                            header_file_name, 
				    filter_dict[(base_in_dir, base_out_dir)][header_file_name] )
                print "writing file"

	        f = open(os.path.join(base_out_dir, header_file_name), "w")
		f.write(processed_string)



    def Write_generatd_i(self, generatd_i_location = "generatd.i"):
        """ This writes all of the header files in an %include like fashion.
	"""

	file_contents = self.Generate_generatd_i()
	open(generatd_i_location, "w").write(file_contents)


    def Write_cs_i(self, cs_i_location ='../../include/ivaria/cs.i'):
        """  Writes the new cs.i file.
	"""

	processed_string = self.ReadTemplate_cs_i()
	open(cs_i_location, "w").write(processed_string)



    def ReadTemplate_cs_i(self, file_path = "cstmpl.i"):
        """ Reads in the template and constructs a string to go in cs.i.
	    This will have the headers replaced in it.

	    file_path - the path to cstmpl.i  Usually scripts/swig/cstmpl.i
	"""
	lines = rid_new_lines( open(file_path).readlines() )

	# Make the generated_headers.
	generated_headers = self.Generate_include_headers()

        processed_header_contents = self.Generate_header_contents()


        new_lines = ["//Auto generated file, do NOT edit.  Please edit cstmpl.i"]

	nl_append = new_lines.append
        for line in lines:
	    processed_line = replace(line, r"%%include_generated_headers%%", generated_headers)

	    processed_line = replace( processed_line, 
	                              r"%%processed_header_contents%%", 
				      processed_header_contents )

	    nl_append( processed_line )

        return join(new_lines, "\n")



    def Generate_header_contents(self):
        """  Makes a big string of all the header contents.
	"""
        processed_strings = []

	for base_in_dir, base_out_dir in self.filter_dict.keys():

	    for header_file_name in self.filter_dict[(base_in_dir, base_out_dir)].keys():

	        p = self._ReadProcessFile(base_in_dir,
	                          header_file_name, 
				  self.filter_dict[(base_in_dir, base_out_dir)][header_file_name] )
		processed_strings.append( p )

	return join(processed_strings, "\n\n")





    def Generate_include_headers(self):
        """ Makes the #include "header names" for the cs.i template.
	    Returns a string of the #includes.
	"""
        headers_i_list = ["// Start of generated headers."]
        
	for a_header_list in self.header_lists:
	    for header_name in a_header_list:
	        headers_i_list.append( '#include "scripts/swig/%s%s"' % (a_header_list.base_out, header_name) )

        headers_i_list.append( "// End of generated headers." )

        return string.join(headers_i_list, "\n")






    def Generate_generatd_i(self):
        """ Writes all of the %include <header name> to the headers.i file.
	    NOTE: these are written in the order they are placed in header_lists.
	"""
        headers_i_list = ["//Auto generated file, do NOT edit.  Please see README."]
        
	for a_header_list in self.header_lists:
	    for header_name in a_header_list:
	        headers_i_list.append( '%%include "scripts/swig/%s%s"' %(a_header_list.base_out, header_name) )

        return string.join(headers_i_list, "\n")







# --------------------------------------------
#  Miscellaneous stuff.

def rid_new_lines(list_lines):
    """ Returns the list_lines given without new lines.
         May be done in place.
    """

    #TODO: faster method to read in lines without new line chars.
    new_lines = []
    for line in list_lines:
        if line[-1] == '\n':
	    new_lines.append( line[:-1] )
	else:
	    new_lines.append( line )

    return new_lines



def main():
    """ 
    """

    if len(sys.argv) != 2:
        print "<include_file>"
	print "see README for details"
	sys.exit(1)


    # read in the include file.
    header_lists = get_include_file_names([ sys.argv[1] ])

    # make the directories for the processed include files.
    for a_header_list in header_lists:
        make_directories_for_headers(a_header_list.base_out, a_header_list)

    # get the config for the functions which process the headers.
    #  NOTE: at the moment, this does very little.
    fc = FilterConfig(header_lists)
    filter_dict = fc.MakeFilterDict()



    hp = HeaderProcessor(header_lists, filter_dict)

    # write the cs.i file.
    hp.Write_cs_i()


    # write the processed header files.
    hp.WriteHeaderFiles(filter_dict)

    # Write the generatd.i file.
    hp.Write_generatd_i()



if __name__ == "__main__":
    main()









