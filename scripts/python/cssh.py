#! /usr/bin/env python

import string, traceback, os, sys, string
VERSION = '0.1'
SH_COMMAND_NOT_FOUND = 32512

from csshex import UserAlias, UserImports

class csShell:
	UserGlobals = {}
	UserLocals = {}
	Prompt='[cssh%s ' % VERSION

	def GetPrompt(self):
		return self.Prompt + '%s] ' % os.getcwd()

	def GetCommand(self):
		Input = None
		while not Input:
			Input = string.rstrip(raw_input(self.GetPrompt()))
		return Input

	def Shell(self, Command):
		if os.system(Command) == SH_COMMAND_NOT_FOUND:
			traceback.print_exc()

#TODO Add support for parameters using the $1 perl notation or equiv
	def Alias(self, Command):
		if UserAlias.has_key(Command):
			return UserAlias[Command]
		return Command

	def LoadImports(self):
		print 'Loading',
		for x in UserImports:
			print x,
			try:
				if(x[0]=='*'):
					exec 'from '+x[1:]+' import *' in self.UserGlobals, self.UserLocals
				else:
					exec 'import '+x in self.UserGlobals, self.UserLocals
			except ImportError, e:
				print
				print e
		print

	def Handle(self, Command):
		Exec=1
		if Command[0] == '#':
			Command=Command[1:]
		else:
			Command=self.Alias(Command)			
			
		if Command[0] == ';':
			Command=Command[1:]
			self.Shell(Command)
			Exec=0
			
		else:
			try:
				if(Exec):
					exec Command in self.UserGlobals, self.UserLocals
			except SystemExit: #Allow script to exit
				raise SystemExit
			except:
				traceback.print_exc()

	def Loop(self):
		while 1:
			try:
				Command = self.GetCommand()
				self.Handle(Command)
			except EOFError:
				print
				raise SystemExit, 0
			
	def __init__(self):
		self.LoadImports()
		
shell=csShell()
shell.Loop()

		

