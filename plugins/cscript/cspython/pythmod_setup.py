#!/usr/bin/env python

# Copyright (C) 2003 Rene Jager <renej@frog.nl>
# License: LGPL

# Arguments: install <derived-files-dir> <source-dir> <include-dir> ... -- \
#                    <libraries-dir> <lib> ... -- <setup-args>
# You can list any number of include directories.  Terminate the list with --.
# You can list any number of extra libraries (sans -l).  Terminate with --.
# See cspython.mak for example usage.

import sys, os, string, traceback, re
from distutils import ccompiler, sysconfig
from distutils.core import setup, Extension

# get non-distutils args and remove them
i = 1
derived_dir = sys.argv[i] ; i += 1
src_dir = sys.argv[i] ; i += 1
incs = []
while i < len(sys.argv) and sys.argv[i] != '--':
    incs.append(sys.argv[i])
    i += 1
i += 1 # skip '--'
if sys.argv[i] == '--':
    lib_dir = ''
else:
    lib_dir = sys.argv[i] ; i += 1
libs = []
while i < len(sys.argv) and sys.argv[i] != '--':
    thelib = sys.argv[i]
    thelib = thelib[:2] == "-l" and thelib[2:] or thelib
    libs.append(thelib)
    i += 1
sys.argv[1:i + 1] = []

libs.extend(['cstool','csgfx','csgeom','csutil'])

ext_module = Extension(
    '_cspace',
    [derived_dir+'/cs_pyth.cpp',
     src_dir+'/plugins/cscript/cspython/pythmod.cpp'
     ],
    include_dirs=incs,
    library_dirs=[lib_dir],
    libraries=libs
)

setup_kwargs = {
    'name'         : 'cspace',
    'description'  : 'Python Crystal Space Module',
    'url'          : 'http://www.crystalspace.org',
    'license'      : 'LGPL',
    'package_dir'  : {'' : derived_dir},
    'py_modules'   : ['cspace'],
    'ext_modules'  : [ext_module],
}

if ccompiler.get_default_compiler() == 'unix':
    ldshared = re.sub('gcc', 'g++', sysconfig.get_config_var('LDSHARED'))
    sysconfig.get_config_vars()['LDSHARED'] = ldshared

dist = apply(setup, [], setup_kwargs)
