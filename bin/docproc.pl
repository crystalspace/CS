#!/usr/bin/perl -w
#==============================================================================
#
#    Automated Texinfo to HTML Conversion and CVS Update Script
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
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
# docproc.pl
#
#    A tool for automatically processing documentation and committing changes
#    to the CVS repository in which the documentation resides.  This tool
#    performs several tasks:
#
#        - Repairs broken @node directives and @menu blocks.
#        - Converts Texinfo documentation to HTML format.
#        - Commits all changes to the CVS repository.
#        - Makes the generated HTML available online for browsing.
#        - Makes archives of the generated HTML availble for download.
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
#        3 1 * * * $HOME/bin/docconv.pl
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
#      CVS, run the various 'make' commands (make platform, make htmldoc,
#      make repairdoc, etc.), and perform the comparision and commit of
#      generated files.
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

my $PROG_NAME = 'docproc.pl';
my $PROG_VERSION = '1.5';
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

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
#    FAKE_DIRS - A list of directories to be created (empty) after files have
#        been extracted from the repository.  This is a list of directories
#        which must exist (though empty) for the conversion process to
#        succeed.  In particular, the makefile configuration step expects
#        CS/include to exists since it places volatile.h there.
#    OLD_HTML_DIR - Relative path to existing HTML directory hierarchy within
#        the CVS repository.
#    NEW_HTML_DIR - Relative path to newly generated HTML directory hierarchy.
#    TEXINFO_DIR - Relative path to Texinfo source directory hierarchy.
#    BROWSEABLE_DIR - Directory into which generated HTML should be copied for
#        browsing via a web browser.
#    PACKAGE_DIR - Directory into which archives of generated documentation
#        are placed to make them available for download in package form.
#    PACKAGE_BASE - Base name of the packaged archives.  An appropriate file
#        extention will be appended for each generated archive.
#    OWNER_GROUP - Group to which to assign all directories which will exist
#        after script's termination (such as the "browseable" directory).  May
#        be 'undef' if no special group should be assigned.
#    PLATFORM - An essentially arbitrary platform name for the makefile
#        configuration step.  The rules for building the documentation do not
#        actually care about the platform, but the makefile architecture
#        expects the makefiles to have been configured before a makefile
#        target can be invoked.
#    LOG_MESSAGE_HTML = Log message for CVS transactions for HTML conversion.
#    LOG_MESSAGE_REPAIR = Log message for CVS transactions for repaired
#        document files.
#    ARCHIVERS - A list of archiver records.  Each arechiver is used to
#        generate a package archive given an input directory.  Each archiver
#        record contains the following keys.  The key "name" specifies the
#        archiver's printable name.  The key "ext" is the file extension for
#        the generated archive file.  The key "cmd" is the actual command
#        template which describes how to generate the given archive.  It may
#        contain the meta-token ~S and ~D.  The name of the source directory
#        is interpolated into the command in place of ~S, and the destination
#        package name is interpolated in place of ~D.
#------------------------------------------------------------------------------

$ENV{'CVS_RSH'} = 'ssh';
my $PROJECT_ROOT = 'CS';
my $CVSUSER = 'sunshine';
my $CVSROOT = "$CVSUSER\@cvs1:/cvsroot/crystal";
my $CVS_SOURCES = "$PROJECT_ROOT/bin $PROJECT_ROOT/docs $PROJECT_ROOT/mk " .
    "$PROJECT_ROOT/Makefile $PROJECT_ROOT/libs/cssys/unix";
my $FAKE_DIRS = "$PROJECT_ROOT/include";
my $OLD_HTML_DIR = 'docs/html';
my $NEW_HTML_DIR = 'out/docs/html';
my $TEXINFO_DIR = 'docs/texinfo';
my $PUBLIC_DOC_DIR = '/home/groups/crystal/htdocs/docs';
my $BROWSEABLE_DIR = "$PUBLIC_DOC_DIR/online/manual";
my $PACKAGE_DIR = "$PUBLIC_DOC_DIR/download";
my $PACKAGE_BASE = 'csmanual';
my $OWNER_GROUP = 'crystal';
my $PLATFORM = 'linux';
my $LOG_MESSAGE_HTML = 'Automated Texinfo to HTML conversion.';
my $LOG_MESSAGE_REPAIR = 'Automated Texinfo @node and @menu repair.';

my @ARCHIVERS =
    ({ 'name' => 'gzip',
       'ext'  => 'tgz',
       'cmd'  => 'tar --create --file=- ~S | gzip > ~D' },
     { 'name' => 'bzip2',
       'ext'  => 'tar.bz2',
       'cmd'  => 'tar --create --file=- ~S | bzip2 > ~D' },
     { 'name' => 'zip',
       'ext'  => 'zip',
       'cmd'  => 'zip -q -r ~D ~S' });

#------------------------------------------------------------------------------
# Internal configuration.
#------------------------------------------------------------------------------
my $DEBUG = undef;
my $CONV_DIR = tmpnam();
my $CAPTURED_OUTPUT = '';
my $MAKE = 'make';

my @SCRIPT_OPTIONS = (
    'debug!' => \$DEBUG,
    'help'   => \&option_help,
    '<>'     => \&option_error
);

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
# Generate a temporary name in a directory.  (Perl tmpnam() only knows '/tmp'.)
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
# its control files.
#------------------------------------------------------------------------------
sub scandir {
    my $dir = shift;
    my @files;
    find(sub{my $n=$File::Find::name; $n=~s|$dir/?||; push(@files,$n) if $n},
	 $dir);
    return sort(grep(!/CVS/,@files));
}

#------------------------------------------------------------------------------
# Remove an entry from the CVS repository if it does not exist in the newly
# generated documentation hierarchy.  Note that for directories, no action is
# taken since CVS automatically removes empty directories on the client side
# when the "-P" switch is used with the CVS "update" command.
#------------------------------------------------------------------------------
sub cvs_remove {
    my $file = shift;
    my $dst = "$OLD_HTML_DIR/$file";
    if (-d $dst) {
	print "Pruning directory: $file\n";
    }
    else {
	print "Removing file: $file\n";
	remove_file($dst);
	run_command("cvs -Q remove $dst") unless $DEBUG;
    }
}

#------------------------------------------------------------------------------
# Add a file or directory from the newly generated documentation hierarchy to
# the CVS repository.  Take special care to use the "-kb" CVS flag when adding
# a binary file (such as an image) to the repository.
#------------------------------------------------------------------------------
sub cvs_add {
    my $file = shift;
    my $src = "$NEW_HTML_DIR/$file";
    my $dst = "$OLD_HTML_DIR/$file";
    if (-d $src) {
	print "Adding directory: $file\n";
	create_directory($dst);
	run_command("cvs -Q add -m \"$LOG_MESSAGE_HTML\" $dst") unless $DEBUG;
    }
    else {
	my $isbin = -B $src;
	my $flags = ($isbin ? '-kb' : '');
	print "Adding file: $file [" . ($isbin ? 'binary' : 'text') . "]\n";
	copy_file($src, $dst);
	run_command("cvs -Q add $flags $dst") unless $DEBUG;
    }
}

#------------------------------------------------------------------------------
# File exists in both existing CVS repository and in newly created
# documentation directory hierarchy.  Copy the newly generated file over top
# of existing file.  Later, at commit time, CVS will determine if file has
# actually been modified.
#------------------------------------------------------------------------------
sub cvs_examine {
    my $file = shift;
    my $src = "$NEW_HTML_DIR/$file";
    my $dst = "$OLD_HTML_DIR/$file";
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
    run_command("cvs -Q -d $CVSROOT checkout -P $CVS_SOURCES");
}

#------------------------------------------------------------------------------
# Print a summary list of files which were modified, added, or removed.
#------------------------------------------------------------------------------
sub cvs_update {
    print "Modification summary:\n" .
	run_command("cvs -q update $TEXINFO_DIR $OLD_HTML_DIR");
}

#------------------------------------------------------------------------------
# Commit files to the CVS repository.  The 'cvs' command is smart enough to
# only commit files which have actually changed.
#------------------------------------------------------------------------------
sub cvs_commit_files {
    my ($files, $message) = @_;
    run_command("cvs -Q commit -m \"$message\" $files") unless $DEBUG;
}

#------------------------------------------------------------------------------
# Commit repaired and converted files to the CVS repository.
#------------------------------------------------------------------------------
sub cvs_commit {
    print "Committing changes.\n";
    cvs_commit_files($TEXINFO_DIR,  $LOG_MESSAGE_REPAIR);
    cvs_commit_files($OLD_HTML_DIR, $LOG_MESSAGE_HTML  );
}

#------------------------------------------------------------------------------
# Extract the appropriate files from the CVS repository.  The curent technique
# of using the makefile to convert documentation also requires some directories
# to exist even though they are not needed by the actual documentation
# conversion step, so fake them up.  In particular, the makefile configuration
# step expects the CS/include directory to exist so that it can create
# CS/include/volatile.h.
#------------------------------------------------------------------------------
sub extract_docs {
    my $dir;
    cvs_checkout();
    foreach $dir ($FAKE_DIRS) {
	print "Faking directory: $dir\n";
	create_directory($dir);
    }
}

#------------------------------------------------------------------------------
# Perform the Texinfo repair and HTML conversion.  This is accomplished by
# invoking the appropriate makefile targets.  Note that the documentation
# related targets only work if the makefile system has been configured for a
# particular platform, thus that step must be perform first.
#------------------------------------------------------------------------------
sub build_docs {
    print "Configuring documentation.\n";
    run_command("$MAKE $PLATFORM");
    print "Repairing documentation.\n";
    run_command("$MAKE repairdoc");
    print "Building HTML documentation.\n";
    run_command("$MAKE htmldoc");
}

#------------------------------------------------------------------------------
# Scan and compare the newly generated documentation directory hierarchy
# against the existing hierarchy from the CVS repository.  For each difference
# between the two hierarchies, apply the appropriate CVS operation, adding or
# removing entries as necessary.
#------------------------------------------------------------------------------
sub apply_diffs {
    print "Scanning directories.\n";
    my @oldfiles = scandir($OLD_HTML_DIR);
    my @newfiles = scandir($NEW_HTML_DIR);

    print "Comparing directories.\n";
    my $oldfile = shift @oldfiles;
    my $newfile = shift @newfiles;

    while (defined($oldfile) || defined($newfile)) {
	if (!defined($newfile) || defined($oldfile) && $oldfile lt $newfile) {
	    cvs_remove($oldfile);
	    $oldfile = shift @oldfiles;
	}
	elsif (!defined($oldfile) || $oldfile gt $newfile) {
	    cvs_add($newfile);
	    $newfile = shift @newfiles;
	}
	else { # Filenames are identical.
	    cvs_examine($newfile);
	    $oldfile = shift @oldfiles;
	    $newfile = shift @newfiles;
	}
    }
}

#------------------------------------------------------------------------------
# Publish a browseable copy of the generated documentation.  Also, copy
# index.html to index.html so that the manual displays automatically if a
# client browses the directory.  (The alternative would be to add a
# "DirectoryIndex index.htm" directive to Apache's .htaccess file, SourceForge
# disallows this.) 
#------------------------------------------------------------------------------
sub publish_browseable {
    print("Publishing browseable.\n"), return if $DEBUG;
    print "Preparing browseable.\n";
    my $parent = dirname($BROWSEABLE_DIR);
    create_directory_deep($parent, $OWNER_GROUP);

    my $new_dir = temporary_name($parent, 'new');
    my $old_dir = temporary_name($parent, 'old');
    run_command("cp -r \"$NEW_HTML_DIR\" \"$new_dir\"");
    change_group_deep($OWNER_GROUP, "$new_dir");
    copy_file("$new_dir/index.htm", "$new_dir/index.html")
	if -e "$new_dir/index.htm";

    print "Publishing browseable.\n";
    rename_file($BROWSEABLE_DIR, $old_dir) if -e $BROWSEABLE_DIR;
    rename_file($new_dir, $BROWSEABLE_DIR);

    print "Cleaning browseable.\n";
    rmtree($old_dir);
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
# Publish an archive of the generated documentation.
#------------------------------------------------------------------------------
sub publish_package {
    my ($archiver, $dir) = @_;
    print "Packaging: $archiver->{'name'}\n";
    return if $DEBUG;
    my $ext = $archiver->{'ext'};
    my $tmp_pkg = temporary_name($PACKAGE_DIR, 'pkg', ".$ext");
    my $package = "$PACKAGE_DIR/$PACKAGE_BASE.$ext";
    my $cmd = $archiver->{'cmd'};
    interpolate($cmd, '~S', $dir);
    interpolate($cmd, '~D', $tmp_pkg);
    run_command($cmd);
    rename_file($tmp_pkg, $package);
}

#------------------------------------------------------------------------------
# Publish generated documentation as archives of various formats.  This
# function packages up the generated documentation directory.  For consistency
# from the user's standpoint, the documentation within the archive should have
# a directory hierarchy identical to the one in the project itself (ie.
# CS/docs/html).  Unfortunately, CVS directories are present within this tree,
# and it would be preferable not to include them in the archive.  To work
# around this problem, this function archives the newly generated directory
# (out/docs/html) instead.  A small amount of sleight-of-hand is used to make
# the directory appear as though it resides directly in the project root.
# (Specifically, it is temporarily renamed from out/docs/html to CS/docs/html.)
#------------------------------------------------------------------------------
sub publish_packages {
    my $fake_root = "$PROJECT_ROOT/$OLD_HTML_DIR";
    create_directory_deep($PACKAGE_DIR, $OWNER_GROUP);
    create_directory_deep(dirname($fake_root));
    rename_file($NEW_HTML_DIR, $fake_root); # Sleight-of-hand (magic).
    foreach my $archiver (@ARCHIVERS) {
	publish_package($archiver, $fake_root);
    }
    rename_file($fake_root, $NEW_HTML_DIR); # Unmagic.
    rmtree($PROJECT_ROOT);

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
# Perform the complete documentation repair and conversion process.
#------------------------------------------------------------------------------
sub process_docs {
    print 'BEGIN: ' . time_now() . "\n";
    create_transient($CONV_DIR);
    extract_docs();
    change_directory($PROJECT_ROOT);
    build_docs();
    apply_diffs();
    cvs_update();
    cvs_commit();
    publish_browseable();
    publish_packages();
    destroy_transient($CONV_DIR);
    print 'END: ' . time_now() . "\n";
}

#------------------------------------------------------------------------------
# Dump output captured from shell commands.
#------------------------------------------------------------------------------
sub dump_captured {
    print "\nCaptured output:\n\n$CAPTURED_OUTPUT";
}

#------------------------------------------------------------------------------
# Display usage statement.
#------------------------------------------------------------------------------
sub print_usage {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    print $stream <<EOT;
$PROG_NAME version $PROG_VERSION
$COPYRIGHT

Repairs out-of-date and broken \@node directives and \@menu blocks, then
converts Texinfo documentation to HTML and updates the CVS repository with
the results.

Usage: $PROG_NAME [options]

Options:
    -h --help   Print this usage message.
    -d --debug  Process the documentation but do not modify the CVS
                repository.
EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@SCRIPT_OPTIONS) or usage_error('');
    print "$PROG_NAME version $PROG_VERSION\n$COPYRIGHT\n\n";
    print "Debugging enabled.\n\n" if $DEBUG;
}

sub option_help  { print_usage(\*STDOUT); exit(0); }
sub option_error { usage_error("Unknown option: @_\n"); }

sub usage_error {
    my $message = shift;
    print STDERR "$message\n";
    print_usage(\*STDERR);
    exit(1);
}

#------------------------------------------------------------------------------
# Run the conversion.
#------------------------------------------------------------------------------
process_options();
process_docs();
dump_captured();
