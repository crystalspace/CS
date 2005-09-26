#!/usr/bin/perl -w
#==============================================================================
#
#    Automated Task Processing, Publishing, and CVS Update Script
#    Copyright (C) 2000-2005 by Eric Sunshine <sunshine@sunshineco.com>
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
# A generalized tool for automatically performing maintenance tasks based upon
# the contents of a project's CVS repository.  Tasks and configuration details
# are specified via an external configuration file. The overall operation of
# jobber.pl follows these steps:
#
# (1) Extract project tree from CVS repository.
# (2) Perform a set of tasks upon the extracted project tree.
# (3) Commit changed files back to the repository.
# (4) Optionally publish project documentation for online browsing and
#     download.
#
# Some common tasks for which jobber.pl may be called upon include:
#
# * Reparing outdated resources which can be generated automatically from
#   other project content.
# * Generating project files for various build tools.
# * Generating documentation in various formats from input files, such as
#   Texinfo or header files.
#
# This script takes special care to invoke the appropriate CVS commands to add
# new files and directories to the repository, and to remove obsolete files.
# It also correctly handles binary files (such as images) by committing them to
# the repository with the "-kb" CVS flag.
#
# Typically this script is run via the 'cron' daemon.  See the cron(8),
# crontab(1), and crontab(8) man pages for information specific to cron.  A
# typical crontab entry which runs this script at 02:13 each morning might look
# like this:
#
#     MAILTO = sunshine@sunshineco.com
#     13 2 * * * $HOME/bin/jobber.pl
#
# The script makes no attempt to perform any sort of CVS authentication.  It is
# the client's responsibility to authenticate with the CVS server if necessary.
# For :pserver: access the easiest way to do so is to login to the CVS server
# one time manually using the appropriate identity.  Once logged in
# successfully, the authentication information is stored in $(HOME)/.cvspass
# and remains there.  From that point onward, CVS considers the account as
# having been authenticated.  Other authentication schemes such as rsh or ssh
# will also work if correctly configured.  The identity used for CVS access
# must have "write" access to the repository if files changed by the performed
# tasks are to be committed back to the repository.
#
#------------------------------------------------------------------------------
#
# At run-time, jobber.pl relies upon an external configuration file which
# specifies the list of tasks to be performed, for settings controlling access
# to the CVS repository, and for indicating where documentation should be
# published.  The configuration file may be specified via --config command-line
# option (i.e. "--config myjobber.cfg"). If it is not specified via --config,
# then jobber.pl looks for a file named jobber.cfg or .jobber, first in the
# current working directory, and then in the home directory ($HOME). If no
# configuration file is found, the script aborts.
#
# At the command-line, jobber.pl allows users to define arbitrary run-time
# properties via the --set option. These properties can be accessed within the
# configuration file by consulting the %jobber_properties hash. For instance,
# the command-line argument "--set foo=bar" sets "bar" as the value of
# $jobber_properties{foo}.
#
# The following Perl variables can be set in the configuration file:
#
# $jobber_project_root [required]
#     Root directory of the project.  This is the top-level directory created
#     as a side-effect of retrieving the files from CVS. No default.
#
# $jobber_cvs_root [required]
#     The CVSROOT setting used for invoking CVS commands.  The specified value
#     must allow "write" access to the repository if files are to be committed
#     back to the repository. No default.
#
# $jobber_cvs_sources [optional]
#     All directories which need to be extracted from the repository in order
#     to perform the conversion process.  This list should include the
#     documentation directories as well as those containing tools and
#     supporting files needed by the tasks in the $jobber_tasks list.  This
#     list may contain CVS module aliases since it is used with the CVS
#     'checkout' command (i.e. "cvs checkout -P $jobber_cvs_sources").
#     Default: $jobber_project_root
#
# $jobber_cvs_flags [optional]
#     Additional flags to pass to each of the `cvs' command invocations.  An
#     obvious example would be to set this variable to "-z9" to enable
#     compression. No default.
#
# $jobber_browseable_dir [conditional]
#     Absolute path of directory into which generated documentation should be
#     copied for online browsing. This setting is required if any tasks publish
#     documentation, otherwise it is optional. No default.
#
# $jobber_package_dir [conditional]
#     Absolute path of directory into which archives of generated documentation
#     are placed to make them available for download in package form.  This
#     setting is required if any tasks publish documentation, otherwise it is
#     optional.  No default.
#
# $jobber_public_group [optional]
#     Group name to which to assign ownership of all directories which will
#     exist after script's termination (such as the "browseable" directory).
#     May be 'undef' if no special group should be assigned. Default: undef
#
# $jobber_public_mode [optional]
#     Mode to which to assign all directories which will exist after script's
#     termination (such as the "browseable" directory).  Use this in
#     conjunction with $jobber_public_group to make directories group-writable,
#     for example. For this purpose, set it to the octal value 0775.  May be
#     'undef' if no special mode should be assigned. Default: undef
#
# $jobber_temp_dir [optional]
#     Absolute path of temporary working directory where all processing should
#     occur.  The script cleans up after itself, so nothing will be left in
#     this directory after the script terminates. Default: "/tmp"
#
# @jobber_binary_override [optional]
#     Normally, jobber.pl determines automatically whether files which it adds
#     to the repository are binary or text (CVS needs to know this
#     information).  There may be special cases, however, when text files need
#     to be treated as binary files. This setting is a list of regular
#     expressions which are matched against the names of files being added to
#     the CVS repository.  If a filename matches one of these expressions, then
#     it is considered binary (thus, the CVS "-kb" option is used).  An example
#     of when this comes in handy is when dealing with Visual-C++ DSW and DSP
#     project files in which the CRLF line-terminator must be preserved.
#     Default: .dsw and .dsp files
#
# @jobber_tasks [required]
#     A list of tasks to perform on the checked-out source tree.  Typical tasks
#     are those which repair outdated files, and those which generate
#     human-consumable documentation from various sources.  Files generated or
#     repaired by the tasks can then optionally be committed back to the CVS
#     repository and/or published for browsing or download. Each task's
#     "command" is invoked in the top-level directory of the project tree
#     ($jobber_project_root).
#
#     Many projects need to be "configured" before various tasks can be
#     performed (often by running some sort of configuration script). If this
#     true for your project, then your very first task should invoke the
#     command(s) which configure the project.
#
#     Each task record is a dictionary which contains the following keys:
#
#     name [required]
#         Human-readable name for this task; used in status messages.
#     action [required]
#         Human-readable verb which describes the action performed by this
#         target. It is combined with the value of the "name" key to construct
#         an informative diagnositc message.
#     command [optional]
#         The actual command which is invoked to perform this task. It may
#         repair outdated files or generate a set of files (such as HTML
#         documentation).
#     newdirs [optional]
#         Refers to an array of directories into which files are generated by
#         this task.  This key should only be present if new files are created
#         by this target.
#     olddirs [optional]
#         Refers to an array of existing directories where files generated by
#         this task are stored in the CVS repository.  If the "newdirs" key is
#         omitted, then the directories mentioned by "olddirs" are those
#         containing files modified in-place by the command, rather than
#         generated anew in a different location.  If both "newdirs" and
#         "olddirs" are present, then entries in "newdirs" must correspond to
#         entries in "olddirs", and each directory in "newdirs" must exactly
#         mirror the layout and hierarchy of each corresponding directory in
#         "olddirs".
#     log [optional]
#         Log message to use for CVS transactions involving this target.  The
#         keys "olddirs" and "log" should be present only if the files
#         generated by this target should be committed back into the CVS
#         repository.
#     export [optional]
#         Refers to a sub-dictionary which describes how to export the target.
#         This key should be present only if the files generated by the task
#         should be published for browsing and downloading.  If this key is
#         used, then one or both of "olddirs" and "newdirs" must also be
#         present.  The sub-dictionary referenced by the "export" entry may
#         contain the following keys:
#
#         dir [required]
#             Directory name into which files for this task are published.
#             Online browseable files are placed into
#             $jobber_browseable_dir/$dir, and downloadable packages are placed
#             into $jobber_package_dir/$dir.
#         name [required]
#             Base package name used when generating downloadable packages via
#             @jobber_archivers (see below).  When published, the base package
#             name is combined with the archiver's file extension and placed
#             within the appropriate subdirectory of $jobber_package_dir.
#             *NOTE* Presently, the implementation is limited to only exporting
#             the first directory referenced by the sibling "newdirs" key.
#         appear [optional]
#             Controls the appearance of the directory in the generated
#             package.  For example, when packaging files from a temporary
#             build directory named "./out/docs/html/manual", it might be
#             preferable if it actually appeared as "CS/docs/html/manual" in
#             the downloadable package.
#         browseable-postprocess [optional]
#             Allows specification of a post-processing step for documentation
#             which is being made available for online browsing.  The value of
#             this key is any valid shell command.  It is invoked after the
#             files have been copied to the browseable directory. If the
#             meta-token ~T appears in the command, then the path of the
#             directory into which the files have been published is
#             interpolated into the command in its place.
#
# @jobber_archivers [optional]
#     A list of archiver records.  An archiver is used to generate a package
#     from an input directory.  Each entry in this list is a dictionary which
#     contains the following keys:
#
#     name [required]
#         Specifies the archiver's printable name.
#     extension [required]
#         File extension for the generated archive file.
#     command [required]
#         Command template which describes how to generate the given archive.
#         The template may contain the meta-token ~S and ~D.  The name of the
#         source directory is interpolated into the command in place of ~S, and
#         the destination package name is interpolated in place of ~D.
#
#     As a convenience, jobber.pl defines several pre-fabricated archiver
#     dictionaries:
#
#     $ARCHIVER_BZIP2
#         Archives with 'tar' and compresses with 'bzip2'. Extension: .tar.bz2
#     $ARCHIVER_GZIP
#         Archives with 'tar' and compresses with 'gzip'. Extension: .tgz
#     $ARCHIVER_ZIP
#         Archives and compresses with 'zip'. Extension: .zip
#
#     Default: ($ARCHIVER_BZIP2, $ARCHIVER_GZIP, $ARCHIVER_ZIP)
#
#------------------------------------------------------------------------------
#
# To-Do List
#
# * Generalize into a "job" processing mechanism.  Each job could reside within
#   its own source file.  Jobs such as checking out files from CVS, committing
#   changes to CVS, and publishing browseable and downloadable documentation
#   can perhaps just be additional tasks in the @jobber_tasks array.
# * The mechanism for publishing packages for download and online browsing
#   needs to be generalized further.  It is still somewhat geared toward the
#   publication of documentation packages and is, thus, not flexible enough to
#   publish packages which do not follow the directory structure designed for
#   documentation.
# * Eliminate the restriction in which only the first directory listed by the
#   "newdirs" array is exported by the "exports" key.
#
#------------------------------------------------------------------------------
use Carp;
use File::Basename;
use File::Copy;
use File::Find;
use File::Path;
use FileHandle;
use Getopt::Long;
use strict;
use warnings;
$Getopt::Long::ignorecase = 0;

my $PROG_NAME = 'jobber.pl';
my $PROG_VERSION = '33';
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000-2005 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

my $ARCHIVER_BZIP2 = {
    'name'      => 'bzip2',
    'extension' => 'tar.bz2',
    'command'   => 'tar --create --file=- ~S | bzip2 > ~D' };
my $ARCHIVER_GZIP = {
    'name'      => 'gzip',
    'extension' => 'tgz',
    'command'   => 'tar --create --file=- ~S | gzip > ~D' };
my $ARCHIVER_ZIP = {
    'name'      => 'zip',
    'extension' => 'zip',
    'command'   => 'zip -q -r ~D ~S' };

my $jobber_project_root = undef;
my $jobber_cvs_root = undef;
my $jobber_cvs_sources = undef;
my $jobber_cvs_flags = '';
my $jobber_browseable_dir = undef;
my $jobber_package_dir = undef;
my $jobber_public_group = undef;
my $jobber_public_mode = undef;
my $jobber_temp_dir = '/tmp';
my @jobber_binary_override = ('(?i)\.(dsw|dsp)$');
my @jobber_tasks = ();
my @jobber_archivers = ($ARCHIVER_BZIP2, $ARCHIVER_GZIP, $ARCHIVER_ZIP);
my %jobber_properties = ();

my $CONFIG_FILE = undef;
my $TESTING = undef;
my $CONV_DIR = undef;
my $CAPTURED_OUTPUT = '';

my @SCRIPT_OPTIONS = (
    'set=s'     => \%jobber_properties,
    'config=s'  => \$CONFIG_FILE,
    'test!'     => \$TESTING,
    'help'      => \&option_help,
    'version|V' => \&option_version,
    '<>'        => \&option_error
);

my @NEW_DIRECTORIES = ();
my @NEW_TEXT_FILES = ();
my @NEW_BINARY_FILES = ();
my @OUTDATED_FILES = ();

my @CONFIG_FILES = ('jobber.cfg', '.jobber');
my @CONFIG_DIRS = ('.');
push @CONFIG_DIRS, $ENV{'HOME'} if exists $ENV{'HOME'};

#------------------------------------------------------------------------------
# Terminate abnormally and print a textual representation of "errno".
#------------------------------------------------------------------------------
sub expire {
    my ($msg, $err) = @_;
    $err = $! unless $err;
    print STDERR "FATAL: $msg failed: $err\n";
    dump_captured();
    my $dir = conversion_dir();
    destroy_transient($dir) if $dir;
    croak 'Stopped';
}

#------------------------------------------------------------------------------
# Configuration file version assertion. The configuration file can invoke this
# function to ensure that the running jobber.pl script is sufficiently
# recent. If it is not, then the script aborts. (We use 'die' rather than
# expire() because this error will be trapped by 'eval' in load_config().
#------------------------------------------------------------------------------
sub jobber_require {
    my $ver = shift;
    die "minimum version assertion failed: requested $ver, got $PROG_VERSION"
        unless $ver <= $PROG_VERSION;
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
# Is file a "binary" file?  (Should the CVS flag "-kb" be used?)  First check
# the filename against the special-case @jobber_binary_override array, then
# check the file's content if necessary.
#------------------------------------------------------------------------------
sub is_binary {
    my $path = shift;
    foreach my $pattern (@jobber_binary_override) {
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
    if (defined($jobber_public_mode)) {
	chmod($jobber_public_mode, @dirs) or
	    expire("chmod($jobber_public_mode, ".join(' ', @dirs).')');
    }
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
    expire("temporary_name($dir,$prefix,$suffix)", "exceeded retry limit")
        if $i >= $limit;
    return "$dir/$prefix$i$suffix";
}

#------------------------------------------------------------------------------
# Return the name of the conversion directory. If not already set, then first
# compute it.
#------------------------------------------------------------------------------
sub conversion_dir {
  $CONV_DIR = temporary_name($jobber_temp_dir) unless $CONV_DIR;
  return $CONV_DIR;
}

#------------------------------------------------------------------------------
# Run an external shell command.
#------------------------------------------------------------------------------
sub run_command {
    my $cmd = shift;
    my $output = `{ $cmd ; } 2>&1`;
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
    run_command("cvs -Q $jobber_cvs_flags remove $paths") unless $TESTING;
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
    run_command("cvs -Q $jobber_cvs_flags add $flags $paths") unless $TESTING;
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
    print "CVSROOT: $jobber_cvs_root\nExtracting: $jobber_cvs_sources\n";
    run_command("cvs -Q $jobber_cvs_flags -d $jobber_cvs_root checkout -P " .
        $jobber_cvs_sources);
}

#------------------------------------------------------------------------------
# Print a summary list of files which were modified, added, or removed.
#------------------------------------------------------------------------------
sub cvs_update {
    my $message = 'Modification summary:';
    my $line = '-' x length($message);
    my $dirs = '';
    foreach my $task (@jobber_tasks) {
	$dirs .= " @{$task->{'olddirs'}}" if exists $task->{'olddirs'};
    }
    if ($dirs) {
        print "$line\n$message\n";
        my $changes = run_command("cvs -q $jobber_cvs_flags update $dirs");
	print $changes ? $changes : "  No files modified\n", "$line\n";
    }
}

#------------------------------------------------------------------------------
# Commit files to the CVS repository.  The 'cvs' command is smart enough to
# only commit files which have actually changed.
#------------------------------------------------------------------------------
sub cvs_commit_dirs {
    my ($dirs, $message) = @_;
    run_command("cvs -Q $jobber_cvs_flags commit -m \"$message\" @{$dirs}")
	unless $TESTING;
}

#------------------------------------------------------------------------------
# Commit repaired and generated files to the CVS repository.
#------------------------------------------------------------------------------
sub cvs_commit {
    foreach my $task (@jobber_tasks) {
	if (exists $task->{'olddirs'}) {
	    print "Committing $task->{'name'}.\n";
	    my $msg = exists $task->{'log'} ?
	        $task->{'log'} : 'Automated file repair/generation.';
	    cvs_commit_dirs($task->{'olddirs'}, $msg);
	}
    }
}

#------------------------------------------------------------------------------
# Perform all tasks by invoking the appropriate command of each task.
#------------------------------------------------------------------------------
sub run_tasks {
    foreach my $task (@jobber_tasks) {
	next unless exists($task->{'command'});
	print "$task->{'action'} $task->{'name'}.\n";
	run_command($task->{'command'});
    }
}

#------------------------------------------------------------------------------
# Scan and compare the newly generated directory hierarchies against existing
# hierarchies from the CVS repository.  For each difference between the two
# hierarchies, apply the appropriate CVS operation, adding or removing entries
# as necessary.
#------------------------------------------------------------------------------
sub apply_diffs {
    foreach my $task (@jobber_tasks) {
	next unless exists $task->{'olddirs'} and exists $task->{'newdirs'};
	print "Applying changes to $task->{'name'}.\n";

	my @olddirs = @{$task->{'olddirs'}};
	my @newdirs = @{$task->{'newdirs'}};
	foreach my $olddir (@olddirs) {
	    my $newdir = shift @newdirs;

	    print "  Scanning  ($olddir <=> $newdir).\n";
	    my @oldfiles = scandir($olddir);
	    my @newfiles = scandir($newdir);
	
	    print "  Comparing ($olddir <=> $newdir).\n";
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
    s/$token/$value/g or expire("token interpolation", "$token in $_");
    $_[0] = $_;
}

#------------------------------------------------------------------------------
# Post-process a browseable directory if requested.
#------------------------------------------------------------------------------
sub postprocess_browseable {
    my ($export, $dir, $indent) = @_;
    return unless exists $export->{'browseable-postprocess'};
    $indent = '' unless defined($indent);
    print "${indent}Post-processing.\n";

    my $cmd = $export->{'browseable-postprocess'};
    interpolate($cmd, '~T', $dir);
    run_command($cmd);
}

#------------------------------------------------------------------------------
# Publish a browseable copy of the generated files.
# FIXME: Presently, only publishes first directory referenced by "newdirs" key.
#------------------------------------------------------------------------------
sub publish_browseable {
    foreach my $task (@jobber_tasks) {
	next unless exists $task->{'export'}
	    and (exists $task->{'olddirs'} or exists $task->{'newdirs'});
	print "Publishing $task->{'name'}.\n";
	next if $TESTING;
	create_directory_deep($jobber_browseable_dir, $jobber_public_group)
	    unless -d $jobber_browseable_dir;

	my @srclist = exists $task->{'newdirs'} ? 
	    @{$task->{'newdirs'}} : @{$task->{'olddirs'}};
	my $src = shift @srclist;		# See FIXME above.
	my $dst = "$jobber_browseable_dir/$task->{'export'}->{'dir'}";
	my $new_dir = temporary_name($jobber_browseable_dir, 'new');
	my $old_dir = temporary_name($jobber_browseable_dir, 'old');

	print "  Preparing.\n";
	run_command("cp -r \"$src\" \"$new_dir\"");
	change_group_deep($jobber_public_group, "$new_dir");
	postprocess_browseable($task->{'export'}, $new_dir, '  ');

	print "  Installing.\n";
	rename_file($dst, $old_dir) if -e $dst;
	rename_file($new_dir, $dst);

	print "  Cleaning.\n";
	rmtree($old_dir);
    }
}

#------------------------------------------------------------------------------
# Publish an archive of the generated files.
#------------------------------------------------------------------------------
sub publish_package {
    my ($archiver, $src, $dst, $base, $indent) = @_;
    $indent = '' unless defined($indent);
    print "${indent}Archiving: $archiver->{'name'}\n";
    return if $TESTING;
    my $ext = $archiver->{'extension'};
    my $tmp_pkg = temporary_name($dst, 'pkg', ".$ext");
    my $package = "$dst/$base.$ext";
    my $cmd = $archiver->{'command'};
    interpolate($cmd, '~S', $src);
    interpolate($cmd, '~D', $tmp_pkg);
    run_command($cmd);
    rename_file($tmp_pkg, $package);
}

#------------------------------------------------------------------------------
# Publish generated directory hierarchies as archives of various formats as
# indicated by the @jobber_archivers array.  The 'appear' key in the 'export'
# dictionary of the @jobber_tasks array is used to control how the packaged
# directory appears in the archive.  For instance, although the directory
# 'out/docs/html/manual' may be packaged, the 'appear' key may indicate that it
# should appear as 'CS/docs/html/manual' in the generated package.  This
# functionality is handled by temporarily giving the target directory the
# desired name just prior to archiving.  Note that during this operation, the
# current working directory is "$CONV_DIR/$jobber_project_root", and all
# operations are performed relative to this location.
# FIXME: Presently, only publishes first directory referenced by "newdirs"
# and/or "olddirs" key.
#------------------------------------------------------------------------------
sub publish_packages {
    foreach my $task (@jobber_tasks) {
	next unless exists $task->{'export'}
	    and (exists $task->{'olddirs'} or exists $task->{'newdirs'});
	print "Packaging $task->{'name'}.\n";

	my @srclist = exists $task->{'newdirs'} ? 
	    @{$task->{'newdirs'}} : @{$task->{'olddirs'}};
	my $src = shift @srclist; # See FIXME above.
	my $export = $task->{'export'};
	my $do_appear = exists $export->{'appear'};
	my $appear = $do_appear ? $export->{'appear'} : $src;

	if ($do_appear) {
	    create_directory_deep(dirname($appear));
	    rename_file($src, $appear); # Sleight-of-hand (magic).
	}

	my $base = $export->{'name'};
	my $dst = "$jobber_package_dir/$export->{'dir'}";
	create_directory_deep($dst, $jobber_public_group) unless $TESTING;
	foreach my $archiver (@jobber_archivers) {
	    publish_package($archiver, $appear, $dst, $base, '  ');
	}
	rename_file($appear, $src) if $do_appear; # Unmagic.
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
    my $convdir = conversion_dir();
    create_transient($convdir);
    cvs_checkout();
    change_directory($jobber_project_root);
    run_tasks();
    apply_diffs();
    cvs_update();
    cvs_commit();
    publish_browseable();
    publish_packages();
    destroy_transient($convdir);
    print 'END: ', time_now(), "\n";
}

#------------------------------------------------------------------------------
# Dump output captured from shell commands.
#------------------------------------------------------------------------------
sub dump_captured {
    print "\nCaptured output:\n\n$CAPTURED_OUTPUT";
}

#------------------------------------------------------------------------------
# Print a validation error and terminate script.
#------------------------------------------------------------------------------
sub cfg_err {
    my $var = shift;
    expire("startup", "configuration property not initialized: $var");
}

#------------------------------------------------------------------------------
# Validate configuration information. Check that all required settings have
# been given values.
#------------------------------------------------------------------------------
sub validate_config {
    $jobber_project_root or cfg_err("\$jobber_project_root");
    $jobber_cvs_root     or cfg_err("\$jobber_cvs_root");
    @jobber_tasks        or cfg_err("\@jobber_tasks");

    foreach my $task (@jobber_tasks) {
        if (exists $task->{'export'}) {
	    $jobber_browseable_dir or cfg_err("\$jobber_browseable_dir");
	    $jobber_package_dir    or cfg_err("\$jobber_package_dir");
	    last;
	}
    }

    $jobber_cvs_sources = $jobber_project_root unless $jobber_cvs_sources;
}

#------------------------------------------------------------------------------
# Load configuration file. If specified via an option, then load that file,
# otherwise search for one.
#------------------------------------------------------------------------------
sub load_config {
    unless ($CONFIG_FILE) {
        SEARCH: foreach my $dir (@CONFIG_DIRS) {
	    foreach my $file (@CONFIG_FILES) {
		my $path = "$dir/$file";
		if (-e $path) {
		    $CONFIG_FILE = $path;
		    last SEARCH;
		}
	    }
        }
    }
    expire("no configuration", "unable to locate configuration file")
        unless $CONFIG_FILE;

    my $content;
    print "Configuration file: $CONFIG_FILE\n\n";
    {
	local $/; # Slurp file mode.
	open my $fh, '<', $CONFIG_FILE or expire("open configuration file");
	$content = <$fh>;
	close $fh;
    }
    eval $content;
    expire("load configuration", $@) if $@;

    validate_config();
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
browsing and download.  It should be run on a periodic basis, typically by an
automated mechanism.  The tasks which it performs are controlled by a
configuration file specified by the --config option. If --config is not used,
then it looks for a file named jobber.cfg or .jobber, first in the current
working directory, and then in the home directory (\$HOME).

Usage: $PROG_NAME [options]

Options:
    -s --set property=value
                 Assign value to an abitrary property name. The configuration
                 file can access this information via the \%jobber_properties
                 hash.
    -c --config file
                 Specify the configuration file rather than searching the
                 current working directory and the home directory for a file
                 named jobber.cfg or .jobber.
    -t --test    Process all tasks but do not actually modify the CVS
                 repository or export any files.
    -h --help    Display this usage message.
    -V --version Display the version number of @{[basename($0)]}

EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@SCRIPT_OPTIONS) or usage_error('');
    banner();
    print "Non-destructive testing mode enabled.\n\n" if $TESTING;
    load_config();
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
