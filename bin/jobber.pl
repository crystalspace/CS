#!/usr/bin/perl -w
#==============================================================================
#
#    Automated Processing, Publishing, and CVS Update Script
#    Copyright (C) 2000-2004 by Eric Sunshine <sunshine@sunshineco.com>
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
# jobber.pl
#
#    A tool for automatically processing tasks based upon the content of a
#    project's CVS repository.  This tool performs several tasks:
#
#        - Extracts the Crystal Space project from the CVS repository.
#        - Repairs broken @node directives and @menu blocks in Texinfo files.
#        - Converts Texinfo documentation to HTML format.
#        - Builds the public API reference manual.
#        - Builds the developer's API reference manual.
#        - Re-builds Visual-C++ DSW/DSP and SLN/VCPROJ project files.
#        - Re-builds the Swig-generated Python and Perl5 files.
#        - Commits all changes to the CVS repository.
#        - Makes all generated HTML available online for browsing.
#        - Makes archives of the generated HTML available for download.
#
#    This script takes special care to invoke the appropriate CVS commands to
#    add new files and directories to the repository, and to remove obsolete
#    files.  It also correctly handles binary files (such as images) by
#    committing them to the repository with the "-kb" CVS flag.
#
#    Typically this script is run via the 'cron' daemon.  See the cron(8),
#    crontab(1), and crontab(8) man pages for information specific to cron.
#    A typical crontab entry which runs this script at 01:03 each morning
#    might look like this:
#
#        MAILTO = sunshine@sunshineco.com
#        3 1 * * * $HOME/bin/jobber.pl
#
#    The script makes no attempt to perform any sort of CVS authentication.
#    Currently, it is the client's responsibility to authenticate with the CVS
#    server if necessary.  For :pserver: access the easiest way to work around
#    this limitation is to login to the CVS server one time manually using the
#    appropriate identity.  Once logged in successfully, the authentication
#    information is stored in $(HOME)/.cvspass and remains there.  From that
#    point onward, CVS considers the account as having been authenticated.
#    Other authentication schemes such as rsh or ssh will also work if
#    correctly configured.  The identity used for CVS access must have "write"
#    access to the repository.
#
# To-Do List
#    * Generalize into a "job" processing mechanism.  Each job should reside
#      within its own source file.  There can be jobs to check out files from
#      CVS, run the various `make' commands (make manualhtml, make repairdoc,
#      make msvc7gen, etc.), and perform the comparision and commit of
#      generated files.
#    * The mechanism used to publish packages for download and online browsing
#      needs to be generalized further.  It is still somewhat geared toward
#      the publication of documentation packages and is, thus, not flexible
#      enough to publish packages which do not follow the directory structure
#      designed for documentation.
#    * Eliminate the restriction in which only the first directory listed by
#      the "newdirs" array is exported by the "exports" key.
#
#------------------------------------------------------------------------------
use Carp;
use File::Basename;
use File::Copy;
use File::Find;
use File::Path;
use FileHandle;
use Getopt::Long;
use POSIX 'tmpnam';
use strict;
$Getopt::Long::ignorecase = 0;

my $PROG_NAME = 'jobber.pl';
my $PROG_VERSION = '30';
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000-2004 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

#------------------------------------------------------------------------------
# Configuration Section
#    PROJECT_ROOT - Root directory of the project.  This is the top-level
#        directory created as a side-effect of retrieving the files from CVS.
#    CVSROOT - The CVSROOT setting used for invoking CVS commands.  The
#        specified value must allow "write" access to the repository.
#    CVS_SOURCES - All directories which need to be extracted from the
#        repository in order to perform the conversion process.  This list
#        should include the documentation directories as well as those
#        containing tools and supporting files used in the conversion process.
#    CVSFLAGS - Additional flags to pass to each of the `cvs' command
#        invocations.  An obvious example would be to set this variable to
#        "-z9" to enable compression.
#    BROWSEABLE_DIR - Directory into which generated documentation should be
#        copied for online browsing.
#    PACKAGE_DIR - Directory into which archives of generated documentation
#        are placed to make them available for download in package form.
#    OWNER_GROUP - Group to which to assign all directories which will exist
#        after script's termination (such as the "browseable" directory).  May
#        be 'undef' if no special group should be assigned.
#    TEMPDIR - The temporary working directory where all processing should
#        occur.  The script cleans up after itself, so nothing will be left in
#        this directory after the script terminates.
#    BINARY - A list of regular expressions which are matched against the names
#        of files which are being added to the CVS repository.  If a filename
#        matches one of these expressions, then it is added to the repository
#        with the "-kb" CVS flag.  Although binary files are automatically
#        given the "-kb" flag, the BINARY list acts as a supplement for those
#        cases when a file appears to be text but should really be handled as
#        binary.  This is useful, for instance, for Visual-C++ DSW and DSP
#        project files in which the CRLF line-terminator must be preserved.
#    TARGETS - A list of target tasks.  Each target represents a task which
#        should be performed to generate some set of output files.  These files
#        can then be optionally committed back to the CVS repository and/or
#        published for browsing or download.  Each target record is a
#        dictionary which contains the following keys.  The key "name" is the
#        human-readable name for this task and is used in status messages.  The
#        key "action" is a human-readable verb which describes the action
#        performed by this target.  It is combined with the value of the "name"
#        key to construct an informative diagnositc message.  The key "build"
#        is the build command which is invoked to generate this particular set
#        of files.  The key "newdirs" refers to an array of directory names
#        into which files are generated for this target.  This key should only
#        be present if new files are created by this target.  The key "olddirs"
#        refers to an array of existing directories where these files are
#        stored in the CVS repository.  If "newdirs" is omitted, then the
#        directories mentioned by "olddirs" are those containing files modified
#        in-place by the build command, rather than generated anew in a
#        different location.  If both "newdirs" and "olddirs" are present, then
#        entries in "newdirs" must correspond to entries in "olddirs", and each
#        directory in "newdirs" must exactly mirror the layout and hierarchy of
#        each corresponding directory in "olddirs".  The key "log" is the log
#        message to use for CVS transactions involving this target.  The keys
#        "olddirs" and "log" should only be present if the files generated by
#        this target should be committed back into the CVS repository.  The key
#        "export" should refer to a sub-dictionary which describes how to
#        export the target.  The "export" key should only be present if the
#        files generated by this target should be published for browsing and
#        downloading.  Within the "export" dictionary, the key "dir" is the
#        directory name within both BROWSEABLE_DIR and PACKAGE_DIR into which
#        the files for this target are published, and the key "name" is the
#        base package name used when generating downloadable packages via
#        ARCHIVERS (below).  When published, the base package name is combined
#        with the archiver's file extension and placed within the appropriate
#        subdirectory of PACKAGE_DIR.  Presently, the implementation is limited
#        to only exporting the first directory referenced by the sibling
#        "newdirs" key.  The key "appear" controls the appearance of the
#        directory in the generated package.  For example, when packaging the
#        directory "out/docs/html/manual", it should actually appear as
#        "CS/docs/html/manual" in the downloadable package.  The
#        "browseable-postprocess" key allows specification of a post-processing
#        step for the documentation which is being made available for online
#        browsing.  The value of this key is any valid shell command.  If the
#        meta-token ~T appears in the command, the path of the directory which
#        is being published is interpolated into the command in its place.
#    ARCHIVERS - A list of archiver records.  Each arechiver is used to
#        generate a package from an input directory.  Each archiver record is
#        a dictionary which contains the following keys.  The key "name"
#        specifies the archiver's printable name.  The key "ext" is the file
#        extension for the generated archive file.  The key "cmd" is the
#        actual command template which describes how to generate the given
#        archive.  The template may contain the meta-token ~S and ~D.  The
#        name of the source directory is interpolated into the command in
#        place of ~S, and the destination package name is interpolated in
#        place of ~D.
#------------------------------------------------------------------------------

# For write-access, SourceForge requires SSH access.
$ENV{'CVS_RSH'} = 'ssh';

# Ensure that Doxygen, Swig, and Jam can be found.
$ENV{'PATH'} =
    '/home/groups/c/cr/crystal/bin:' .
    '/home/groups/c/cr/crystal/swig/bin:' .
    $ENV{'PATH'} .
    ':/usr/local/bin';

# This is just plain ugly.  The SourceForge CVS server is so mis-configured
# that it drops the connection any time the (admittedly) large Swig-generated
# CS/scripts/perl5/cswigpl5.inc is committed to the repository even though this
# script is running on a local SourceForge machine.  (This also happens with
# other files, but seems most frequent with the large ones, such as
# cswigpl5.inc.) To work around the problem, we enable compression in order
# send less information to the CVS server, under the assumption that the huge
# file size is what is triggering the server to drop the connection.  Enabling
# compression helps to side-step the problem some of the time (which is better
# than nothing).
my $CVSFLAGS = '-z9';

my $PROJECT_ROOT = 'CS';
my $CVSUSER = 'sunshine';
my $CVSROOT = "$CVSUSER\@cvs1:/cvsroot/crystal";
my $CVS_SOURCES = $PROJECT_ROOT;
my $PUBLIC_DOC_DIR = '/home/groups/c/cr/crystal/htdocs/docs';
my $BROWSEABLE_DIR = "$PUBLIC_DOC_DIR/online";
my $PACKAGE_DIR = "$PUBLIC_DOC_DIR/download";
my $OWNER_GROUP = 'crystal';
my $TEMPDIR = '/home/groups/c/cr/crystal';
my @BINARY = ('(?i)\.(dsw|dsp)$');

my @TARGETS =
    ({ 'name'    => 'Visual-C++ DSW and DSP files',
       'action'  => 'Repairing',
       'build'    => 'jam msvc6gen',
       'newdirs' => ['out/mk/visualc6'],
       'olddirs' => ['mk/visualc6'],
       'log'     => 'Automated Visual-C++ DSW and DSP project file repair.' },
     { 'name'    => 'Visual-C++ SLN and VCPROJ files',
       'action'  => 'Repairing',
       'build'    => 'jam msvc7gen',
       'newdirs' => ['out/mk/visualc7'],
       'olddirs' => ['mk/visualc7'],
       'log'     =>
	   'Automated Visual-C++ SLN and VCPROJ project file repair.'},
     { 'name'    => 'Texinfo files',
       'action'  => 'Repairing @node and @menu directives in',
       'build'    => 'jam repairdoc',
       'olddirs' => ['docs/texinfo'],
       'log'     => 'Automated Texinfo @node and @menu repair.' },
     { 'name'    => 'Swig Python files',
       'action'  => 'Repairing',
       'build'    => 'jam pythonfreeze',
       'olddirs' => ['plugins/cscript/cspython', 'scripts/python'],
       'log'     => 'Automated Swig Python file repair.' },
     { 'name'    => 'Swig Perl5 files',
       'action'  => 'Repairing',
       'build'    => 'jam perl5freeze',
       'olddirs' => ['scripts/perl5'],
       'log'     => 'Automated Swig Perl5 file repair.' },
     { 'name'    => 'User\'s Manual',
       'action'  => 'Generating',
       'build'    => 'jam manualhtml',
       'newdirs' => ['out/docs/html/manual'],
       'olddirs' => ['docs/html/manual'],
       'log'     => 'Automated Texinfo to HTML conversion.',
       'export'  =>
	   { 'dir'    => 'manual',
	     'name'   => 'csmanual-html',
	     'appear' => "$PROJECT_ROOT/docs/html/manual",
	     'browseable-postprocess' =>
	         'sh docs/support/annotate/transform.sh ~T' }},
     { 'name'    => 'Public API Reference',
       'action'  => 'Generating',
       'build'    => 'jam apihtml',
       'newdirs' => ['out/docs/html/api'],
       'olddirs' => ['docs/html/api'],
       'log'     => 'Automated API reference generation.',
       'export'  =>
	   { 'dir'    => 'api',
	     'name'   => 'csapi-html',
	     'appear' => "$PROJECT_ROOT/docs/html/api",
	     'browseable-postprocess' =>
	         'sh docs/support/annotate/transform.sh ~T' }},
#      { 'name'    => 'Developer API Reference',
#        'action'  => 'Generating',
#        'build'    => 'jam apidevhtml',
#        'newdirs' => ['out/docs/html/apidev'],
#        'export'  =>
# 	   { 'dir'    => 'apidev',
# 	     'name'   => 'csapidev-html',
# 	     'appear' => "$PROJECT_ROOT/docs/html/apidev",
# 	     'browseable-postprocess' =>
# 	         'sh docs/support/annotate/transform.sh ~T' }}
     );

my @ARCHIVERS =
    (
#    { 'name' => 'gzip',
#      'ext'  => 'tgz',
#      'cmd'  => 'tar --create --file=- ~S | gzip > ~D' },
     { 'name' => 'bzip2',
       'ext'  => 'tar.bz2',
       'cmd'  => 'tar --create --file=- ~S | bzip2 > ~D' },
#    { 'name' => 'zip',
#      'ext'  => 'zip',
#      'cmd'  => 'zip -q -r ~D ~S' }
    );

#------------------------------------------------------------------------------
# Internal configuration.
#------------------------------------------------------------------------------
my $TESTING = undef;
my $CONV_DIR = temporary_name($TEMPDIR);
my $CAPTURED_OUTPUT = '';

# In order to invoke the various actions performed by this script, the project
# needs to be configured, however the SourceForge shell machine does not have
# compilers installed, thus we can not configure the project with the Autoconf
# "configure" script.  Instead, we fake up project configuration by creating a
# minimal Jamconfig and Jamfile.
my $CONFIGURE = 
    "csver=`./configure --version | awk '/crystal/ { print \$3 }'`\n" .
    "csverlist=`echo \$csver | tr '.' ' '`\n" .
    "cscopy=`sed '/\\[[Cc]opyright/!d;s/[^[]*\\[\\([^]]*\\)\\].*/\\1/'" .
    "  < configure.ac`\n" .
    "cat << EOF > Jamconfig\n" .
    "CMD.C++ ?= \"g++\" ;\n" .
    "CMD.CC ?= \"gcc\" ;\n" .
    "CMD.DOXYGEN ?= \"doxygen\" ;\n" .
    "CMD.DVIPS ?= \"dvips\" ;\n" .
    "CMD.LINK ?= \"\\\$(CMD.C++)\" ;\n" .
    "CMD.MAKEINFO ?= \"makeinfo\" ;\n" .
    "CMD.MKDIR ?= \"mkdir\" ;\n" .
    "CMD.MKDIRS ?= \"mkdir -p\" ;\n" .
    "CMD.OBJC ?= \"gcc\" ;\n" .
    "CMD.OBJC++ ?= \"g++\" ;\n" .
    "CMD.SWIG ?= \"swig\" ;\n" .
    "CMD.TEXI2DVI ?= \"texi2dvi\" ;\n" .
    "JAM ?= \"jam\" ;\n" .
    "MODE ?= \"optimize\" ;\n" .
    "MSVCGEN_SILENT ?= \"yes\" ;\n" .
    "PACKAGE_COPYRIGHT ?= \"\$cscopy\" ;\n" .
    "PACKAGE_HOMEPAGE ?= \"http://crystal.sourceforge.net/\" ;\n" .
    "PACKAGE_LONGNAME ?= \"Crystal Space\" ;\n" .
    "PACKAGE_NAME ?= \"crystal\" ;\n" .
    "PACKAGE_STRING ?= \"crystal \$csver\" ;\n" .
    "PACKAGE_VERSION ?= \"\$csver\" ;\n" .
    "PACKAGE_VERSION_LIST ?= \$csverlist ;\n" .
    "PERL ?= \"perl\" ;\n" .
    "PERL5.AVAILABLE ?= \"yes\" ;\n" .
    "PYTHON ?= \"python\" ;\n" .
    "PYTHON.AVAILABLE ?= \"yes\" ;\n" .
    "TARGET.OS ?= \"UNIX\" ;\n" .
    "EOF\n" .
    "sed 's/\@top_srcdir\@/./;s/\@top_builddir\@//' < Jamfile.in > Jamfile\n";

my @SCRIPT_OPTIONS = (
    'test!'     => \$TESTING,
    'help'      => \&option_help,
    'version|V' => \&option_version,
    '<>'        => \&option_error
);

my @NEW_DIRECTORIES = ();
my @NEW_TEXT_FILES = ();
my @NEW_BINARY_FILES = ();
my @OUTDATED_FILES = ();

#------------------------------------------------------------------------------
# Terminate abnormally and print a textual representation of "errno".
#------------------------------------------------------------------------------
sub expire {
    my $msg = shift;
    print STDERR "FATAL: $msg failed: $!\n";
    dump_captured();
    destroy_transient($CONV_DIR);
    croak 'Stopped';
}

#------------------------------------------------------------------------------
# Convert a list of pathnames into a string which can be passed to a shell
# command via the command-line.  Also protect special characters from the
# shell by escaping them.
#------------------------------------------------------------------------------
sub prepare_pathnames {
    my ($path, @paths);
    foreach $path (@_) {
	$path =~ s/ /\\ /g;
	push(@paths, $path);
    }
    return join(' ', @paths);
}

#------------------------------------------------------------------------------
# Is file a "binary" file?  (Should the CVS flag "-kb" be used?)
# First check the filename against the special-case @BINARY array, then check
# the file's content if necessary.
#------------------------------------------------------------------------------
sub is_binary {
    my $path = shift;
    foreach my $pattern (@BINARY) {
	return 1 if $path =~ /$pattern/;
    }
    return -B $path;
}

#------------------------------------------------------------------------------
# Change group ownership on a list of directories.
#------------------------------------------------------------------------------
sub change_group {
    my $group = shift;
    my @dirs = @_;
    return unless $group && @dirs;
    my $gid = getgrnam($group) or expire("getgrnam($group)");
    chown(-1, $gid, @dirs) or expire("chown(-1, $gid, ".join(' ', @dirs).')');
    chmod(0775, @dirs) or expire("chmod(0775, ".join(' ', @dirs).')');
}


#------------------------------------------------------------------------------
# Change group ownership of directories and all subdirectories (recursive).
#------------------------------------------------------------------------------
sub change_group_deep {
    my $group = shift;
    my @dirs = @_;
    my @children = ();
    find(sub{my $n=$File::Find::name; push(@children, $n) if -d $n}, @dirs);
    change_group($group, @dirs, @children);
}

#------------------------------------------------------------------------------
# Create a directory.
#------------------------------------------------------------------------------
sub create_directory {
    my ($dir, $group) = @_;
    mkdir($dir, 0755) or expire("mkdir($dir)");
    change_group($group, $dir);
}

#------------------------------------------------------------------------------
# Create a directory and all intermediate directories.
#------------------------------------------------------------------------------
sub create_directory_deep {
    my ($dir, $group) = @_;
    my @dirs = mkpath($dir);
    change_group($group, @dirs);
}

#------------------------------------------------------------------------------
# Change the working directory.
#------------------------------------------------------------------------------
sub change_directory {
    my $dir = shift;
    chdir($dir) or expire("chdir($dir)");
}

#------------------------------------------------------------------------------
# Copy a file.
#------------------------------------------------------------------------------
sub copy_file {
    my ($src, $dst) = @_;
    copy($src, $dst) or expire("copy($src,$dst)");
}

#------------------------------------------------------------------------------
# Remove a file.
#------------------------------------------------------------------------------
sub remove_file {
    my $file = shift;
    unlink($file) or expire("unlink($file)");
}

#------------------------------------------------------------------------------
# Rename a file.
#------------------------------------------------------------------------------
sub rename_file {
    my ($src, $dst) = @_;
    rename($src, $dst) or expire("rename($src,$dst)");
}

#------------------------------------------------------------------------------
# Generate a temporary name in a directory.  Perl tmpnam() only works with
# '/tmp', so must do this manually, instead.
#------------------------------------------------------------------------------
sub temporary_name {
    my ($dir, $prefix, $suffix) = @_;
    $prefix = 'tmp' unless $prefix;
    $suffix = '' unless $suffix;
    my ($i, $limit) = (0, 100);
    $i++ while -e "$dir/$prefix$i$suffix" && $i < $limit;
    expire("temporary_name($dir,$prefix,$suffix)") if $i >= $limit;
    return "$dir/$prefix$i$suffix";
}

#------------------------------------------------------------------------------
# Run an external shell command.
#------------------------------------------------------------------------------
sub run_command {
    my $cmd = shift;
    my $output = `$cmd 2>&1`;
    $CAPTURED_OUTPUT .= "==> $cmd\n$output\n";
    expire("run_command($cmd)") if $?;
    return $output;
}

#------------------------------------------------------------------------------
# Perform a recursive scan of a directory and return a sorted list of all
# files and directories contained therein, except for the "CVS" directory and
# its control files.  Also ignores ".cvsignore" files.
#------------------------------------------------------------------------------
sub scandir {
    my $dir = shift;
    my @files;
    find(sub{my $n=$File::Find::name; $n=~s|$dir/?||; push(@files,$n) if $n},
	 $dir);
    return sort(grep(!/.cvsignore/,grep(!/CVS/,@files)));
}

#------------------------------------------------------------------------------
# Apply the CVS `remove' command to a batch of files.
#------------------------------------------------------------------------------
sub cvs_remove {
    my $files = shift;
    return unless @{$files};
    my $paths = prepare_pathnames(@{$files});
    print "Invoking CVS remove: ${\scalar(@{$files})} paths\n";
    run_command("cvs -Q $CVSFLAGS remove $paths") unless $TESTING;
}

#------------------------------------------------------------------------------
# Queue an entry for removal from the CVS repository if it does not exist in
# the newly generated directory hierarchy.  Note that for directories, no
# action is taken since CVS automatically removes empty directories on the
# client side when the "-P" switch is used with the CVS "update" command.
#------------------------------------------------------------------------------
sub cvs_queue_remove {
    my $dst = shift;
    my $file = basename($dst);
    if (-d $dst) {
	print "Pruning directory: $file\n";
    }
    else {
	print "Removing file: $file\n";
	remove_file($dst);
	push(@OUTDATED_FILES, $dst);
    }
}

#------------------------------------------------------------------------------
# Apply the CVS `add' command to a batch of files.  Allows specification of
# extra CVS flags (such as `-kb').
#------------------------------------------------------------------------------
sub cvs_add {
    my ($files, $flags) = @_;
    return unless @{$files};
    my $paths = prepare_pathnames(@{$files});
    $flags = '' unless defined($flags);
    print "Invoking CVS add: ${\scalar(@{$files})} paths" .
	($flags ? " [$flags]" : '') . "\n";
    run_command("cvs -Q $CVSFLAGS add $flags $paths") unless $TESTING;
}

#------------------------------------------------------------------------------
# Queue a file or directory from the newly generated directory hierarchy for
# addition to the CVS repository.  Takes special care to use the "-kb" CVS flag
# when adding a binary file (such as an image) to the repository.
#------------------------------------------------------------------------------
sub cvs_queue_add {
    my ($src, $dst) = @_;
    my $file = basename($dst);
    if (-d $src) {
	print "Adding directory: $file\n";
	create_directory($dst);
	push(@NEW_DIRECTORIES, $dst);
    }
    else {
	my $isbin = is_binary($src);
	print "Adding file: $file [", ($isbin ? 'binary' : 'text'), "]\n";
	push(@NEW_TEXT_FILES, $dst) unless $isbin;
	push(@NEW_BINARY_FILES, $dst) if $isbin;
	copy_file($src, $dst);
    }
}

#------------------------------------------------------------------------------
# File exists in both existing CVS repository and in newly generated directory
# hierarchy.  Overwrite the existing file with the newly generated one.
# Later, at commit time, CVS will determine if any changes to the file have
# actually taken place.
#------------------------------------------------------------------------------
sub cvs_examine {
    my ($src, $dst) = @_;
    if (!-d $src) {
	remove_file($dst);
	copy_file($src, $dst);
    }
}

#------------------------------------------------------------------------------
# Extract the appropriate files from the CVS repository.
#------------------------------------------------------------------------------
sub cvs_checkout {
    print "CVSROOT: $CVSROOT\nExtracting: $CVS_SOURCES\n";
    run_command("cvs -Q $CVSFLAGS -d $CVSROOT checkout -P $CVS_SOURCES");
}

#------------------------------------------------------------------------------
# Print a summary list of files which were modified, added, or removed.
#------------------------------------------------------------------------------
sub cvs_update {
    my $message = 'Modification summary:';
    my $line = '-' x length($message);
    my $dirs = '';
    foreach my $target (@TARGETS) {
	$dirs .= " @{$target->{'olddirs'}}" if exists $target->{'olddirs'};
    }
    print "$line\n$message\n", run_command("cvs -q $CVSFLAGS update $dirs"),
        "$line\n" if $dirs;
}

#------------------------------------------------------------------------------
# Commit files to the CVS repository.  The 'cvs' command is smart enough to
# only commit files which have actually changed.
#------------------------------------------------------------------------------
sub cvs_commit_dirs {
    my ($dirs, $message) = @_;
    run_command("cvs -Q $CVSFLAGS commit -m \"$message\" @{$dirs}")
	unless $TESTING;
}

#------------------------------------------------------------------------------
# Commit repaired and generated files to the CVS repository.
#------------------------------------------------------------------------------
sub cvs_commit {
    foreach my $target (@TARGETS) {
	if (exists $target->{'olddirs'}) {
	    print "Committing $target->{'name'}.\n";
	    cvs_commit_dirs($target->{'olddirs'}, $target->{'log'});
	}
    }
}

#------------------------------------------------------------------------------
# Perform all tasks by invoking the appropriate build command of each task.
# Note that the build system is first configured before running the commands
# since they will not work prior to configuration.
#------------------------------------------------------------------------------
sub run_tasks {
    print "Configuring project.\n";
    run_command($CONFIGURE);
    foreach my $target (@TARGETS) {
	print "$target->{'action'} $target->{'name'}.\n";
	run_command($target->{'build'});
    }
}

#------------------------------------------------------------------------------
# Scan and compare the newly generated directory hierarchies against existing
# hierarchies from the CVS repository.  For each difference between the two
# hierarchies, apply the appropriate CVS operation, adding or removing entries
# as necessary.
#------------------------------------------------------------------------------
sub apply_diffs {
    foreach my $target (@TARGETS) {
	next unless
	    exists $target->{'olddirs'} and exists $target->{'newdirs'};
	print "Applying changes to $target->{'name'}.\n";

	my @olddirs = @{$target->{'olddirs'}};
	my @newdirs = @{$target->{'newdirs'}};
	foreach my $olddir (@olddirs) {
	    my $newdir = shift @newdirs;
    
	    print "Scanning directories ($olddir <=> $newdir).\n";
	    my @oldfiles = scandir($olddir);
	    my @newfiles = scandir($newdir);
	
	    print "Comparing directories ($olddir <=> $newdir).\n";
	    my $oldfile = shift @oldfiles;
	    my $newfile = shift @newfiles;
	
	    while (defined($oldfile) || defined($newfile)) {
		if (!defined($newfile) ||
		    (defined($oldfile) && $oldfile lt $newfile)) {
		    cvs_queue_remove("$olddir/$oldfile");
		    $oldfile = shift @oldfiles;
		}
		elsif (!defined($oldfile) || $oldfile gt $newfile) {
		    cvs_queue_add("$newdir/$newfile", "$olddir/$newfile");
		    $newfile = shift @newfiles;
		}
		else { # Filenames are identical.
		    cvs_examine("$newdir/$newfile", "$olddir/$newfile");
		    $oldfile = shift @oldfiles;
		    $newfile = shift @newfiles;
		}
	    }
	}
    }
    cvs_add(\@NEW_DIRECTORIES);
    cvs_add(\@NEW_TEXT_FILES);
    cvs_add(\@NEW_BINARY_FILES, '-kb');
    cvs_remove(\@OUTDATED_FILES);
}

#------------------------------------------------------------------------------
# Interpolate a value into a string in place of a token.
#------------------------------------------------------------------------------
sub interpolate {
    local $_ = $_[0];
    my ($token, $value) = @_[1..2];
    s/$token/$value/g or expire("Interpolation of $token in $_");
    $_[0] = $_;
}

#------------------------------------------------------------------------------
# Post-process a browseable directory if requested.
#------------------------------------------------------------------------------
sub postprocess_browseable {
    my ($export, $dir) = @_;
    return unless exists $export->{'browseable-postprocess'};
    print "Post-processing.\n";

    my $cmd = $export->{'browseable-postprocess'};
    interpolate($cmd, '~T', $dir);
    run_command($cmd);
}

#------------------------------------------------------------------------------
# Publish a browseable copy of the generated files.
# FIXME: Presently, only publishes first directory referenced by "newdirs" key.
#------------------------------------------------------------------------------
sub publish_browseable {
    create_directory_deep($BROWSEABLE_DIR, $OWNER_GROUP);
    foreach my $target (@TARGETS) {
	next unless exists $target->{'export'};
	print "Publishing $target->{'name'}.\n";
	next if $TESTING;
    
	my @srclist = @{$target->{'newdirs'}};
	my $src = shift @srclist;		# See FIXME above.
	my $dst = "$BROWSEABLE_DIR/$target->{'export'}->{'dir'}";
	my $new_dir = temporary_name($BROWSEABLE_DIR, 'new');
	my $old_dir = temporary_name($BROWSEABLE_DIR, 'old');

	print "Preparing.\n";
	run_command("cp -r \"$src\" \"$new_dir\"");
	change_group_deep($OWNER_GROUP, "$new_dir");
	postprocess_browseable($target->{'export'}, $new_dir);
    
	print "Installing.\n";
	rename_file($dst, $old_dir) if -e $dst;
	rename_file($new_dir, $dst);
    
	print "Cleaning.\n";
	rmtree($old_dir);
    }
}

#------------------------------------------------------------------------------
# Publish an archive of the generated files.
#------------------------------------------------------------------------------
sub publish_package {
    my ($archiver, $src, $dst, $base) = @_;
    print "Packaging: $archiver->{'name'}\n";
    return if $TESTING;
    my $ext = $archiver->{'ext'};
    my $tmp_pkg = temporary_name($dst, 'pkg', ".$ext");
    my $package = "$dst/$base.$ext";
    my $cmd = $archiver->{'cmd'};
    interpolate($cmd, '~S', $src);
    interpolate($cmd, '~D', $tmp_pkg);
    run_command($cmd);
    rename_file($tmp_pkg, $package);
}

#------------------------------------------------------------------------------
# Publish generated directory hierarchies as archives of various formats as
# indicated by the ARCHIVERS array.  The 'appear' key in the 'export'
# dictionary of the TARGETS array, is used to control how the packaged
# directory appears in the archive.  For instance, although the directory
# 'out/docs/html/manual' may be packaged, the 'appear' key may indicate that it
# should appear as 'CS/docs/html/manual' in the generated package.  This
# functionality is handled by temporarily giving the target directory the
# desired name just prior to archiving.  Note that during this operation, the
# current working directory is CONV_DIR/PROJECT_ROOT, and all operations are
# performed relative to this location.
# FIXME: Presently, only publishes first directory referenced by "newdirs"
# and/or "olddirs" key.
#------------------------------------------------------------------------------
sub publish_packages {
    foreach my $target (@TARGETS) {
	next unless exists $target->{'export'}
	    and (exists $target->{'olddirs'} or exists $target->{'newdirs'});
	print "Packaging $target->{'name'}.\n";

	my $export = $target->{'export'};
	my $appear = $export->{'appear'};

	my @srclist = ();
	if (exists $target->{'olddirs'}) {
	    @srclist = @{$target->{'olddirs'}};
	}
	else {
	    @srclist = @{$target->{'newdirs'}};
	}
	my $src = shift @srclist; # See FIXME above.

	create_directory_deep(dirname($appear));
	rename_file($src, $appear); # Sleight-of-hand (magic).

	my $base = $export->{'name'};
	my $dst = "$PACKAGE_DIR/$export->{'dir'}";
	create_directory_deep($dst, $OWNER_GROUP);
	foreach my $archiver (@ARCHIVERS) {
	    publish_package($archiver, $appear, $dst, $base);
	}
	rename_file($appear, $src); # Unmagic.
    }
}

#------------------------------------------------------------------------------
# Create the temporary working directory.
#------------------------------------------------------------------------------
sub create_transient {
    my $dir = shift;
    print "Temporary directory: $dir\n";
    create_directory($dir);
    change_directory($dir);
}

#------------------------------------------------------------------------------
# Remove the temporary working directory.
#------------------------------------------------------------------------------
sub destroy_transient {
    my $dir = shift;
    print "Purging temporary directory.\n";
    change_directory('/');
    rmtree($dir);
}

#------------------------------------------------------------------------------
# Return a canonical representation of the current time.
#------------------------------------------------------------------------------
sub time_now {
    return gmtime() . ' UTC';
}

#------------------------------------------------------------------------------
# Perform the complete process of running tasks, committing to CVS, and
# publishing packages.
#------------------------------------------------------------------------------
sub run {
    print 'BEGIN: ', time_now(), "\n";
    create_transient($CONV_DIR);
    cvs_checkout();
    change_directory($PROJECT_ROOT);
    run_tasks();
    apply_diffs();
    cvs_update();
    cvs_commit();
    publish_browseable();
    publish_packages();
    destroy_transient($CONV_DIR);
    print 'END: ', time_now(), "\n";
}

#------------------------------------------------------------------------------
# Dump output captured from shell commands.
#------------------------------------------------------------------------------
sub dump_captured {
    print "\nCaptured output:\n\n$CAPTURED_OUTPUT";
}

#------------------------------------------------------------------------------
# Display an opening banner.
#------------------------------------------------------------------------------
sub banner {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    print $stream "\n$PROG_NAME version $PROG_VERSION\n$COPYRIGHT\n\n";
}

#------------------------------------------------------------------------------
# Display usage statement.
#------------------------------------------------------------------------------
sub print_usage {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    banner($stream);
    print $stream <<EOT;
This program performs a series of tasks to generate and repair files, commit
modified files to the CVS repository, and publish the generated files for
browsing and download.  It should be run on a periodic basis, typically by
an automated mechanism.  The tasks which it performs are:

    o Repairs out-of-date Visual-C++ DSW/DSP and SLN/VCPROJ project files.
    o Repairs out-of-date Swig-generated Python and Perl5 files.
    o Repairs out-of-date and broken \@node directives and \@menu blocks
      in Texinfo files.
    o Converts the User's Manual Texinfo source files to HTML format.
    o Generates the Public API Reference manual in HTML format.
    o Generates the Developer's API Reference manual in HTML format.
    o Commits all changed files back into the CVS repository.
    o Publishes all HTML files for online browsing.
    o Creates and publishes packages from HTML files for download.

Usage: $PROG_NAME [options]

Options:
    -t --test    Process all tasks but do not actually modify the CVS
                 repository or export any files.
    -h --help    Display this usage message.
    -v --version Display the version number of @{[basename($0)]}

EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@SCRIPT_OPTIONS) or usage_error('');
    banner();
    print "Non-destructive testing mode enabled.\n\n" if $TESTING;
}

sub option_help    { print_usage(\*STDOUT); exit(0); }
sub option_version { banner(\*STDOUT); exit(0); }
sub option_error   { usage_error("Unknown option: @_\n"); }

sub usage_error {
    my $msg = shift;
    print STDERR "ERROR: $msg\n" if $msg;
    print_usage(\*STDERR);
    exit(1);
}

#------------------------------------------------------------------------------
# Run the conversion.
#------------------------------------------------------------------------------
process_options();
run();
dump_captured();
