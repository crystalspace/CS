#!/usr/bin/perl -w
#==============================================================================
#
#    Microsoft Visual C++ .SLN and .VCPROJ project file generator.
#    Copyright (C) 2000,2001 by Eric Sunshine <sunshine@sunshineco.com>
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
# msvcgen.pl
#
#    A tool for generating Microsoft Visual C++ .SLN and .VCPROJ project files
#    from a set of templates.  Invoke the --help option for detailed
#    instructions.
#
#------------------------------------------------------------------------------
use strict;
use File::Basename;
use Getopt::Long;
$Getopt::Long::ignorecase = 0;

use Digest::MD5 qw(md5_hex);
use HTMLEntities qw(encode_entities);

my $PROG_NAME = 'msvc7gen.pl';
my $PROG_VERSION = 3;
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000,2001 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

$main::opt_vcproj = 0;
$main::opt_d = 0;	# Alias for 'vcproj'.
$main::opt_sln = 0;
$main::opt_w = 0;	# Alias for 'sln'.
$main::opt_name = '';
$main::opt_N = '';	# Alias for 'name'.
$main::opt_template = '';
$main::opt_t = '';	# Alias for 'template'.
$main::opt_project = '';
$main::opt_target = '';
$main::opt_g = '';	# Alias for 'target'.
@main::opt_library = ();
@main::opt_L = ();	# Alias for 'library'.
@main::opt_lflags = ();
@main::opt_l = ();	# Alias for 'lflags'.
@main::opt_cflags = ();
$main::opt_output = '';
$main::opt_fragment = undef;
@main::opt_depend = ();
@main::opt_D = ();	# Alias for 'depend'.
$main::opt_template_dir = '';
$main::opt_T = '';	# Alias for 'template-dir'.
$main::opt_verbose = 0;
$main::opt_v = 0;	# Alias for 'verbose'.
$main::opt_quiet = 0;
$main::opt_version = 0;
$main::opt_V = 0;	# Alias for 'version'.
$main::opt_help = 0;

my @script_options = (
    'vcproj',
    'd',		# Alias for 'vcproj'.
    'sln',
    'w',		# Alias for 'sln'.
    'name=s',
    'N=s',		# Alias for 'name'.
    'template=s',
    't=s',		# Alias for 'template'.
    'project=s',
    'target=s',
    'g=s',		# Alias for 'target'.
    'library=s@',
    'L=s@',		# Alias for 'library'.
    'lflags=s@',
    'l=s@',		# Alias for 'lflags'.
    'cflags=s@',
    'output=s',
    'fragment:s',
    'depend=s@',
    'D=s@',		# Alias for 'depend'.
    'template-dir=s',
    'T=s',		# Alias for 'template-dir'.
    'verbose!',
    'v!',		# Alias for 'verbose'.
    'quiet!',
    'version',
    'V',		# Alias for 'version'.
    'help'
);

$main::verbosity = 0;
$main::makefile = '';
$main::groups = {};
@main::vpi_fragments = ();
@main::dpi_fragments = ();
@main::cfi_fragments = ();

$main::vcproj_template = '';
$main::vcproj_group_template = '';
$main::vcproj_file_template = '';

$main::sln_template = '';
$main::sln_group_template = '';
$main::sln_depend_template = '';
$main::sln_config_template = '';

$main::patterns = {
    'sources'     => {
	'name'    => 'Source Files',
	'pattern' => '(?i)\.(c|cc|cpp)$'
    },
    'headers'     => {
	'name'    => 'Header Files',
	'pattern' => '(?i)\.h$'
    },
    'resources'   => {
	'name'    => 'Resource Files'
    }
};

$main::targets = {
    'appgui'  => {
	'suffix' => 'exe'
    },
    'appcon'  => {
	'suffix' => 'exe'
    },
    'plugin'  => {
	'suffix' => 'dll'
    },
    'library' => {
	'suffix' => 'lib'
    },
    'group'   => {
	'suffix' => ''
    }
};

#------------------------------------------------------------------------------
# Emit an error message and terminate abnormally.
#------------------------------------------------------------------------------
sub fatal {
    my $msg = shift;
    die "ERROR: $msg\n";
}

#------------------------------------------------------------------------------
# Emit a warning message.
#------------------------------------------------------------------------------
sub warning {
    my $msg = shift;
    warn "WARNING: $msg\n";
}

#------------------------------------------------------------------------------
# Should we be verbose?
#------------------------------------------------------------------------------
sub verbose {
    return $main::verbosity > 0;
}

#------------------------------------------------------------------------------
# Should we be quiet?
#------------------------------------------------------------------------------
sub quiet {
    return $main::verbosity < 0;
}

#------------------------------------------------------------------------------
# Print an underlined title.
#------------------------------------------------------------------------------
sub print_title {
    my $msg = shift;
    print "$msg\n", '-' x length($msg), "\n";
}

#------------------------------------------------------------------------------
# Add a suffix to a filename if not already present.
#------------------------------------------------------------------------------
sub add_suffix {
    my ($name, $suffix) = @_;
    $name .= ".$suffix" unless $name =~ /(?i)\.$suffix$/;
    $_[0] = $name;
}

#------------------------------------------------------------------------------
# Remove a suffix from a filename if present.
#------------------------------------------------------------------------------
sub remove_suffix {
    my ($name, $suffix) = @_;
    $name =~ s/(?i)\.$suffix$//;
    $_[0] = $name;
}

#------------------------------------------------------------------------------
# create a unique guid from a project name.
#------------------------------------------------------------------------------
sub guid_from_name {
    my ($projname) = @_;
    my $rawguid = md5_hex($projname);
    my $shapedguid = '{'.substr($rawguid, 0, 8).'-'.substr($rawguid, 8, 4).'-'.
      substr($rawguid, 12, 4).'-'.substr($rawguid, 16, 4).'-'.substr($rawguid, 20, 12).'}';
    return $shapedguid;
}

#------------------------------------------------------------------------------
# insert html entities
#------------------------------------------------------------------------------
sub clean_string {
    my ($result) = @_;
    $result = join(' ', @{$result}) if ref($result) and ref($result) eq 'ARRAY';
    encode_entities ($result);
    return $result;
}

#------------------------------------------------------------------------------
# Read file contents.
#------------------------------------------------------------------------------
sub load_file {
    my $path = shift;
    my $content = '';
    my $size = -s $path;
    fatal("Failed to load file $path: $!") unless defined($size);
    if ($size) {
	open(FILE, "<$path") or fatal("Unable to open $path: $!");
	binmode(FILE);
	read(FILE, $content, $size) == $size
	    or fatal("Failed to read all $size bytes of $path: $!");
	close(FILE);
    }
    return $content;
}

#------------------------------------------------------------------------------
# Save file contents.
#------------------------------------------------------------------------------
sub save_file {
    my ($path, $content) = @_;
    open (FILE, ">$path") or fatal("Unable to open $path: $!");
    binmode(FILE);
    print FILE $content if length($content);
    close(FILE);
}

#------------------------------------------------------------------------------
# Load a template file.
#------------------------------------------------------------------------------
sub load_template {
    my ($file, $ext) = @_;
    return load_file("$main::opt_template_dir/$file.$ext");
}

#------------------------------------------------------------------------------
# Load templates.
#------------------------------------------------------------------------------
sub load_templates {
    if ($main::opt_sln) {
	$main::sln_template = load_template('sln', 'tpl');
    }
    else {
	$main::vcproj_template = load_template("$main::opt_template", 'tpl');
	$main::vcproj_group_template  = load_template('vcprojgroup', 'tpi');
	$main::vcproj_file_template   = load_template('vcprojfile', 'tpi');
	$main::sln_group_template  = load_template('slngroup', 'tpi');
	$main::sln_depend_template = load_template('slndep', 'tpi');
	$main::sln_config_template = load_template('slncfg', 'tpi');
    }
}

#------------------------------------------------------------------------------
# Interpolate a value into a string.  If the value is an array reference, then
# compose a string out of the array elements and use the result as the
# replacement value.  Otherwise assume that the value is a simple string.
#------------------------------------------------------------------------------
sub interpolate {
    my ($pattern, $value, $buffer) = @_;
    $value = join(' ', @{$value}) if ref($value) and ref($value) eq 'ARRAY';
    ${$buffer} =~ s/$pattern/$value/g;
}

#------------------------------------------------------------------------------
# For each item in a list, interpolate the item into a template based upon
# a pattern string, and return the concatenation of all resulting buffers.
#------------------------------------------------------------------------------
sub interpolate_items {
    my ($items, $pattern, $template) = @_;
    my $result = '';
    my $item;
    foreach $item (sort(@{$items})) {
	my $buffer = $template;
	interpolate($pattern, $item, \$buffer);
	$result .= $buffer;
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of a single .VCPROJ file group.
#------------------------------------------------------------------------------
sub interpolate_vcproj_group {
    my $group = shift;
    my $result = $main::vcproj_group_template;
    my $name = $main::patterns->{$group}->{'name'};
    my $files = $main::groups->{$group};
    my $files_buffer =
	interpolate_items($files, '%file%', $main::vcproj_file_template);
    interpolate('%group%', $name, \$result);
    interpolate('%files%', $files_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build all file-groups for a .VCPROJ from a template.
#------------------------------------------------------------------------------
sub interpolate_vcproj_groups {
    my $result = '';
    my $group;
    foreach $group (sort(keys(%{$main::groups}))) {
        $result .= interpolate_vcproj_group($group);
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build a .VCPROJ from a template.
#------------------------------------------------------------------------------
sub interpolate_vcproj {
    my $result = $main::vcproj_template;
    interpolate('%project%',  clean_string($main::opt_project),     \$result);
    interpolate('%makefile%', clean_string($main::makefile),        \$result);
    interpolate('%target%',   clean_string($main::opt_target),      \$result);
    interpolate('%libs%',     clean_string(\@main::opt_library),    \$result);
    interpolate('%lflags%',   clean_string(\@main::opt_lflags),     \$result);
    interpolate('%cflags%',   clean_string(\@main::opt_cflags),     \$result);
    interpolate('%groups%',   interpolate_vcproj_groups(), \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of .SLN project fragment.
#------------------------------------------------------------------------------
sub interpolate_sln_project {
    my $result = $main::sln_group_template;
    interpolate('%project%', $main::opt_project, \$result);
    interpolate('%vcproj%', basename($main::opt_output), \$result);
    interpolate('%guid%', guid_from_name($main::opt_name), \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of .SLN dependency group fragment.
#------------------------------------------------------------------------------
sub interpolate_sln_dependency {
    my $result = '';
    my $depcnt = 0;
    my $dependency;
    foreach $dependency (sort(@main::opt_depend)) {
	my $buffer = $main::sln_depend_template;
	interpolate('%depnum%', $depcnt, \$buffer);
	interpolate('%guid%', guid_from_name($main::opt_name), \$buffer);
	interpolate('%depguid%', guid_from_name($dependency), \$buffer);
	$result .= $buffer;
	$depcnt++;
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of .SLN config group fragment.
#------------------------------------------------------------------------------
sub interpolate_sln_config {
    my $result = $main::sln_config_template;
    interpolate('%guid%', guid_from_name($main::opt_name), \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build a .SLN from a list of dependency groups fragments.
#------------------------------------------------------------------------------
sub interpolate_sln {
    my $proj_buffer = '';
    my $depends_buffer = '';
    my $config_buffer = '';
    my $fragment;
    foreach $fragment (sort(@main::vpi_fragments)) {
	$proj_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::dpi_fragments)) {
	$depends_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::cfi_fragments)) {
	$config_buffer .= load_file($fragment);
    }
    my $result = $main::sln_template;
    interpolate('%groups%', $proj_buffer, \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    interpolate('%configs%', $config_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Create a .VCPROJ file and optionally a .SLN dependency fragment file.
#------------------------------------------------------------------------------
sub create_vcproj {
    save_file($main::opt_output, interpolate_vcproj());
    print "Generated: $main::opt_output\n" unless quiet();
    if (defined($main::opt_fragment)) {
	save_file($main::opt_fragment, '');
	print "Generated: $main::opt_fragment\n" unless quiet();
	
    	my $vpi_frag = $main::opt_fragment;
    	add_suffix ($vpi_frag, 'vpi');
	save_file($vpi_frag, interpolate_sln_project());
	print "Generated: $vpi_frag\n" unless quiet();

    	my $dpi_frag = $main::opt_fragment;
    	add_suffix ($dpi_frag, 'dpi');
	save_file($dpi_frag, interpolate_sln_dependency());
	print "Generated: $dpi_frag\n" unless quiet();

    	my $cfi_frag = $main::opt_fragment;
    	add_suffix ($cfi_frag, 'cfi');
	save_file($cfi_frag, interpolate_sln_config());
	print "Generated: $cfi_frag\n" unless quiet();
    }
    print "\n" unless quiet();
}

#------------------------------------------------------------------------------
# Create a .SLN file.
#------------------------------------------------------------------------------
sub create_sln {
    save_file($main::opt_output, interpolate_sln());
    print "Generated: $main::opt_output\n\n" unless quiet();
}

#------------------------------------------------------------------------------
# Display .SLN option summary.
#------------------------------------------------------------------------------
sub summarize_sln_options {
    print "Mode:   sln\n";
    print "Name:   $main::opt_name\n" if $main::opt_name;
    print "Output: $main::opt_output\n";
}

#------------------------------------------------------------------------------
# Display .VCPROJ option summary.
#------------------------------------------------------------------------------
sub summarize_vcproj_options {
    print <<"EOT";
Mode:      vcproj
Name:      $main::opt_name
Output:    $main::opt_output
EOT
    print <<"EOT" if defined($main::opt_fragment);
Fragment:  $main::opt_fragment
EOT
    print <<"EOT";
Template:  $main::opt_template
Project:   $main::opt_project
Target:    $main::opt_target
Makefile:  $main::makefile
EOT
    print <<"EOT" if @main::opt_library;
Libraries: @main::opt_library
EOT
    print <<"EOT" if @main::opt_lflags;
LFLAGS:    @main::opt_lflags
EOT
    print <<"EOT" if @main::opt_cflags;
CFLAGS:    @main::opt_cflags
EOT
    my @groups = sort(keys(%{$main::groups}));
    print <<"EOT" if @groups;
Groups:    @groups
EOT
}

#------------------------------------------------------------------------------
# Display an option summary.
#------------------------------------------------------------------------------
sub summarize_options {
    print_title("Option Summary");
    summarize_sln_options() if $main::opt_sln;
    summarize_vcproj_options() if $main::opt_vcproj;
    print "\n";
}

#------------------------------------------------------------------------------
# Validate .VCPROJ options.
#------------------------------------------------------------------------------
sub validate_vcproj_options {
    usage_error("The --depend option can be used only with --fragment.")
	if !defined($main::opt_fragment) and @main::opt_depend;
    usage_error("Must specify a template type.") unless $main::opt_template;
    usage_error("Unrecognized template type.")
	unless $main::targets->{$main::opt_template};
    usage_error("Must specify a name for the project.") unless $main::opt_name;
}

#------------------------------------------------------------------------------
# Validate .SLN options.
#------------------------------------------------------------------------------
sub validate_sln_options {
    usage_error("The --template option can be used only with --vcproj.")
	if $main::opt_template;
    usage_error("The --project option can be used only with --vcproj.")
	if $main::opt_project;
    usage_error("The --target option can be used only with --vcproj.")
	if $main::opt_target;
    usage_error("The --library option can be used only with --vcproj.")
	if @main::opt_library;
    usage_error("The --lflags option can be used only with --vcproj.")
	if @main::opt_lflags;
    usage_error("The --cflags option can be used only with --vcproj.")
	if @main::opt_cflags;
    usage_error("The --fragment option can be used only with --vcproj.")
	if defined($main::opt_fragment);
    usage_error("The --depend option can be used only with --vcproj.")
	if @main::opt_depend;
    usage_error("Must specify --name or --output.")
	unless $main::opt_name or $main::opt_output;
    usage_error("Must specify --name or --output, but not both.")
	if $main::opt_name and $main::opt_output;
}

#------------------------------------------------------------------------------
# Validate options.
#------------------------------------------------------------------------------
sub validate_options {
    usage_error("Must specify --sln or --vcproj.")
	unless $main::opt_sln or $main::opt_vcproj;
    usage_error("Must specify --sln or --vcproj, but not both.")
	if $main::opt_sln and $main::opt_vcproj;

    validate_sln_options() if $main::opt_sln;
    validate_vcproj_options() if $main::opt_vcproj;
}

#------------------------------------------------------------------------------
# Process options which apply to .SLN and .VCPROJ.
#------------------------------------------------------------------------------
sub process_global_options {
    $main::verbosity =  1 if $main::opt_verbose;
    $main::verbosity = -1 if $main::opt_quiet;
    $main::opt_output = $main::opt_name unless $main::opt_output;
    $main::opt_template_dir = '.' unless $main::opt_template_dir;
}

#------------------------------------------------------------------------------
# Process option aliases.
#------------------------------------------------------------------------------
sub process_option_aliases {
    $main::opt_verbose = 1 if $main::opt_v;
    $main::opt_version = 1 if $main::opt_V;
    $main::opt_vcproj = 1 if $main::opt_d;
    $main::opt_sln = 1 if $main::opt_w;
    $main::opt_name = $main::opt_N unless $main::opt_name;
    $main::opt_target = $main::opt_g unless $main::opt_target;
    $main::opt_template = $main::opt_t unless $main::opt_template;
    $main::opt_template_dir = $main::opt_T unless $main::opt_template_dir;
    push(@main::opt_library, @main::opt_L);
    push(@main::opt_lflags,  @main::opt_l);
    push(@main::opt_depend,  @main::opt_D);
}

#------------------------------------------------------------------------------
# Process .VCPROJ command-line options.
#------------------------------------------------------------------------------
sub process_vcproj_options {
    add_suffix($main::opt_output, 'vcproj');
    ($main::makefile = basename($main::opt_output)) =~ s/(?i)\.vcproj$/\.mak/;
    $main::opt_project = $main::opt_name unless $main::opt_project;

    my $target_ext = $main::targets->{$main::opt_template}->{'suffix'};
    $main::opt_target = $main::opt_name unless $main::opt_target;
    add_suffix($main::opt_target, $target_ext) if $target_ext;

    $main::opt_fragment = $main::opt_name
	if defined($main::opt_fragment) and !$main::opt_fragment;
#    add_suffix($main::opt_fragment, 'sli') if defined($main::opt_fragment);

    my @libraries;
    my $library;
    foreach $library (@main::opt_library) {
	add_suffix($library, 'lib');
	push(@libraries, $library);
    }
    @main::opt_library = @libraries;

    my @depends;
    my $depend;
    foreach $depend (@main::opt_depend) {
	remove_suffix($depend, 'vcproj');
	push(@depends, $depend);
    }
    @main::opt_depend = @depends;

    my @files;
    my $file;
    foreach $file (@ARGV) {
	$file =~ tr:/:\\:;
	push(@files, $file);
    }

    my $default_type = '';
    my $type;
    foreach $type (keys(%{$main::patterns})) {
	my $pattern = $main::patterns->{$type}->{'pattern'};
	$default_type = $type, next unless $pattern;
	my @matches = grep(/$pattern/, @files);
	if (@matches) {
	    $main::groups->{$type} = \@matches;
	    @files = grep(!/$pattern/, @files);
	}
    }
    $main::groups->{$default_type} = \@files if @files;
}

#------------------------------------------------------------------------------
# Process .SLN command-line options.
#------------------------------------------------------------------------------
sub process_sln_options {
    add_suffix($main::opt_output, 'sln');

    my $fragment;
    foreach $fragment (@ARGV) {
	my $vpi_frag = $fragment;    	
	add_suffix($vpi_frag, 'vpi');
	push(@main::vpi_fragments, $vpi_frag);
	
	my $dpi_frag = $fragment;    	
	add_suffix($dpi_frag, 'dpi');
	push(@main::dpi_fragments, $dpi_frag);
	
	my $cfi_frag = $fragment;    	
	add_suffix($cfi_frag, 'cfi');
	push(@main::cfi_fragments, $cfi_frag);
    }
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
    print $stream <<"EOT";
Given a set of input files and project dependencies, generates Microsoft
Visual C++ 7 (aka 2002 aka .NET) .SLN and .VCPROJ project files from a set
of templates.

A .VCPROJ file is generated when --vcproj is specified.  The type of project
represented by the .VCPROJ file is selected using the --template option.  The
template can be one of "appgui", "appcon", "plugin", "library", or "group";
which represent a GUI application, a console application, a dynamic link
library (DLL), a static library (LIB), or an group project, respectively.
The "group" project type is used for creating pseudo-dependency targets
within a .SLN but does not actually generate any resources.

Template files are loaded from the current working directory or from a named
template directory.  The template files appgui.tpl, appcon.tpl, plugin.tpl,
library.tpl, and group.tpl correspond to the project types available via the
--template option.  Template files may contain the variables \%project\%,
\%target\%, \%makefile\%, \%groups\%, \%libs\%, \%lflags\%, and \%cflags\%.
The variables \%project\%, \%target\%, \%libs\%, \%flags\%, and \%cflags\%
are replaced with values specified via the --project, --target, --library,
--lflags, and --cflags options, respectively.  The replacement value for
\%makefile\% is the same as the name of the generated .VCPROJ file with the
exception that .mak is substituted for .vcproj as the suffix.

The template vcprojgroup.tpi is used multiple times to build a value for the
\%groups\% variable mentioned above.  This template is used to create the
file groups "Source Files", "Header Files", and "Resource Files", as needed,
within the generated .VCPROJ file.  This template may contain the variables
\%group\% and \%files\%.  The \%group\% variable is automatically replaced
by the name of the group being generated (for instance "Source Files").

The template vcprojfile.tpi is used multiple times to build a value for the
\%files\% variable mentioned above.  This template is used to specify each
file which makes up a file group (represented by vcprojgroup.tpi).  This
template may contain the variable \%file\% which represents a file name
provided as an argument to this script during .VCPROJ generation.

During .VCPROJ file generation, a .SLN dependency fragment file can also be
generated with the --fragment option and zero or more --depend options.
The generated fragment file can later be used to create a complete .SLN file
containing a dependency graph for the entire project.

The dependency fragment file is created from the template slngroup.tpi.
This template may contain the variables \%project\%, \%vcproj\%, and
\%depends\%.  The \%project\% variable has the same meaning as it does when
used with the templates specified via --template.  The \%vcproj\% variable is
replaced by the name of the generated .VCPROJ file (see --output), except that
the directory portion of the path is stripped off.

The template slndep.tpi is used mulitple times to build a value for the
\%depends\% variable mentioned above.  This template is used to specify each
project name which makes up a dependency group (represented by
slngroup.tpi).  This template may contain the variable \%depend\% which
represents a project name upon which the generated .VCPROJ file depends (see
--depend).

Finally, a .SLN file is generated when --sln is specified.  The SLN file is
created by merging the contents of dependency fragment files into the
template sln.tpl.  This template may contain the variable \%groups\%, which
is replaced by the collective contents of all fragment files named as
arguments to this script during .SLN generation.

Usage: $PROG_NAME [options] <file> ...

Global Options:
    -d --vcproj  Generate a .VCPROJ file.
    -w --sln     Generate a .SLN file.
    -T <path>
    --template-dir=<path>
                 Specifies the directory where the template files reside.
                 If not specified, then template files are assumed to exist
                 in the current working directory.
    -v --verbose Emit informational messages about the processing.  Can be
                 negated with --noverbose.  Deafult is --noverbose.
    -q --quiet   Suppress all output except for error messages.  Can be
                 negated with --noquiet.  Default is --noquiet.
    -V --version Display the version number of $PROG_NAME.
    -h --help    Display this usage message.

.VCPROJ Options:
<file> Path of a file which belongs to the project represented by this .VCPROJ.
       Any number of files may be specified, or none if the project contains
       no files.  Files with the suffixes .c, .cc, and .cpp are placed in
       the project's "Source Files" group; files with the suffix .h are
       placed in the "Header Files" group; and all other files are placed in
       the "Resource Files" group.  Each mentioned file replaces the
       variable \%file\% in the vcprojfile.tpi template.

    -N <name>
    --name=<name>
                 The basic name associated with this .VCPROJ file.  This name is
                 used for automatic generation of other required names (such
                 as output name, target name, fragment name) if those names
                 are not explicitly specified.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated .VCPROJ file.  A .vcproj
                 suffix is automatically appended if absent.  If not
                 specified, then the name given with --name is used as the
                 output name and the file is written to the current working
                 directory.
    -t <type>
    --template=<type>
                 Specifies the template type for this .VCPROJ.  The type may be
                 one of "appgui", "appcon", "plugin", "library", or "group".
                 See the discussion of .VCPROJ generation (above) for an
                 explanation of the various template types.
    -p <name>
    --project=<name>
                 Specifies the display name of the project for the Microsoft
                 Visual C++ IDE.  This is the replacement value for the
                 \%project\% variable in template files.  If not specified,
                 then the name given with --name is used as the project name.
    -g <name>
    --target=<name>
                 Specifies the name of the actual target generated by this
                 .VCPROJ.  This is the replacement value for the \%target\%
                 variable in the template files.  For GUI and console
                 programs, this is the name of the resultant executable
                 (EXE); for plugins it is the dynamic link library (DLL);
                 and for libraries, it is the static library (LIB).  If not
                 specified, then the name given with --name is used as the
                 target name.  If the name does not end with an appropriate
                 suffix (one of .exe, .dll, or .lib), then the suffix is
                 added automatically.
    -L <name>
    --library=<name>
                 Specifies the name of an extra Windows library with which
                 the project should be linked in addition to those which are
                 already mentioned in the template file.  This is the
                 replacement value for the \%libs\% variable in the template
                 files.  Typically, libraries are only specified for
                 executable and plug-in templates.  A .lib suffix is added
                 to the name automatically if absent.  The --library option
                 may be given any number of times to specify any number of
                 additional libraries, or not at all if no additional
                 libraries are required.  The --library option differs from
                 the --depend option in that it refers to libraries which
                 exist outside of the project graph, whereas --depend always
                 refers to projects which are members of the project graph.
    -l <flags>
    --lflags=<flags>
                 Specifies extra linker options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%lflags\% variable
                 in the template files.  Typically, linker options are only
                 specified for executable and plug-in templates.  The
                 --lflags option may be specified any number of times, in
                 which case the effects are cumulative, or not at all if no
                 extra linker options are required.
    -c <flags>
    --cflags=<flags>
                 Specifies extra compiler options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%cflags\% variable
                 in the template files.  The --cflags option may be
                 specified any number of times, in which case the effects
                 are cumulative, or not at all if no extra compiler options
                 are required.  As an example, a pre-processor macro named
                 __FOOBAR__ can be defined with: --cflags='/D "__FOOBAR__"'
    -f
    -f <path>
    --fragment
    --fragment=<path>
                 Specifies whether or not to generate a .SLN dependency
                 fragment file along with the .VCPROJ file.  A dependency
                 fragment file lists the other projects upon which this .VCPROJ
                 relies.  If not specified, then no fragment file is
                 generated.  If specified, but no path is given, then the
                 name given with --name is used as the fragment name.
                 Fragment files use the suffix .sli.  This suffix is added
                 automatically if absent.  Generated fragment files can
                 later be incorporated into a .SLN file to collectively
                 define a dependency graph for the entire project.  Each
                 dependency specified with the --depend option is listed in
                 the generated fragment file.
    -D <project>
    --depend=<project>
                 Specifies the name of a project upon which this .VCPROJ depends.
                 Each project name is written to the .SLN dependency fragment
                 file for this .VCPROJ and is the replacement value for the
                 \%depend\% variable in the slndep.tpi template.  The
                 --depend option may be specified any number of times, or
                 not at all if the project has no dependencies (which is
                 often the case for "library" projects).  This option can
                 only be used in conjunction with the --fragment option.
                 
.SLN Options:
<file> Path of a dependency fragment file emitted during a .VCPROJ generation
       phase.  The fragment file contains a list of project names on which
       the containing project fragment depends.  Any number of fragment
       files may be specified, or none if the .SLN does not need to contain
       any dependency groups.  Taken collectively, these fragments define an
       entire dependency graph for the .SLN.  See the discussion of fragment
       file generation (above) for details.

    -N <name>
    --name=<name>
                 Specifies the base name of the generated .SLN file.  A .sln
                 suffix is automatically appended to generate the final
                 output name and the file is created in the current working
                 directory. This option and the --output option are mutually
                 exclusive.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated .SLN file.  A .sln
                 suffix is automatically appended if absent.  This option
                 and the --name option are mutually exclusive.
EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@script_options) or usage_error('');
    process_option_aliases();
    print_help() if $main::opt_help;
    print_version() if $main::opt_version;
    validate_options();
    process_global_options();
    process_sln_options() if $main::opt_sln;
    process_vcproj_options() if $main::opt_vcproj;
    banner() unless quiet();
}

sub print_version {
    banner(\*STDOUT);
    exit(0);
}

sub print_help {
    print_usage(\*STDOUT);
    exit(0);
}

sub usage_error {
    my $msg = shift;
    print STDERR "ERROR: $msg\n" if $msg;
    print_usage(\*STDERR);
    exit(1);
}

#------------------------------------------------------------------------------
# Perform the complete repair process.
#------------------------------------------------------------------------------
process_options();
summarize_options() if verbose();
load_templates();
create_vcproj() if $main::opt_vcproj;
create_sln() if $main::opt_sln;
