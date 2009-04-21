#!/usr/bin/python

#    -------------------------------------------------------------------------
#   ( Web front-end for CS' jamtemplate                                       )
#   ( Copyright (C) 2009 by Vincent Knecht <vknecht@users.sourceforge.net>    )
#   (                                                                         )
#   ( This CGI script runs Crystal Space' createproject.sh aka. jamtemplate,  )
#   ( generates MSVC project files then sends a tar.bz2 archive to the client.)
#   ( Hopefuly quite useful for people without Un*x'ish dev environment...    )
#   (                                                                         )
#   ( Special thanks to Andres Freund for the code review.                    )
#   (                                                                         )
#   ( This program is free software; you can redistribute it and/or modify    )
#   ( it under the terms of the GNU General Public License as published by    )
#   ( the Free Software Foundation; either version 2 of the License, or       )
#   ( (at your option) any later version.                                     )
#   (                                                                         )
#   ( This program is distributed in the hope that it will be useful,         )
#   ( but WITHOUT ANY WARRANTY; without even the implied warranty of          )
#   ( MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           )
#   ( GNU General Public License for more details.                            )
#   (                                                                         )
#   ( You should have received a copy of the GNU General Public License       )
#   ( along with this program; if not, write to the Free Software             )
#   ( Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA )
#   (                                                                         )
#    -------------------------------------------------------------------------
#          \   ^__^
#           \  (**)\_______
#              (__)\       )\/\
#               U  ||----w |
#                  ||     ||

import os, sys, re, time, subprocess, cgi, tarfile

# Should only be enabled for debugging...
#import cgitb; cgitb.enable()

#-----------------------------------------------------------------------------
# The CS & CEL checkouts must have been configured (use --without-cs for CEL)
# then jam cs-config & jam cel-config respectively in CS & CEL directories.
# Also install perl::TemplateToolkit (check ttree is present).
# myCS & myCEL  should contain eg. 'trunk', '1.4' and '1.2.1' subdirectories
myCS  = '/var/www/CS/'
myCEL = '/var/www/cel/'
myCP  = 'scripts/jamtemplate/createproject.sh'
# Make sure this directory is writtable by the webserver user.
# This script will attempt to create it if it doesn't exist.
myTMP = '/tmp/Progen/'
#-----------------------------------------------------------------------------

def printHTML (string, doHeader = True, doExit = True):
    if doHeader is True:
        print 'Content-Type: text/html'
        print
    print string
    if doExit is True:
        sys.exit()
    return

def checkDir (path, doCreate = False, doExitOnError = True):
    if os.path.isdir (path) is True:
        return True
    else:
        if doCreate is True:
            try:
                os.makedirs (path)
            except:
                printHTML ('<p>Problem creating %s</p>' % path)
        else:
            if doExitOnError:
                printHTML ('<p>Problem accessing %s</p>' % path)
    return False

def removeDir (path):
    for root, dirs, files in os.walk (path, topdown = False):
        for name in files:
            os.remove (os.path.join (root, name))
        for name in dirs:
            os.rmdir (os.path.join (root, name))
    os.rmdir (path)
    return

def changeDir (path):
    try:
        os.chdir (path)
    except:
        printHTML ('Error while changing directory to %s' % path)
    return 

if __name__ == "__main__":

    # Check CS, CEL and work directories
    for dir in myCS, myCEL:
        checkDir(dir)
    checkDir (myTMP, doCreate = True)
    changeDir (myTMP)

    # Fetch form data
    form = cgi.FieldStorage()

    # The fields expected by createproject.sh must come first, in correct order
    reqfields = ['shortname', 'longname', 'version', 'homepage', 'author', \
                 'email', 'copyright', 'gplcompat', 'usecel', 'csversion']

    # Don't put those additional fields in answer file
    addfields = ['csversion']

    # Regexps to check field values against
    regexps = { \
               'shortname': '^(?:([a-zA-Z0-9])){1,24}$', \
               'longname':  '^(?:([a-zA-Z0-9\'\"\_\-\ ])){1,79}$', \
               'version':   '^(?:(\\d+)\.){0,2}(\\d+)$', \
               'homepage':  '^(?:([a-zA-Z0-9\_\-\:\/\.\~])){4,79}$', \
               'author':    '^(?:([a-zA-Z0-9\_\-\.\ ])){1,79}$', \
               'email':     '^(?:([a-zA-Z0-9\_\-\.\@])){1,79}$', \
               'copyright': '^(?:([a-zA-Z0-9\_\-\.\(\)\ ])){1,79}$', \
               'gplcompat': '^yes|no$', \
               'usecel':    '^yes|no$', \
               'csversion': '^((?:(\\d+)\.){0,2}(\\d+)|trunk)$', \
              }

    # Keep valid field values in there
    answers = { }

    # Process the form, checking values against regexps when defined for a field
    for field in reqfields:
        # Are all required fields present ?
        if not (form.has_key (field)):
            printHTML ('<p>Please fill %s field</p>' % field)
        else:
            # Do we have a regex for this input ?
            if regexps.has_key (field):
                # Yes, check if it matches
                if re.match (regexps[field], form[field].value):
                    # Good, keep that answer
                    answers[field] = form[field].value
                else:
                    printHTML ('Invalid value for %s field' % field)
            else:
                # No, mmm, better reject it and stop the script
                printHTML ('Unable to validate %s field' % field)

    # Set CS & CEL environment variables depending on version choice
    os.environ['CRYSTAL'] = os.path.join (myCS, str (answers['csversion']))
    checkDir (os.environ['CRYSTAL'])
    os.environ['CEL'] = os.path.join (myCEL, str (answers['csversion']))
    checkDir (os.environ['CEL'])

    # Create an answer file to feed createproject.sh with
    answersfile = os.path.join (myTMP, 'workfile-' + answers['shortname'] \
                  + '-' + str (time.time ()))
    f = open (answersfile, 'w')
    for key in reqfields:
        # Skip field values not needed by createproject.sh
        if key not in addfields:
            f.write (answers[key] + '\n')
    f.close ()

    # Check if projdir already exists, remove it if so
    projdir = os.path.join (myTMP, answers['shortname'])
    if checkDir (projdir, doExitOnError = False):
        removeDir (projdir)

    # We need /dev/null to redirect script and jam outputs
    fnull = open ('/dev/null', 'w')

    # Call createproject.sh with answers
    f = open (answersfile, 'r')
    p = subprocess.Popen ([os.path.join (os.environ['CRYSTAL'], myCP)], \
                          stdout = fnull, stdin = f)
    sts = os.waitpid (p.pid, 0)
    f.close ()
    os.remove (answersfile)

    # Go into project's directory and run its configure script
    changeDir (projdir)
    p = subprocess.Popen (['./configure', '--without-cs', '--without-cel'], \
                          stdout = fnull)
    sts = os.waitpid (p.pid, 0)

    # Generate MSVC project files, and move them in correct place
    p = subprocess.Popen (['jam', 'msvcgen'], stdout = fnull)
    sts = os.waitpid (p.pid, 0)
    projfiles = os.listdir (os.path.join (projdir, 'out', 'msvc'))
    for file in projfiles:
        os.rename (os.path.join (projdir, 'out', 'msvc', file), \
                   os.path.join (projdir, 'msvc', file))

    # Clean generated files
    p = subprocess.Popen (['jam', 'distclean'], stdout = fnull)
    sts = os.waitpid (p.pid, 0)

    # Good boy, close /dev/null
    fnull.close ()
    # Go back to work directory
    changeDir (myTMP)
    # Rename project's directory so its name contains the version number
    projdirver = answers['shortname'] + '-' + answers['version']
    os.rename (projdir, os.path.join (myTMP, projdirver))

    # Create a bzip2 tar archive containing aforementioned directory
    archivename = projdirver + '.tar.bz2'
    tar = tarfile.open (archivename, 'w:bz2')
    tar.add (projdirver)
    tar.close ()

    # Remove the project's directory
    removeDir (projdirver)

    # Send the archive to the client's browser
    file = open (archivename, 'r')
    length = os.path.getsize (archivename)
    file.seek (0)
    print 'Content-Type: application/x-bzip'
    print 'Content-Disposition: attachment; filename=%s' % archivename
    print 'Content-Title: %s' % archivename
    print 'Content-Length: %i' % length
    print
    sys.stdout.write (file.read ())
    file.close ()

    # Remove the archive
    os.remove (archivename)

    # I love this script :-)

