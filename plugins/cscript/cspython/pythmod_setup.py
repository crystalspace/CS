#!/usr/bin/env python

import sys, os, string, traceback, re
from distutils import ccompiler, sysconfig
from distutils.core import setup, Extension

if sys.argv[1] == 'pythmod_clean':

    try:
        f = open('pythmod_files')
        for l in f.readlines():
            try:
                os.remove(string.strip(l))
            except:
                pass
        f.close()
        os.remove('pythmod_files')
    except:
        #traceback.print_exc()
        pass
    sys.exit(0)

elif sys.argv[1] == 'pythmod_install':

    OUT = sys.argv[2]
    SCRIPTDIR = sys.argv[3]
    INCDIRS = sys.argv[4:]

    argv = ['install',
            '--install-purelib', SCRIPTDIR,
            '--install-platlib', SCRIPTDIR,
            '--record', 'pythmod_files'
           ]
    sys.argv[1:] = argv

    ext_module = Extension(
        '_cspace',
        ['cs_pyth.cpp', 'pythmod.cpp'],
        include_dirs=INCDIRS,
        library_dirs=[OUT],
        libraries=['cstool', 'csgfx', 'csgeom', 'cssys', 'csutil',
                   'cssys', 'csutil'
                  ]
    )

    setup_kwargs = {
        'name'         : 'cspace',
        #'version'      : '1.0.1',
        'description'  : 'Python Crystal Space Module',
        'url'          : 'http://www.crystalspace.org',
        'license'      : 'LGPL',
        'py_modules'   : ['cspace'],
        'ext_modules'  : [ext_module],
        'author'       : 'Rene Jager',
        'author_email' : 'renej.frog@yucom.be'
    }

    if ccompiler.get_default_compiler() == 'unix':
        ldshared = re.sub('gcc', 'g++', sysconfig.get_config_var('LDSHARED'))
        sysconfig.get_config_vars()['LDSHARED'] = ldshared

    dist = apply(setup, [], setup_kwargs)

