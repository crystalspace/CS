#Pysh Helper Functions

import os

def do_listdir(InDir):
	print "\nDirectory of "+os.getcwd()+'\\'+InDir
	for x in os.listdir(InDir):
		print x
	print
	
UserImports = ['*csshex', '*sys', '*os', '*cspace', 'cspacec']

UserAlias={
	'quit':'sys.exit(0)', 
	'exit':'sys.exit(0)', 
	'dir':'do_listdir(".")', 
};
