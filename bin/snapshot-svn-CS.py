#!/usr/bin/env python
#==============================================================================
#
#    CVS Snapshot Generation Script
#    Copyright (C) 2000-2005 by Eric Sunshine <sunshine@sunshineco.com>
#                       2006 by Marten Svanfeldt <developer@svanfeldt.com
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#==============================================================================
#------------------------------------------------------------------------------
# snapshot-svn.py
#
#    A tool for generating snapshots and 'diffs' of a directory within a SVN
#    repository.
#
#    Typically this script is run via the 'cron' daemon by the machine on
#    which the final snapshots will reside.  See the cron(8), crontab(1), and
#    crontab(8) man pages for information specific to cron.  A typical crontab
#    entry which runs this script at 01:03 each morning might look like this:
#
#        MAILTO = sunshine@sunshineco.com
#        3 1 * * * $HOME/bin/snapshot-SVN.py
#
#    The script makes no attempt to perform any sort of SVN authentication.
#    Currently, it is the client's responsibility to authenticate with the SVN
#    server if necessary.  
#
#    The configuration settings 'stripdotsvn' specifies if .svn directories
#    should be stripped or not. Stripping .svn directories leads to vastly
#    smaller snapshots and diffs, however it also makes you unable to update
#    from the repository after extracting the snapshot.
#
#    Author's note: This script can certainly be improved.  Better error
#    handling, more options (such as --verbose, --quiet, etc.), better
#    abstraction and generalization, are all future possibilities.  There is
#    room for improvement.
#
#------------------------------------------------------------------------------
import commands, glob, grp, os, re, string, sys, tempfile, time

prog_name = "snapshot.py"
prog_version = "15"
author_name = "Eric Sunshine"
author_email = "sunshine@sunshineco.com"
author_info = author_name + " <" + author_email + ">"
copyright = "Copyright (C) 2000-2005 by " + author_info + "\nSVN support by Marten Svanfeldt"

#------------------------------------------------------------------------------
# Configuration Section
#    svnurl - URL of directory to check out.
#    stripdotsvn - If .svn directories should be stripped.
#    basedir - The name of the directory which is created when the module is
#        checked out from SVN (frequently identical to last part of svnurl).
#    ownergroup - The group name which will be given to the 'chgrp' command
#        when directories are created.  Assigning group ownership to a
#        directory allows others in the group to manipulate the contents of
#        the directory.  May be None.
#    packprefix - Prefix used to compose the final package name of the form
#        prefix-YYYY-MM-DD-HHMMSS.ext".
#    snapdir - Directory where snapshot packages will be placed.
#    checksumfile - Name of checksum file which will be placed in each archiver
#        subdirectory. This file will contain checksums for all the published
#        packages in the directory.
#    checksumprog - Name of program to compute checksums of packages. The
#        program should accept a list of filenames for which it should compute
#        checksums. It should also emit a report on its standard-output stream
#        which can be redirected to 'checksumfile'.
#    keepsnaphots - Number of historical snapshots to retain.
#    keepdiffs - Number of historical 'diffs' to retain.
#    keeplogs - Number of historical log files to retain.
#    workdir - Temporary working directory for checkouts.
#    warnlevel - Warning level.  Defaults is 0.  Higher values may produce
#        warnings about certain non-fatal problems, such as when "chgrp" on
#        a directory fails when user is not owner of directory.
#    archivers - A tuple of archivers used to generate the project packages.
#        Each tuple element is a dictionary with the following keys.  The key
#        "name" specifies the name of the directory under 'snapdir' into which
#        this archived package will be placed.  The key "dir" is a dictionary
#        describing how to archive a directory into a single package.  The key
#        "file" is a dictionary describing how to archive a single file into a
#        package.  The "dir" and "file" dictionaries contain the following
#        keys.  The key "ext" is the file extension for the generated file.
#        The key "cmd" is the actual command template which describes how to
#        generate the given archive.  It may contain the meta-tokens @S and
#        @D.  The token @S is replaced with the name of the source directory
#        or file which is being archived, and @D is replaced with the final
#        destination package name.
#------------------------------------------------------------------------------

svnurl = "file:///home/crystal/scm/crystal/CS/trunk/"
stripdotsvn = True
basedir = "CS"
ownergroup = "crystal"
packprefix = "cs-"
snapdir = "/home/crystal/www/htdocs/cvs-snapshots"
checksumfile = "checksums.md5"
checksumprog = "md5sum"
keepsnapshots = 2
keepdiffs = 14
keeplogs = 14
workdir = "/tmp"
warnlevel = 0

archivers = (
    {"name": "gzip",
     "dir": {"ext": "tgz", "cmd": "tar --create --file=- @S | gzip > @D"},
     "file": {"ext": "gz", "cmd": "gzip --stdout @S > @D"}},
    {"name": "bzip2",
     "dir": {"ext": "tar.bz2", "cmd": "tar --create --file=- @S | bzip2 > @D"},
     "file": {"ext": "bz2", "cmd": "bzip2 --stdout @S > @D"}},
    {"name": "zip",
     "dir": {"ext": "zip", "cmd": "zip -q -r @D @S"},
     "file": {"ext": "zip", "cmd": "zip -q @D @S"}})

#------------------------------------------------------------------------------
# Directory Stack Class
#------------------------------------------------------------------------------
class DirStack:
    stack = []

    def pushdir(self, dir):
        self.stack.append(os.getcwd())
        os.chdir(dir)

    def popdir(self):
        os.chdir(self.stack[-1])
        del self.stack[-1]

#------------------------------------------------------------------------------
# Snapshot Class
#------------------------------------------------------------------------------
class Snapshot:
    def __init__(self):
        self.packtemplate = packprefix + "????-??-??.??????"
        self.packbase = packprefix + time.strftime(
            "%Y-%m-%d.%H%M%S", time.gmtime(time.time()))
        self.linkbase = packprefix + "current-snapshot"
        self.diffext = ".diff"
        self.diffname = self.packbase + self.diffext
        self.logdir  = os.path.join(snapdir, "logs")
        self.logext = ".log"
        self.logname = self.packbase + self.logext
        self.logpath = os.path.join(self.logdir, self.logname)
        self.logfile = None
        self.revisionpath = os.path.join(self.logdir, "lastrun.revision")
        self.revision = 0
        self.hasdiff = False
        self.dirstack = DirStack()
        self.svncommand = '/usr/bin/svn'


    def timenow(self):
        return time.asctime(time.gmtime(time.time())) + " UTC";

    def log(self, msg):
        s = msg + "\n"
        sys.stdout.write(s)
        if self.logfile:
            self.logfile.write(s)

    def run(self, cmd):
        rc = commands.getstatusoutput(cmd)
        if len(rc[1]) > 0:
            self.log("Command failed: " + cmd)
            self.log(rc[1])
        return (rc[0] == 0)

    def removefile(self, path):
        try:
            os.remove(path)
        except OSError, e:
            if warnlevel > 1:
                self.log('Error removing file "' + path + '"; reason: ' +
                    str(e))

    def makedirectory(self, path):
        if not os.path.exists(path) :
            os.mkdir(path)
        try:
            os.chmod(path, 0775)
        except Exception, e:
            if warnlevel > 0:
                self.log("Error making directory group writable: " +
                         path + '; reason: ' + str(e))
        if ownergroup:
            try:
                os.chown(path, os.getuid(), grp.getgrnam(ownergroup)[2])
            except Exception, e:
                if warnlevel > 0:
                    self.log('Error setting group ownership "' + ownergroup +
                             '" on ' + path + '; reason: ' + str(e))

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

    def writerevision(self):
        file = open(self.revisionpath, "w")
        file.write(self.revision + "\n")
        file.close()

    def readrevision(self):
        revision = None
        if os.path.exists(self.revisionpath):
            file = open(self.revisionpath, "r")
            revision = string.strip(file.readline())
            file.close()
        return revision

    def purge(self, pattern, keep):
        files = glob.glob(pattern)
        blast = len(files) - keep
        if blast > 0:
            files.sort()
            for i in range(0, blast):
                self.log("Purging old file: " + os.path.basename(files[i]))
                os.remove(files[i])

    def purgeold(self):
        self.purge(os.path.join(
            self.logdir, self.packtemplate + self.logext), keeplogs)
        for dict in archivers:
            self.purge(os.path.join(
                snapdir, dict["name"], self.packtemplate +
                "." + dict["dir"]["ext"]), keepsnapshots)
            self.purge(os.path.join(
                snapdir, dict["name"], self.packtemplate + self.diffext +
                "." + dict["file"]["ext"]), keepdiffs)

    def purgetransient(self):
        self.log("Purging working directory")
        self.removedir(self.builddir)

    def preparetransient(self):
        tempfile.tempdir = workdir
        self.builddir = tempfile.mktemp()
        self.log("Creating working directory: " + self.builddir)
        self.makedirectory(self.builddir)

    def removedir(self, dir):
        for root, dirs, files in os.walk(dir, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def cleardotsvn(self, dir):
        dirsToRemove = []
        for root, dirs, files in os.walk(dir):
            if '.svn' in dirs:
                dirsToRemove.extend([os.path.join(root, '.svn')])
                dirs.remove('.svn')
        for dir in dirsToRemove:
            self.removedir(dir)

    def checkout(self, outdir):
        self.log("Retrieving latest version of " + svnurl)
        self.makedirectory(outdir)
        self.dirstack.pushdir(outdir)
        rc = self.run(self.svncommand + " export -q " + svnurl + ' ' + basedir)
        if rc:
            # Get revision
            rc2 = commands.getstatusoutput(self.svncommand + " info " +
                                           svnurl)
            if rc2[0] == 0:
                try:
                    s = rc2[1]
                    self.revision = re.compile("Revision: (\d+)").search(s).group(1)
                except Exception, e:
                    self.log("Error getting SVN revision")
                    self.log(e)
                    return False
            else:
                self.log("Error getting SVN info")
                return False
        self.dirstack.popdir()
        return rc

    def gendiff(self):
        oldrevision = self.readrevision()
        if oldrevision:
            self.log("Generating diff of " + oldrevision + " & " +
                     self.revision)
            self.run(self.svncommand + " diff -r " + oldrevision + ":" +
                     self.revision + " " + svnurl +  " > " + self.diffname)
            self.hasdiff = True

    def genpackage(self, dirname, dict, src, dst):
        outdir = os.path.join(snapdir, dirname)
        self.makedirectory(outdir)
        target = os.path.join(outdir, dst + "." + dict["ext"])
        cmd = string.replace(
            string.replace(dict["cmd"], "@S", src), "@D", target)
        return self.run(cmd)

    def genpackages(self):
        self.dirstack.pushdir(self.builddir)
        for dict in archivers:
            name = dict["name"]
            self.log("Generating '" + name + "' packages")
            if self.genpackage(name, dict["dir"], basedir, self.packbase):
                if self.hasdiff:
                    self.genpackage(name, dict["file"], self.diffname,
                                    self.diffname)
        self.dirstack.popdir()
        self.writerevision()

    def makelink(self, ext, src, linkname):
        src = src + "." + ext
        linkname = linkname + "." + ext
        self.removefile(linkname)
        os.symlink(src, linkname)

    def makelinks(self):
        for dict in archivers:
            name = dict["name"]
            self.log("Linking to current '" + name + "' packages")
            self.dirstack.pushdir(os.path.join(snapdir, name))
            self.makelink(dict["dir"]["ext"], self.packbase, self.linkbase)
            if self.hasdiff:
                self.makelink(dict["file"]["ext"],
                              self.packbase + self.diffext,
                              self.linkbase + self.diffext)
            self.dirstack.popdir()

    def checksum(self, files):
        self.removefile(checksumfile)
        if len(files) > 0:
            self.run(checksumprog + ' "' + '" "'.join(files) + '" > ' +
                     checksumfile)

    def checksums(self):
        for dict in archivers:
            name = dict["name"]
            extd = "." + dict["dir"]["ext"]
            extf = "." + dict["file"]["ext"]
            self.log("Generating checksums for '" + name + "' packages")
            self.dirstack.pushdir(os.path.join(snapdir, name))
            files = []
            files.extend(glob.glob(self.packtemplate + extd))
            files.extend(glob.glob(self.packtemplate + self.diffext + extf))
            files.extend(glob.glob(self.linkbase + extd))
            files.extend(glob.glob(self.linkbase + self.diffext + extf))
            self.checksum(files)
            self.dirstack.popdir()

    def dobulk(self):
        if self.checkout(self.builddir):
            self.gendiff()
            #if stripdotsvn:
            #    self.cleardotsvn(self.builddir)
            self.genpackages()
            self.makelinks()
            self.purgeold()
            self.checksums()

    def doall(self):
        self.makedirectory(snapdir)
        self.makedirectory(self.logdir)
        self.openlog()
        self.log(prog_name + " version " + prog_version)
        self.log(copyright + "\n")
        self.log("BEGIN: " + self.timenow())
        try:
            self.preparetransient()
            self.dirstack.pushdir(self.builddir)
            try:
                self.dobulk()
            except Exception, e:
                self.log("A fatal exception occurred: " + str(e))
            self.dirstack.popdir()
            self.purgetransient()
        finally:
            self.log("END: " + self.timenow())
            self.closelog()

tool = Snapshot()
tool.doall()
