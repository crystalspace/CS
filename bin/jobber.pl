#!/usr/bin/perl -w
#==============================================================================
#
#    Automated Documentation Generation and CVS Update Script
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
# jobber.pl
#
#    A tool for automatically processing documentation and committing changes
#    to the CVS repository in which the documentation resides.  This tool
#    performs several tasks:
#
#        - Repairs broken @node directives and @menu blocks.
#        - Converts Texinfo documentation to HTML format.
#        - Builds the public API reference.
#        - Builds the developer's API reference.
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

my $PROG_NAME = 'jobber.pl';
my $PROG_VERSION = '1.9';
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
#    OLD_DOC_DIR - Relative path to existing documentation directory hierarchy
#        within the CVS repository.
#    NEW_DOC_DIR - Relative path to generated documented directory hierarchy.
#    TEXINFO_DIR - Relative path to Texinfo source directory hierarchy.
#    BROWSEABLE_DIR - Directory into which generated documentation should be
#        copied for online browsing.
#    PACKAGE_DIR - Directory into which archives of generated documentation
#        are placed to make them available for download in package form.
#    OWNER_GROUP - Group to which to assign all directories which will exist
#        after script's termination (such as the "browseable" directory).  May
#        be 'undef' if no special group should be assigned.
#    PLATFORM - An essentially arbitrary platform name for the makefile
#        configuration step.  The rules for building the documentation do not
#        actually care about the platform, but the makefile architecture
#        expects the makefiles to have been configured before a makefile
#        target can be invoked.  For instance, if this script runs on Linux,
#        then this value should be "linux".  
#    LOG_MESSAGE_REPAIR - CVS log message for repaired Texinfo files.
#    LOG_MESSAGE_HTML - CVS log message for for Texinfo to HTML conversion.
#    LOG_MESSAGE_API - CVS log message for for generated API documentation.
#    TARGETS - A list of documentation target records.  Each target is used to
#        generate a particular documentation set.  Each target record is a
#        dictionary which contains the following keys.  The key "name" is the
#        human-readable name for this document set and is used in status
#        messages.  The key "dir" is the name of the directory containing the
#        document set within both OLD_DOC_DIR and NEW_DOC_DIR.  The key "make"
#        is the Makefile target to invoke in order to generate the particular
#        document set.  The key "exportto" is the directory name within
#        BROWSEABLE_DIR and PACKAGE_DIR into which the directory set is
#        published.  The key "exportas" is the base package name used when
#        generating downloadable packages via ARCHIVERS (below).  When
#        published, the base package name is combined with the archiver's file
#        extension and placed within the appropriate subdirectory of
#        PACKAGE_DIR.  The key "commit" is a boolean flag controlling whether
#        or not the generated files for this target should be committed to
#        CVS.  The key "log" is the log message to use for CVS transactions
#        involving this target.
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

# Doxygen resides in /usr/local/bin on SourceForge, so add that to path.
$ENV{'PATH'} .= ':/usr/local/bin';

my $PROJECT_ROOT = 'CS';
my $CVSUSER = 'sunshine';
my $CVSROOT = "$CVSUSER\@cvs1:/cvsroot/crystal";
my $CVS_SOURCES = $PROJECT_ROOT;
my $OLD_DOC_DIR = 'docs';
my $NEW_DOC_DIR = 'out/docs';
my $TEXINFO_DIR = 'docs/texinfo';
my $PUBLIC_DOC_DIR = '/home/groups/ftp/pub/crystal/docs';
my $BROWSEABLE_DIR = "$PUBLIC_DOC_DIR/online";
my $PACKAGE_DIR = "$PUBLIC_DOC_DIR/download";
my $OWNER_GROUP = 'crystal';
my $PLATFORM = 'linux';
my $LOG_MESSAGE_REPAIR = 'Automated Texinfo @node and @menu repair.';
my $LOG_MESSAGE_HTML = 'Automated Texinfo to HTML conversion.';
my $LOG_MESSAGE_API = 'Automated API reference generation.';

my @TARGETS =
    ({ 'name'     => 'User\'s Manual',
       'dir'      => 'html',
       'make'     => 'htmldoc',
       'exportto' => 'manual',
       'exportas' => 'csmanual-html',
       'commit'   => 1,
       'log'      => $LOG_MESSAGE_HTML },
     { 'name'     => 'Public API Reference',
       'dir'      => 'pubapi',
       'make'     => 'pubapi',
       'exportto' => 'pubapi',
       'exportas' => 'cspubapi-html',
       'commit'   => 1,
       'log'      => $LOG_MESSAGE_API },
     { 'name'     => 'Developer\'s API Reference',
       'dir'      => 'devapi',
       'make'     => 'devapi',
       'exportto' => 'devapi',
       'exportas' => 'csdevapi-html',
       'commit'   => 0,
       'log'      => $LOG_MESSAGE_API });

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
    my $dst = shift;
    my $file = basename($dst);
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
    my ($src, $dst) = @_;
    my $file = basename($dst);
    if (-d $src) {
	print "Adding directory: $file\n";
	create_directory($dst);
	run_command("cvs -Q add $dst") unless $DEBUG;
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
    run_command("cvs -Q -d $CVSROOT checkout -P $CVS_SOURCES");
}

#------------------------------------------------------------------------------
# Print a summary list of files which were modified, added, or removed.
#------------------------------------------------------------------------------
sub cvs_update {
    my $message = "Modification summary:";
    my $line = '-' x length($message);
    my $dirs = $TEXINFO_DIR;
    foreach my $target (@TARGETS) {
	if ($target->{'commit'}) {
	    $dirs .= " $OLD_DOC_DIR/$target->{'dir'}";
	}
    }
    print "$line\n$message\n" . run_command("cvs -q update $dirs") . "$line\n";
}

#------------------------------------------------------------------------------
# Commit files to the CVS repository.  The 'cvs' command is smart enough to
# only commit files which have actually changed.
#------------------------------------------------------------------------------
sub cvs_commit_dir {
    my ($dir, $message) = @_;
    run_command("cvs -Q commit -m \"$message\" $dir") unless $DEBUG;
}

#------------------------------------------------------------------------------
# Commit repaired and converted files to the CVS repository.
#------------------------------------------------------------------------------
sub cvs_commit {
    print "Committing Texinfo.\n";
    cvs_commit_dir($TEXINFO_DIR, $LOG_MESSAGE_REPAIR);
    foreach my $target (@TARGETS) {
	if ($target->{'commit'}) {
	    print "Committing $target->{'name'}.\n";
	    cvs_commit_dir("$OLD_DOC_DIR/$target->{'dir'}", $target->{'log'});
	}
    }
}

#------------------------------------------------------------------------------
# Perform the Texinfo repair and HTML conversion, then build public and
# developer's API reference manuals.  This is accomplished by invoking the
# appropriate makefile targets.  Note that the documentation related targets
# only work if the makefile system has been configured for a particular
# platform, thus that step must be performed first.
#------------------------------------------------------------------------------
sub build_docs {
    print "Configuring documentation.\n";
    run_command("$MAKE $PLATFORM");
    print "Repairing Texinfo files.\n";
    run_command("$MAKE repairdoc");
    foreach my $target (@TARGETS) {
	print "Building $target->{'name'}.\n";
	run_command("$MAKE $target->{'make'}");
    }
}

#------------------------------------------------------------------------------
# Scan and compare the newly generated documentation directory hierarchy
# against the existing hierarchy from the CVS repository.  For each difference
# between the two hierarchies, apply the appropriate CVS operation, adding or
# removing entries as necessary.
#------------------------------------------------------------------------------
sub apply_diffs {
    foreach my $target (@TARGETS) {
	if ($target->{'commit'}) {
	    print "Applying changes to $target->{'name'}.\n";
	    my $olddir = "$OLD_DOC_DIR/$target->{'dir'}";
	    my $newdir = "$NEW_DOC_DIR/$target->{'dir'}";
	
	    print "Scanning directories.\n";
	    my @oldfiles = scandir($olddir);
	    my @newfiles = scandir($newdir);
	
	    print "Comparing directories.\n";
	    my $oldfile = shift @oldfiles;
	    my $newfile = shift @newfiles;
	
	    while (defined($oldfile) || defined($newfile)) {
		if (!defined($newfile) ||
		    (defined($oldfile) && $oldfile lt $newfile)) {
		    cvs_remove("$olddir/$oldfile");
		    $oldfile = shift @oldfiles;
		}
		elsif (!defined($oldfile) || $oldfile gt $newfile) {
		    cvs_add("$newdir/$newfile", "$olddir/$newfile");
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
}

#------------------------------------------------------------------------------
# Publish a browseable copy of the generated documentation.  Also, copy
# index.htm to index.html (if present) so that the manual displays
# automatically when a client browses the directory.  (If SourceForge's Apache
# configuration file contained a "DirectoryIndex index.htm" directive, then it
# would not be necessary to copy index.htm to index.html.)
#------------------------------------------------------------------------------
sub publish_browseable {
    create_directory_deep($BROWSEABLE_DIR, $OWNER_GROUP);
    foreach my $target (@TARGETS) {
	my $name = $target->{'name'};
	print "Publishing $name.\n";
	next if $DEBUG;
    
	my $src = "$NEW_DOC_DIR/$target->{'dir'}";
	my $dst = "$BROWSEABLE_DIR/$target->{'exportto'}";
	my $new_dir = temporary_name($BROWSEABLE_DIR, 'new');
	my $old_dir = temporary_name($BROWSEABLE_DIR, 'old');

	print "Preparing.\n";
	run_command("cp -r \"$src\" \"$new_dir\"");
	change_group_deep($OWNER_GROUP, "$new_dir");
	copy_file("$new_dir/index.htm", "$new_dir/index.html")
	    if -e "$new_dir/index.htm";
    
	print "Installing.\n";
	rename_file($dst, $old_dir) if -e $dst;
	rename_file($new_dir, $dst);
    
	print "Cleaning.\n";
	rmtree($old_dir);
    }
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
    my ($archiver, $src, $dst, $base) = @_;
    print "Packaging: $archiver->{'name'}\n";
    return if $DEBUG;
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
# Publish generated documentation as archives of various formats.  This
# function packages up the generated documentation directory.  For consistency
# from the user's standpoint, the documentation within the archive should have
# a directory hierarchy identical to the one in the project itself (ie.
# CS/docs/html).  Unfortunately, CVS directories are present within this tree,
# and it would be preferable not to include them in the archive.  To work
# around this problem, this function archives files from the newly generated
# directory (out/docs) instead.  A small amount of sleight-of-hand is used to
# make the directory appear as though it resides directly in the project root.
# (Specifically, it is temporarily renamed from out/docs to CS/docs.)
#------------------------------------------------------------------------------
sub publish_packages {
    my $fake_root = "$PROJECT_ROOT/$OLD_DOC_DIR";
    create_directory_deep(dirname($fake_root));
    rename_file($NEW_DOC_DIR, $fake_root); # Sleight-of-hand (magic).
    foreach my $target (@TARGETS) {
	print "Packaging $target->{'name'}.\n";
	my $base = $target->{'exportas'};
	my $src = "$fake_root/$target->{'dir'}";
	my $dst = "$PACKAGE_DIR/$target->{'exportto'}";
	create_directory_deep($dst, $OWNER_GROUP);
	foreach my $archiver (@ARCHIVERS) {
	    publish_package($archiver, $src, $dst, $base);
	}
    }
    rename_file($fake_root, $NEW_DOC_DIR); # Unmagic.
    rmtree($PROJECT_ROOT); # Remove fake project root.
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
    cvs_checkout();
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

Usage: $PROG_NAME [options]

Options:
    -h --help   Print this usage message.
    -d --debug  Process the documentation but do not actually modify
                the CVS repository or export any files.

This program performs a series of documentation-related tasks.  It should be
run on a periodic basis, typically by some automated means.  The tasks which
it performs are:

    o Repairs out-of-date and broken \@node directives and \@menu blocks
      in the Texinfo source files.
    o Converts User's Manual Texinfo source files to HTML format.
    o Generates Public API Reference in HTML format.
    o Generates Developer's API Reference in HTML format.
    o Commits all changed Texinfo and HTML files to the CVS repository.
    o Publishes all HTML files for online browsing.
    o Creates and publishes packages from HTML files for download.

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
