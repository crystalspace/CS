#file: fixheaders_tests.py
"""
Purpose: unit tests for the various functions/classes in fixheaders.py

"""

import fixheaders
import unittest
import os
import shutil







class Test_make_directories_for_headers(unittest.TestCase):

    # set up, make testfiles/test_subdirs/ directory.
    # remove everything under there.
    def setUp(self):
        if not os.path.isdir("testfiles/test_subdirs"):
	    os.system("mkdir testfiles/test_subdirs")
	else:
	    shutil.rmtree("testfiles/test_subdirs/")
	    os.system("mkdir testfiles/test_subdirs")

    def testMakingClean(self):
        make_directories_for_headers = fixheaders.make_directories_for_headers
        make_directories_for_headers("testfiles/test_subdirs/", 
	                             ["csutil/asdf.h", "csgeom/blasdfbl.h",
				      "csutil/asdfsdfef.h", "csbla/sadf.h"])
	# check to see if the new dirs exist.
	self.assertEqual( os.path.isdir("testfiles/test_subdirs/csutil") and os.path.isdir("testfiles/test_subdirs/csbla") and os.path.isdir("testfiles/test_subdirs/csgeom"), 1 )
	    

    def testMakingWhenDirsAreThere(self):
        """The directories are there allready, trying to add them."""

        make_directories_for_headers = fixheaders.make_directories_for_headers
        make_directories_for_headers("testfiles/test_subdirs/", 
	                             ["csutil/asdf.h", "csgeom/blasdfbl.h",
				      "csutil/asdfsdfef.h", "csbla/sadf.h"])

        make_directories_for_headers("testfiles/test_subdirs/", 
	                             ["csutil/asdf.h", "csgeom/blasdfbl.h",
				      "csutil/asdfsdfef.h", "csbla/sadf.h"])
	self.assertEqual( os.path.isdir("testfiles/test_subdirs/csutil") and os.path.isdir("testfiles/test_subdirs/csbla") and os.path.isdir("testfiles/test_subdirs/csgeom") , 1)



class Test_create_iObjectRegistry(unittest.TestCase):
    def testOutput(self):
        """checking if output of iObjectRegistry is ok for swig. """
	example = """\
//****** System Interface
struct iObjectRegistry:public iBase
{
public:
  %addmethods
  {
    iEngine* Query_iEngine()
    {
      iEngine* rval_iEngine = CS_QUERY_REGISTRY (self, iEngine);
      rval_iEngine->DecRef ();
      return rval_iEngine;
    }

    void Print(int mode, const char* format) {
      printf (format);
    }
  }
};"""
	
	generated = fixheaders.create_iObjectRegistry(["iEngine"])

        self.assertEqual(generated, example)



class Test_HeaderProcessor(unittest.TestCase):

    def test_WritingFiles(self):
        """ reading in files, and writing filtered ones. """
        # Filter config only returns a dict with all header files, and 
	#   all functions at the moment.

	#inc_exc = fixheaders.IncludeExclude(["testfiles/includes.txt"], 
	#                                    ["testfiles/excludes.txt"])
	#file_names = inc_exc.GetFileNames()

	file_names = fixheaders.get_include_file_names(["testfiles/includes.txt"])

	#print file_names

	fc = fixheaders.FilterConfig(file_names)
	#print fc
	filter_dict = fc.MakeFilterDict()

        hp = fixheaders.HeaderProcessor(file_names)
	#print filter_dict
	hp.WriteHeaderFiles(filter_dict)


    def test_Generate_generatd_i(self):
        """  Testing the Generate_generatd_i which makes a headers.i file.
	"""

	file_names = fixheaders.get_include_file_names(["testfiles/includes.txt"])
        hp = fixheaders.HeaderProcessor(file_names)

	headers_i_string = hp.Generate_generatd_i()
	#print "\n::%s::\n" % headers_i_string


        should_be  = """//Auto generated file, do NOT edit.  Please see README.
%include "scripts/swig/testfiles/output/csutil/bitset.h"
%include "scripts/swig/testfiles/output/csutil/csevcord.h"
%include "scripts/swig/testfiles/output/csutil/csevent.h"
%include "scripts/swig/testfiles/output/csutil/csinput.h\""""

        if should_be != headers_i_string:
	    raise "wrong value"
        
    def test_ReadTemplate_cs_i(self):
        """  This test needs to be improved, to check the output...
	       This will do for now.
	     TODO: make better test.
	"""
        
	file_names = fixheaders.get_include_file_names(["testfiles/includes.txt"])
        hp = fixheaders.HeaderProcessor(file_names)
	cs_i = hp.ReadTemplate_cs_i()
	#print cs_i


if __name__ == "__main__":
    unittest.main()



