#! /usr/bin/env python

### Imports
import string, traceback, os, sys, string

try:
	import readline 
except ImportError:
	# If the import failed, raw_input will still function, but without readline functionality.
	print "No readline module found"

### "Hard-coded Constants":
VERSION = '0.1'

# This is the value os.system returns when my shell (bash) can't find the command:
SH_COMMAND_NOT_FOUND = 32512

# Use these to assign values to pseudo-booleans, but use (<BOOLEAN>) to test:
TRUE = 1
FALSE = 0

### Dynamic Variables:
UserGlobals = {
	'Prompt1': 'pysh%s$ ' % VERSION,
	'Prompt2': ' ' + ' ' * len(VERSION) + '...$ '
}
UserLocals = {}

from csshex import UserAlias;
from csshex import UserImports;

### Function Definitions:
def get_command(Prompt = UserGlobals['Prompt1']):
	Input = None
	while not Input:
		Input = string.rstrip(raw_input(Prompt))
	if (Input[-1] == ':'):
		Input = Input + get_command(UserGlobals['Prompt2'])
	return Input

def shell(Command):
	if os.system(Command) == SH_COMMAND_NOT_FOUND:
			traceback.print_exc()

def debug(Output):
	try:
		if UserGlobals['DEBUG']:
			sys.stderr.write(Output)
			sys.stderr.flush()
	except KeyError:
		pass

#TODO Add support for parameters using the $1 perl notation or equiv
def Alias(Command):
	if UserAlias.has_key(Command):
		return UserAlias[Command]
	return Command

#Import default libraries	
print 'Loading',
for x in UserImports:
	print x,
	exec 'from '+x+' import *' in UserGlobals, UserLocals
	
print 

### Main
while TRUE:
	try:
		Command = get_command()
		debug('Input: %s' % `Command`)
	except EOFError:
		print
		raise SystemExit, 0
		
	if Command[0] != '#':
		Command=Alias(Command)
	else:
		Command=Command[1:]
		
	if Command[0] == ';':
		shell(Command[1:])
	else:
		try:
			exec Command in UserGlobals, UserLocals
		except NameError, Description:
			shell(Command)
		except SystemExit: #Allow script to exit
			raise SystemExit
		except:
			traceback.print_exc()

