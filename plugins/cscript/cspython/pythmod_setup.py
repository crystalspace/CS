#!/usr/bin/env python

# Copyright (C) 2003 Rene Jager <renej@frog.nl>
# License: LGPL

# Arguments: install <gen-files-dir> <include-dir> <libraries-dir> <setup-args>
# See cspython.mak for example usage.

import sys, os, string, traceback, re
from distutils import ccompiler, sysconfig
from distutils.core import setup, Extension

# get non-distutils args and remove them
src_dir = sys.argv[1]
inc_dir = sys.argv[2]
lib_dir = sys.argv[3]
sys.argv[1:4] = []

ext_module = Extension(
    '_cspace',
    [src_dir+'/cs_pyth.cpp', 'plugins/cscript/cspython/pythmod.cpp'],
    include_dirs=[inc_dir],
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
    'package_dir'  : {'' : src_dir},
    'py_modules'   : ['cspace'],
    'ext_modules'  : [ext_module],
}

if ccompiler.get_default_compiler() == 'unix':
    ldshared = re.sub('gcc', 'g++', sysconfig.get_config_var('LDSHARED'))
    sysconfig.get_config_vars()['LDSHARED'] = ldshared

dist = apply(setup, [], setup_kwargs)

