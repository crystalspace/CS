#!/usr/bin/perl -w
#==============================================================================
#
#    Microsoft Visual C++ DSW and DSP project file generator.
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
#    A tool for generating Microsoft Visual C++ DSW and DSP project files
#    from a set of templates.  Invoke the --help option for detailed
#    instructions.
#
#------------------------------------------------------------------------------
use strict;
use File::Basename;
use Getopt::Long;
$Getopt::Long::ignorecase = 0;

my $PROG_NAME = 'msvcgen.pl';
my $PROG_VERSION = 3;
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000,2001 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

$main::opt_dsp = 0;
$main::opt_d = 0;	# Alias for 'dsp'.
$main::opt_dsw = 0;
$main::opt_w = 0;	# Alias for 'dsw'.
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
    'dsp',
    'd',		# Alias for 'dsp'.
    'dsw',
    'w',		# Alias for 'dsw'.
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
@main::fragments = ();

$main::dsp_template = '';
$main::dsp_group_template = '';
$main::dsp_file_template = '';

$main::dsw_template = '';
$main::dsw_group_template = '';
$main::dsw_depend_template = '';

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
    if ($main::opt_dsw) {
	$main::dsw_template = load_template('dsw', 'tpl');
    }
    else {
	$main::dsp_template = load_template("$main::opt_template", 'tpl');
	$main::dsp_group_template  = load_template('dspgroup', 'tpi');
	$main::dsp_file_template   = load_template('dspfile', 'tpi');
	$main::dsw_group_template  = load_template('dswgroup', 'tpi');
	$main::dsw_depend_template = load_template('dswdep', 'tpi');
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
# Build the contents of a single DSP file group.
#------------------------------------------------------------------------------
sub interpolate_dsp_group {
    my $group = shift;
    my $result = $main::dsp_group_template;
    my $name = $main::patterns->{$group}->{'name'};
    my $files = $main::groups->{$group};
    my $files_buffer =
	interpolate_items($files, '%file%', $main::dsp_file_template);
    interpolate('%group%', $name, \$result);
    interpolate('%files%', $files_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build all file-groups for a DSP from a template.
#------------------------------------------------------------------------------
sub interpolate_dsp_groups {
    my $result = '';
    my $group;
    foreach $group (sort(keys(%{$main::groups}))) {
        $result .= interpolate_dsp_group($group);
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build a DSP from a template.
#------------------------------------------------------------------------------
sub interpolate_dsp {
    my $result = $main::dsp_template;
    interpolate('%project%',  $main::opt_project,       \$result);
    interpolate('%makefile%', $main::makefile,          \$result);
    interpolate('%target%',   $main::opt_target,        \$result);
    interpolate('%libs%',     \@main::opt_library,      \$result);
    interpolate('%lflags%',   \@main::opt_lflags,       \$result);
    interpolate('%cflags%',   \@main::opt_cflags,       \$result);
    interpolate('%groups%',   interpolate_dsp_groups(), \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of DSW dependency group fragment.
#------------------------------------------------------------------------------
sub interpolate_dsw_group {
    my $depends_buffer = interpolate_items(\@main::opt_depend, '%depend%',
	$main::dsw_depend_template);
    my $result = $main::dsw_group_template;
    interpolate('%project%', $main::opt_project, \$result);
    interpolate('%dsp%', basename($main::opt_output), \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build a DSW from a list of dependency groups fragments.
#------------------------------------------------------------------------------
sub interpolate_dsw {
    my $buffer = '';
    my $fragment;
    foreach $fragment (sort(@main::fragments)) {
	$buffer .= load_file($fragment);
    }
    my $result = $main::dsw_template;
    interpolate('%groups%', $buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Create a DSP file and optionally a DSW dependency fragment file.
#------------------------------------------------------------------------------
sub create_dsp {
    save_file($main::opt_output, interpolate_dsp());
    print "Generated: $main::opt_output\n" unless quiet();
    if (defined($main::opt_fragment)) {
	save_file($main::opt_fragment, interpolate_dsw_group());
	print "Generated: $main::opt_fragment\n" unless quiet();
    }
    print "\n" unless quiet();
}

#------------------------------------------------------------------------------
# Create a DSW file.
#------------------------------------------------------------------------------
sub create_dsw {
    save_file($main::opt_output, interpolate_dsw());
    print "Generated: $main::opt_output\n\n" unless quiet();
}

#------------------------------------------------------------------------------
# Display DSW option summary.
#------------------------------------------------------------------------------
sub summarize_dsw_options {
    print "Mode:   dsw\n";
    print "Name:   $main::opt_name\n" if $main::opt_name;
    print "Output: $main::opt_output\n";
}

#------------------------------------------------------------------------------
# Display DSP option summary.
#------------------------------------------------------------------------------
sub summarize_dsp_options {
    print <<"EOT";
Mode:      dsp
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
    summarize_dsw_options() if $main::opt_dsw;
    summarize_dsp_options() if $main::opt_dsp;
    print "\n";
}

#------------------------------------------------------------------------------
# Validate DSP options.
#------------------------------------------------------------------------------
sub validate_dsp_options {
    usage_error("The --depend option can be used only with --fragment.")
	if !defined($main::opt_fragment) and @main::opt_depend;
    usage_error("Must specify a template type.") unless $main::opt_template;
    usage_error("Unrecognized template type.")
	unless $main::targets->{$main::opt_template};
    usage_error("Must specify a name for the project.") unless $main::opt_name;
}

#------------------------------------------------------------------------------
# Validate DSW options.
#------------------------------------------------------------------------------
sub validate_dsw_options {
    usage_error("The --template option can be used only with --dsp.")
	if $main::opt_template;
    usage_error("The --project option can be used only with --dsp.")
	if $main::opt_project;
    usage_error("The --target option can be used only with --dsp.")
	if $main::opt_target;
    usage_error("The --library option can be used only with --dsp.")
	if @main::opt_library;
    usage_error("The --lflags option can be used only with --dsp.")
	if @main::opt_lflags;
    usage_error("The --cflags option can be used only with --dsp.")
	if @main::opt_cflags;
    usage_error("The --fragment option can be used only with --dsp.")
	if defined($main::opt_fragment);
    usage_error("The --depend option can be used only with --dsp.")
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
    usage_error("Must specify --dsw or --dsp.")
	unless $main::opt_dsw or $main::opt_dsp;
    usage_error("Must specify --dsw or --dsp, but not both.")
	if $main::opt_dsw and $main::opt_dsp;

    validate_dsw_options() if $main::opt_dsw;
    validate_dsp_options() if $main::opt_dsp;
}

#------------------------------------------------------------------------------
# Process options which apply to DSW and DSP.
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
    $main::opt_dsp = 1 if $main::opt_d;
    $main::opt_dsw = 1 if $main::opt_w;
    $main::opt_name = $main::opt_N unless $main::opt_name;
    $main::opt_target = $main::opt_g unless $main::opt_target;
    $main::opt_template = $main::opt_t unless $main::opt_template;
    $main::opt_template_dir = $main::opt_T unless $main::opt_template_dir;
    push(@main::opt_library, @main::opt_L);
    push(@main::opt_lflags,  @main::opt_l);
    push(@main::opt_depend,  @main::opt_D);
}

#------------------------------------------------------------------------------
# Process DSP command-line options.
#------------------------------------------------------------------------------
sub process_dsp_options {
    add_suffix($main::opt_output, 'dsp');
    ($main::makefile = basename($main::opt_output)) =~ s/(?i)\.dsp$/\.mak/;
    $main::opt_project = $main::opt_name unless $main::opt_project;

    my $target_ext = $main::targets->{$main::opt_template}->{'suffix'};
    $main::opt_target = $main::opt_name unless $main::opt_target;
    add_suffix($main::opt_target, $target_ext) if $target_ext;

    $main::opt_fragment = $main::opt_name
	if defined($main::opt_fragment) and !$main::opt_fragment;
    add_suffix($main::opt_fragment, 'dwi') if defined($main::opt_fragment);

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
	remove_suffix($depend, 'dsp');
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
# Process DSW command-line options.
#------------------------------------------------------------------------------
sub process_dsw_options {
    add_suffix($main::opt_output, 'dsw');

    my $fragment;
    foreach $fragment (@ARGV) {
	add_suffix($fragment, 'dwi');
	push(@main::fragments, $fragment);
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
Visual C++ DSW and DSP project files from a set of templates.

A DSP file is generated when --dsp is specified.  The type of project
represented by the DSP file is selected using the --template option.  The
template can be one of "appgui", "appcon", "plugin", "library", or "group";
which represent a GUI application, a console application, a dynamic link
library (DLL), a static library (LIB), or an group project, respectively.
The "group" project type is used for creating pseudo-dependency targets
within a DSW but does not actually generate any resources.

Template files are loaded from the current working directory or from a named
template directory.  The template files appgui.tpl, appcon.tpl, plugin.tpl,
library.tpl, and group.tpl correspond to the project types available via the
--template option.  Template files may contain the variables \%project\%,
\%target\%, \%makefile\%, \%groups\%, \%libs\%, \%lflags\%, and \%cflags\%.
The variables \%project\%, \%target\%, \%libs\%, \%flags\%, and \%cflags\%
are replaced with values specified via the --project, --target, --library,
--lflags, and --cflags options, respectively.  The replacement value for
\%makefile\% is the same as the name of the generated DSP file with the
exception that .mak is substituted for .dsp as the suffix.

The template dspgroup.tpi is used multiple times to build a value for the
\%groups\% variable mentioned above.  This template is used to create the
file groups "Source Files", "Header Files", and "Resource Files", as needed,
within the generated DSP file.  This template may contain the variables
\%group\% and \%files\%.  The \%group\% variable is automatically replaced
by the name of the group being generated (for instance "Source Files").

The template dspfile.tpi is used multiple times to build a value for the
\%files\% variable mentioned above.  This template is used to specify each
file which makes up a file group (represented by dspgroup.tpi).  This
template may contain the variable \%file\% which represents a file name
provided as an argument to this script during DSP generation.

During DSP file generation, a DSW dependency fragment file can also be
generated with the --fragment option and zero or more --depend options.
The generated fragment file can later be used to create a complete DSW file
containing a dependency graph for the entire project.

The dependency fragment file is created from the template dswgroup.tpi.
This template may contain the variables \%project\%, \%dsp\%, and
\%depends\%.  The \%project\% variable has the same meaning as it does when
used with the templates specified via --template.  The \%dsp\% variable is
replaced by the name of the generated DSP file (see --output), except that
the directory portion of the path is stripped off.

The template dswdep.tpi is used mulitple times to build a value for the
\%depends\% variable mentioned above.  This template is used to specify each
project name which makes up a dependency group (represented by
dswgroup.tpi).  This template may contain the variable \%depend\% which
represents a project name upon which the generated DSP file depends (see
--depend).

Finally, a DSW file is generated when --dsw is specified.  The DSW file is
created by merging the contents of dependency fragment files into the
template dsw.tpl.  This template may contain the variable \%groups\%, which
is replaced by the collective contents of all fragment files named as
arguments to this script during DSW generation.

Usage: $PROG_NAME [options] <file> ...

Global Options:
    -d --dsp     Generate a DSP file.
    -w --dsw     Generate a DSW file.
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

DSP Options:
<file> Path of a file which belongs to the project represented by this DSP.
       Any number of files may be specified, or none if the project contains
       no files.  Files with the suffixes .c, .cc, and .cpp are placed in
       the project's "Source Files" group; files with the suffix .h are
       placed in the "Header Files" group; and all other files are placed in
       the "Resource Files" group.  Each mentioned file replaces the
       variable \%file\% in the dspfile.tpi template.

    -N <name>
    --name=<name>
                 The basic name associated with this DSP file.  This name is
                 used for automatic generation of other required names (such
                 as output name, target name, fragment name) if those names
                 are not explicitly specified.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated DSP file.  A .dsp
                 suffix is automatically appended if absent.  If not
                 specified, then the name given with --name is used as the
                 output name and the file is written to the current working
                 directory.
    -t <type>
    --template=<type>
                 Specifies the template type for this DSP.  The type may be
                 one of "appgui", "appcon", "plugin", "library", or "group".
                 See the discussion of DSP generation (above) for an
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
                 DSP.  This is the replacement value for the \%target\%
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
                 Specifies whether or not to generate a DSW dependency
                 fragment file along with the DSP file.  A dependency
                 fragment file lists the other projects upon which this DSP
                 relies.  If not specified, then no fragment file is
                 generated.  If specified, but no path is given, then the
                 name given with --name is used as the fragment name.
                 Fragment files use the suffix .dwi.  This suffix is added
                 automatically if absent.  Generated fragment files can
                 later be incorporated into a DSW file to collectively
                 define a dependency graph for the entire project.  Each
                 dependency specified with the --depend option is listed in
                 the generated fragment file.
    -D <project>
    --depend=<project>
                 Specifies the name of a project upon which this DSP depends.
                 Each project name is written to the DSW dependency fragment
                 file for this DSP and is the replacement value for the
                 \%depend\% variable in the dswdep.tpi template.  The
                 --depend option may be specified any number of times, or
                 not at all if the project has no dependencies (which is
                 often the case for "library" projects).  This option can
                 only be used in conjunction with the --fragment option.
                 
DSW Options:
<file> Path of a dependency fragment file emitted during a DSP generation
       phase.  The fragment file contains a list of project names on which
       the containing project fragment depends.  Any number of fragment
       files may be specified, or none if the DSW does not need to contain
       any dependency groups.  Taken collectively, these fragments define an
       entire dependency graph for the DSW.  See the discussion of fragment
       file generation (above) for details.

    -N <name>
    --name=<name>
                 Specifies the base name of the generated DSW file.  A .dsw
                 suffix is automatically appended to generate the final
                 output name and the file is created in the current working
                 directory. This option and the --output option are mutually
                 exclusive.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated DSW file.  A .dsw
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
    process_dsw_options() if $main::opt_dsw;
    process_dsp_options() if $main::opt_dsp;
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
create_dsp() if $main::opt_dsp;
create_dsw() if $main::opt_dsw;
