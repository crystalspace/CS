#!/usr/bin/env python

# Copyright (C) 2003 Rene Jager <renej@frog.nl>
# License: LGPL

# Arguments: install <derived-files-dir> <source-dir> <include-dir> \
#                    <alt-include-dir> <libraries-dir> <setup-args>
# See cspython.mak for example usage.

import sys, os, string, traceback, re
from distutils import ccompiler, sysconfig
from distutils.core import setup, Extension

# get non-distutils args and remove them
derived_dir = sys.argv[1]
src_dir = sys.argv[2]
inc_dir = sys.argv[3]
inc_dir_alt = sys.argv[4]
lib_dir = sys.argv[5]
sys.argv[1:6] = []

ext_module = Extension(
    '_cspace',
    [derived_dir+'/cs_pyth.cpp',
     src_dir+'/plugins/cscript/cspython/pythmod.cpp'
     ],
    include_dirs=[inc_dir, inc_dir_alt],
    library_dirs=[lib_dir],
    libraries=['cstool', 'csgfx', 'csgeom', 'cssys', 'csutil',
               'cssys', 'csutil'
              ]
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

