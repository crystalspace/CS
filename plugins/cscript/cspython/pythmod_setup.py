#!/usr/bin/env python

# Copyright (C) 2003 Rene Jager <renej@frog.nl>
# License: LGPL

# Arguments: <c++compiler> <derived-files-dir> <source-dir> \
#            [<include-dir> ...] -- [<library-dir> ...] -- [<lib> ...] -- \
#            [<cflags> ...] -- [<lflags> ...] -- [<setup-args>]
# Bracketed arguments are optional; delimiters `--' are not.  You can list any
# number of include and library directories; and any number of extra link
# libraries (with or without the -l directive).  Terminate each list with a
# literal `--'.  Any number of additional compiler and linker flags may be
# listed, also terminated with `--'.  Setup arguments (setup-args) are any
# additional command-line arguments which should be passed to
# distutils.core.setup.

import sys, os, string, traceback, re
from distutils import ccompiler, sysconfig
from distutils.core import setup, Extension

# Get non-distutils args and remove them.
i = 1
cxx = sys.argv[i] ; i += 1
derived_dir = sys.argv[i] ; i += 1
src_dir = sys.argv[i] ; i += 1

inc_dir = []
while i < len(sys.argv) and sys.argv[i] != '--':
    inc_dir.append(sys.argv[i])
    i += 1
i += 1 # skip '--'

lib_dir = []
while i < len(sys.argv) and sys.argv[i] != '--':
    lib_dir.append(sys.argv[i])
    i += 1
i += 1 # skip '--'

libs = []
while i < len(sys.argv) and sys.argv[i] != '--':
    lib = sys.argv[i]
    lib = lib[:2] == "-l" and lib[2:] or lib
    libs.append(lib)
    i += 1
i += 1 # skip '--'

cflags = []
while i < len(sys.argv) and sys.argv[i] != '--':
    cflags.append(sys.argv[i])
    i += 1
i += 1 # skip '--'

lflags = []
while i < len(sys.argv) and sys.argv[i] != '--':
    lflags.append(sys.argv[i])
    i += 1
i += 1 # skip '--'

sys.argv[1:i] = []

ext_module = Extension(
    '_cspace',
    [derived_dir+'/cs_pyth.cpp',
     src_dir+'/plugins/cscript/cspython/pythmod.cpp'
     ],
    include_dirs=inc_dir,
    library_dirs=lib_dir,
    libraries=libs,
    extra_compile_args=cflags,
    extra_link_args=lflags
)

setup_kwargs = {
    'name'         : 'cspace',
    'description'  : 'Python Crystal Space Module',
    'url'          : 'http://www.crystalspace.org',
    'license'      : 'LGPL',
    'package_dir'  : {'' : derived_dir},
    'py_modules'   : ['cspace'],
    'ext_modules'  : [ext_module]
}

if ccompiler.get_default_compiler() == 'unix':
    ldshared = sysconfig.get_config_var('LDSHARED')
    ldshared = re.sub('gcc', cxx, ldshared)
    ldshared = re.sub('-arch\s+(i386|ppc)', '', ldshared) # Sanitize MacOS/X.
    sysconfig.get_config_vars()['LDSHARED'] = ldshared

dist = apply(setup, [], setup_kwargs)
