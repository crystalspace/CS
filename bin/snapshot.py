#!/usr/bin/env python
#==============================================================================
#    CVS Snapshot Generation Script
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#==============================================================================
#------------------------------------------------------------------------------
# snapshot.py
#
#    A tool for generating snapshots and 'diffs' of a module within a CVS
#    repository.
#
#    Author's note: This script can certainly be improved.  Better error
#    handling, more options (such as --verbose, --quiet, etc.), better
#    abstraction and generalization, are all future possibilities.  There is
#    a lot of room for improvement.
#
#------------------------------------------------------------------------------
import sys, os, glob, grp, tempfile, time

#------------------------------------------------------------------------------
# Configuration Section
#    cvsroot - CVSROOT setting for CVS.
#    cvsmodule - The module to checkout from the CVS repository.
#    moduledir - The name of the directory which is created when the module
#        is checked out from CVS (frequently identical to cvsmodule).
#    ownergroup - The group name which will be given to the 'chgrp' command
#        when directories are created.  Assigning group ownership to a
#        directory allows others in the group to manipulate the contents of
#        the directory.  May be None.
#    packprefix - Prefix used to compose the final package name of the form
#        prefix-YYYY-MM-DD-*.ext".
#    snapdir - Directory where snapshot packages will be placed.
#    keepsnaps - Number of recent packages to retain.
#------------------------------------------------------------------------------

cvsroot = ":pserver:anonymous@cvs1:/cvsroot/crystal"
cvsmodule = "crystal"
moduledir = "CS"
ownergroup = "crystal"
packprefix = "cs-"
snapdir = "/home/groups/ftp/pub/crystal/cvs-snapshots"
keepsnaps = 4

#------------------------------------------------------------------------------
# Snapshot Class
#------------------------------------------------------------------------------
class Snapshot:
    def __init__(self):
        self.packbase = packprefix + time.strftime(
            "%Y-%m-%d.%H%M%S", time.gmtime(time.time()))
        self.workdir = os.path.join(snapdir, "transient")
        self.logdir  = os.path.join(snapdir, "logs")
        self.logext = ".log"
        self.logname = self.packbase + self.logext
        self.logpath = os.path.join(self.logdir, self.logname)
        self.logfile = None
        self.packext = ".tgz"
        self.packname = self.packbase + self.packext
        self.packtemplate = packprefix + "????-??-??.*" + self.packext
        self.linkname = packprefix + "current-snapshot" + self.packext
        self.diffext = ".diff.gz"
        self.diffname = self.packbase + self.diffext

    def log(self, msg):
        s = msg + "\n"
        sys.stdout.write(s)
        if self.logfile:
            self.logfile.write(s)

    def timestamp(self):
        return time.asctime(time.gmtime(time.time())) + " UTC"

    def run(self, cmd):
        ok = None
        try:
            out = os.popen(cmd + " 2>&1")
        except IOError, e:
            self.log("Error running command: " + cmd +
                     " (" + repr(e.args) + ")")
        else:
            s = out.readline()
            while s != "":
                self.log(s)
                s = out.readline()
            rc = out.close()
            if rc == None:
                ok = 1 # OK
            else:
                self.log("Command exited abnormally: " + cmd +
                         " (" + str(rc) + ")")
        return ok

    def makedirectory(self, path):
        if not os.path.exists(path) :
            os.mkdir(path)
	try:
            os.chmod(path, 0775)
        except Exception, e:
            self.log("Error making directory group writable: " +
                     path + '; reason: ' + str(e))
        if ownergroup:
            try:
                os.chown(path, os.getuid(), grp.getgrnam(ownergroup)[2])
            except Exception, e:
                self.log('Error setting group ownership "' + ownergroup +
                         '" on ' + path + '; reason: ' + str(e))

    def gendiff(self):
        # Must call this before installing new package in snapshot directory.
        files = glob.glob(os.path.join(snapdir, self.packtemplate))
        if len(files) > 0:
            files.sort()
            files.reverse()
            prev = files[0]
            self.log("Generating diff against: " + os.path.basename(prev))
            savedir = os.getcwd()
            olddir = "old"
            oldpath = os.path.join(self.builddir, "old")
            self.makedirectory(oldpath)
            os.chdir(oldpath)
            rc = self.run("tar xfz " + prev)
            os.chdir(savedir)
            if rc:
                self.run("diff -crN " + os.path.join(olddir, moduledir) +
                         " " + moduledir + " | gzip > " + self.diffname)

    def openlog(self):
        if not self.logfile:
            try:
                self.logfile = open(self.logpath, "w")
            except IOError, e:
                self.log("Error opening log file: " + self.logpath + " " +
                         repr(e.args))

    def closelog(self):
        if self.logfile:
            self.logfile.close()
            self.logfile = None

    def purge(self, pattern):
        files = glob.glob(pattern)
        blast = len(files) - keepsnaps
        if blast > 0:
            files.sort()
            for i in range(0, blast):
                self.log("Purging old file: " + os.path.basename(files[i]))
                os.remove(files[i])

    def purgeold(self):
        self.purge(os.path.join(self.logdir, packprefix + "*" + self.logext))
        self.purge(os.path.join(snapdir, packprefix + "*" + self.diffext))
        self.purge(os.path.join(snapdir, self.packtemplate))

    def purgetransient(self):
        savedir = os.getcwd()
        os.chdir(snapdir)
        self.log("Purging working directory: " + self.builddir)
        # Remove our local work directory.
        self.run("rm -rf " + self.builddir)
        # Attempt to remove parent work directory.  This will only succeed if
        # it is empty (i.e. no other snapshots are in progress).
        try:
            os.rmdir(self.workdir)
        except Exception:
            pass
        os.chdir(savedir)

    def preparetransient(self):
        tempfile.tempdir = self.workdir
        self.builddir = tempfile.mktemp();
        self.log("Creating working directory: " + self.builddir)
        self.makedirectory(self.workdir)
        self.makedirectory(self.builddir)

    def linkcurrent(self):
        self.log("Linking current package to: " + self.linkname)
        if os.path.exists(self.linkname):
            os.remove(self.linkname)
        os.symlink(self.packname, self.linkname)
                            
    def dobulk(self):
        self.log("Retrieving module: " + cvsmodule)
        if self.run("cvs -Q -d " + cvsroot + " checkout " + cvsmodule):
            self.gendiff()
            self.log("Creating package: " + self.packname)
            if self.run("tar cfz " + self.packname + " " + moduledir):
                self.log("Installing package into: " + snapdir)
                if self.run("mv " + self.packname + " " + snapdir):
                    if os.path.exists(self.diffname):
                        self.run("mv " + self.diffname + " " + snapdir)
                    os.chdir(snapdir)
                    self.linkcurrent()
                    self.purgeold()

    def doall(self):
        self.makedirectory(snapdir)
        self.makedirectory(self.logdir)
        self.openlog()
        self.log("BEGIN: " + self.timestamp())
        try:
            self.preparetransient()
            os.chdir(self.builddir)
            try:
                self.dobulk()
            except Exception, e:
                self.log("A fatal exception occurred: " + str(e))
            self.purgetransient()
        finally:
            self.log("END: " + self.timestamp())
            self.closelog()

tool = Snapshot()
tool.doall()
