#!/usr/bin/perl -w
#==============================================================================
#
#    Microsoft Visual C++ project and workspace file generator.
#    Copyright (C) 2000-2002 by Eric Sunshine <sunshine@sunshineco.com>
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
#    A tool for generating Microsoft Visual C++ project files and workspaces
#    from a set of templates.  Invoke the --help option for detailed
#    instructions.
#
#------------------------------------------------------------------------------
use strict;
use Digest::MD5 qw(md5_hex);
use File::Basename;
use Getopt::Long;
$Getopt::Long::ignorecase = 0;

my $PROG_NAME = 'msvcgen.pl';
my $PROG_VERSION = 4;
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000-2002 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

$main::opt_project = 0;
$main::opt_p = 0;	# Alias for 'project'.
$main::opt_projext = '';
$main::opt_px = '';	# Alias for 'projext'.
$main::opt_workspace = 0;
$main::opt_w = 0;	# Alias for 'workspace'.
$main::opt_wsext = '';
$main::opt_wx = '';	# Alias for 'wsext'.
$main::opt_name = '';
$main::opt_N = '';	# Alias for 'name'.
$main::opt_template = '';
$main::opt_t = '';	# Alias for 'template'.
$main::opt_projname = '';
$main::opt_htmlents = 0;
$main::opt_H = 0;	# Alias for 'htmlents'.
$main::opt_target = '';
$main::opt_g = '';	# Alias for 'target'.
@main::opt_library = ();
@main::opt_L = ();	# Alias for 'library'.
@main::opt_delaylib = ();
@main::opt_Y = ();	# Alias for 'delaylib'.
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
    'project',
    'p',		# Alias for 'project'.
    'projext=s',
    'px=s', 		# Alias for 'projext'.
    'workspace',
    'w',		# Alias for 'workspace'.
    'wsext=s',	
    'wx=s',		# Alias for 'wsext'.
    'name=s',
    'N=s',		# Alias for 'name'.
    'template=s',
    't=s',		# Alias for 'template'.
    'projname=s',
    'htmlents',		
    'H',		# Alias for 'htmlents'.
    'target=s',
    'g=s',		# Alias for 'target'.
    'library=s@',
    'L=s@',		# Alias for 'library'.
    'delaylib=s@',
    'Y=s@',		# Alias for 'delaylib'.
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
$main::guid = '';
$main::groups = {};
@main::pji_fragments = ();
@main::dpi_fragments = ();
@main::cfi_fragments = ();

$main::project_template = '';
$main::project_group_template = '';
$main::project_file_template = '';
$main::project_delaylib_template = '';

$main::workspace_template = '';
$main::workspace_project_template = '';
$main::workspace_depend_template = '';
$main::workspace_config_template = '';

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
    my $shapedguid = '{' .
	substr($rawguid,  0,  8) . '-' .
	substr($rawguid,  8,  4) . '-' .
	substr($rawguid, 12,  4) . '-' . 
	substr($rawguid, 16,  4) . '-' .
	substr($rawguid, 20, 12) .
	'}';
    return uc($shapedguid);
}

#------------------------------------------------------------------------------
# insert html entities
#------------------------------------------------------------------------------
sub clean_string {
    my ($result) = @_;
    $result = join(' ', @{$result}) if ref($result) and ref($result) eq 'ARRAY';
    $result =~ s/\"/\&quot\;/g;
    $result =~ s/\</\&lt\;/g;
    $result =~ s/\>/\&gt\;/g;
    return $result;
}

#------------------------------------------------------------------------------
# only insert html entities when --htmlents option is set
#------------------------------------------------------------------------------
sub wellformed_string {
    my ($result) = @_;
    $result = join(' ', @{$result}) if ref($result) and ref($result) eq 'ARRAY';
    if ($main::opt_htmlents) {
    	return clean_string($result);
    } else {
    	return $result;
    }
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
    if ($main::opt_workspace) {
	$main::workspace_template = load_template('ws', 'tpl');
    }
    else {
	$main::project_template = load_template("$main::opt_template", 'tpl');
	$main::project_group_template  = load_template('prjgroup', 'tpi');
	$main::project_file_template   = load_template('prjfile', 'tpi');
	$main::project_delaylib_template   = load_template('prjdelay', 'tpi');
	$main::workspace_project_template  = load_template('wsgroup', 'tpi');
	$main::workspace_depend_template = load_template('wsdep', 'tpi');
	$main::workspace_config_template = load_template('wscfg', 'tpi');
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
# Build the contents of a single project file group.
#------------------------------------------------------------------------------
sub interpolate_project_group {
    my $group = shift;
    my $result = $main::project_group_template;
    my $name = $main::patterns->{$group}->{'name'};
    my $files = $main::groups->{$group};
    my $files_buffer =
	interpolate_items($files, '%file%', $main::project_file_template);
    interpolate('%group%', $name, \$result);
    interpolate('%files%', $files_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build all file-groups for a project from a template.
#------------------------------------------------------------------------------
sub interpolate_project_groups {
    my $result = '';
    my $group;
    foreach $group (sort(keys(%{$main::groups}))) {
        $result .= interpolate_project_group($group);
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build a project from a template.
#------------------------------------------------------------------------------
sub interpolate_project {
    my $result = $main::project_template;
    my $delaylibs = "";
    foreach my $delaylib (@main::opt_delaylib) {
    	$delaylibs .= $main::project_delaylib_template;
    	interpolate('%delaylib%', $delaylib, \$delaylibs);
    }
    interpolate('%project%',   wellformed_string($main::opt_projname), \$result);
    interpolate('%makefile%',  wellformed_string($main::makefile),     \$result);
    interpolate('%target%',    wellformed_string($main::opt_target),   \$result);
    interpolate('%libs%',      wellformed_string(\@main::opt_library), \$result);
    interpolate('%delaylibs%', wellformed_string($delaylibs), 	       \$result);
    interpolate('%lflags%',    wellformed_string(\@main::opt_lflags),  \$result);
    interpolate('%cflags%',    wellformed_string(\@main::opt_cflags),  \$result);
    interpolate('%groups%',    interpolate_project_groups(),           \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of a workspace dependency group fragment.
#------------------------------------------------------------------------------
sub interpolate_ws_dependency {
    my $result = '';
    my $depcnt = 0;
    my $dependency;
    foreach $dependency (sort(@main::opt_depend)) {
	my $buffer = $main::workspace_depend_template;
	interpolate('%depnum%', $depcnt, \$buffer);
	interpolate('%guid%', $main::guid, \$buffer);
	interpolate('%depguid%', guid_from_name($dependency), \$buffer);
	interpolate('%depend%', $dependency, \$buffer);
	$result .= $buffer;
	$depcnt++;
    }
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of workspace dependency group fragment.
#------------------------------------------------------------------------------
sub interpolate_ws_project {
    my $depends_buffer = shift;
    my $result = $main::workspace_project_template;
    interpolate('%project%', $main::opt_projname, \$result);
    interpolate('%projfile%', basename($main::opt_output), \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    interpolate('%guid%', $main::guid, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of workspace config group fragment.
#------------------------------------------------------------------------------
sub interpolate_ws_config {
    my $result = $main::workspace_config_template;
    interpolate('%guid%', $main::guid, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build a workspace from a list of dependency groups fragments.
#------------------------------------------------------------------------------
sub interpolate_workspace {
    my $proj_buffer = '';
    my $depends_buffer = '';
    my $config_buffer = '';
    my $fragment;
    foreach $fragment (sort(@main::pji_fragments)) {
	$proj_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::dpi_fragments)) {
	$depends_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::cfi_fragments)) {
	$config_buffer .= load_file($fragment);
    }
    my $result = $main::workspace_template;
    interpolate('%projects%', $proj_buffer, \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    interpolate('%configs%', $config_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Create a DSP file and optionally a DSW dependency fragment file.
#------------------------------------------------------------------------------
sub create_project {
    save_file($main::opt_output, interpolate_project());
    print "Generated: $main::opt_output\n" unless quiet();
    
    if (defined($main::opt_fragment)) {
    	$main::guid = guid_from_name($main::opt_projname);
    	
	save_file($main::opt_fragment, "Dummy file.\n");
	print "Generated: $main::opt_fragment\n" unless quiet();
	
    	my $dependencies = interpolate_ws_dependency();
    	my $dpi_frag = $main::opt_fragment;
    	add_suffix ($dpi_frag, 'dpi');
	save_file($dpi_frag, $dependencies);
	print "Generated: $dpi_frag\n" unless quiet();

    	my $pji_frag = $main::opt_fragment;
    	add_suffix ($pji_frag, 'pji');
	save_file($pji_frag, interpolate_ws_project($dependencies));
	print "Generated: $pji_frag\n" unless quiet();

    	my $cfi_frag = $main::opt_fragment;
    	add_suffix ($cfi_frag, 'cfi');
	save_file($cfi_frag, interpolate_ws_config());
	print "Generated: $cfi_frag\n" unless quiet();
    }
}

#------------------------------------------------------------------------------
# Create a DSW file.
#------------------------------------------------------------------------------
sub create_workspace {
    save_file($main::opt_output, interpolate_workspace());
    print "Generated: $main::opt_output\n\n" unless quiet();
}

#------------------------------------------------------------------------------
# Display DSW option summary.
#------------------------------------------------------------------------------
sub summarize_workspace_options {
    print "Mode:      workspace\n";
    print "Name:      $main::opt_name\n" if $main::opt_name;
    print "Output:    $main::opt_output\n";
    print "Extension: $main::opt_wsext\n";
}

#------------------------------------------------------------------------------
# Display project option summary.
#------------------------------------------------------------------------------
sub summarize_project_options {
    print <<"EOT";
Mode:      project
Name:      $main::opt_name
Output:    $main::opt_output
Extension: $main::opt_projext
EOT
    print <<"EOT" if defined($main::opt_fragment);
Fragment:  $main::opt_fragment
EOT
    print <<"EOT";
Template:  $main::opt_template
Project:   $main::opt_projname
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
    summarize_workspace_options() if $main::opt_workspace;
    summarize_project_options() if $main::opt_project;
    print "\n";
}

#------------------------------------------------------------------------------
# Validate project options.
#------------------------------------------------------------------------------
sub validate_project_options {
    usage_error("The --depend option can be used only with --fragment.")
	if !defined($main::opt_fragment) and @main::opt_depend;
    usage_error("Must specify a template type.") unless $main::opt_template;
    usage_error("Unrecognized template type.")
	unless $main::targets->{$main::opt_template};
    usage_error("Must specify a name for the project.") unless $main::opt_name;
    usage_error("Must specify --projext.")
    	unless $main::opt_projext;
}

#------------------------------------------------------------------------------
# Validate workspace options.
#------------------------------------------------------------------------------
sub validate_workspace_options {
    usage_error("The --template option can be used only with --project.")
	if $main::opt_template;
    usage_error("The --project option can be used only with --project.")
	if $main::opt_project;
    usage_error("The --target option can be used only with --project.")
	if $main::opt_target;
    usage_error("The --library option can be used only with --project.")
	if @main::opt_library;
    usage_error("The --lflags option can be used only with --project.")
	if @main::opt_lflags;
    usage_error("The --cflags option can be used only with --project.")
	if @main::opt_cflags;
    usage_error("The --fragment option can be used only with --project.")
	if defined($main::opt_fragment);
    usage_error("The --depend option can be used only with --project.")
	if @main::opt_depend;
    usage_error("Must specify --name or --output.")
	unless $main::opt_name or $main::opt_output;
    usage_error("Must specify --name or --output, but not both.")
	if $main::opt_name and $main::opt_output;
    usage_error("Must specify --projext and --wsext.")
    	unless $main::opt_projext and $main::opt_wsext;
}

#------------------------------------------------------------------------------
# Validate options.
#------------------------------------------------------------------------------
sub validate_options {
    usage_error("Must specify --workspace or --project.")
	unless $main::opt_workspace or $main::opt_project;
    usage_error("Must specify --workspace or --project, but not both.")
	if $main::opt_workspace and $main::opt_project;

    validate_workspace_options() if $main::opt_workspace;
    validate_project_options() if $main::opt_project;
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
    $main::opt_project = 1 if $main::opt_p;
    $main::opt_workspace = 1 if $main::opt_w;
    $main::opt_projext = $main::opt_px unless $main::opt_projext;
    $main::opt_wsext = $main::opt_wx unless $main::opt_wsext;
    $main::opt_name = $main::opt_N unless $main::opt_name;
    $main::opt_target = $main::opt_g unless $main::opt_target;
    $main::opt_template = $main::opt_t unless $main::opt_template;
    $main::opt_template_dir = $main::opt_T unless $main::opt_template_dir;
    push(@main::opt_library, @main::opt_L);
    push(@main::opt_delaylib, @main::opt_Y);
    push(@main::opt_lflags,  @main::opt_l);
    push(@main::opt_depend,  @main::opt_D);
    $main::opt_htmlents = 1 if $main::opt_H;
}

#------------------------------------------------------------------------------
# Process DSP command-line options.
#------------------------------------------------------------------------------
sub process_project_options {
    add_suffix($main::opt_output, $main::opt_projext);
    ($main::makefile = basename($main::opt_output)) =~ s/(?i)\.dsp$/\.mak/;
    $main::opt_project = $main::opt_name unless $main::opt_project;

    my $target_ext = $main::targets->{$main::opt_template}->{'suffix'};
    $main::opt_target = $main::opt_name unless $main::opt_target;
    add_suffix($main::opt_target, $target_ext) if $target_ext;

    $main::opt_fragment = $main::opt_name
	if defined($main::opt_fragment) and !$main::opt_fragment;

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
	remove_suffix($depend, $main::opt_projext);
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
sub process_workspace_options {
    add_suffix($main::opt_output, $main::opt_wsext);

   my $fragment;
    foreach $fragment (@ARGV) {
	my $pji_frag = $fragment;    	
	add_suffix($pji_frag, 'pji');
	push(@main::pji_fragments, $pji_frag);
	
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
Visual C++ workspace and project project files from a set of templates.

Project files and workspaces can be built for both MSVC 6 and 7, given the 
appropriate templates. The command line options are the same except for the 
following: --projext and --wsext set the suffixes appended to the project and 
workspace file names. for MSVC 6, those are 'dsp'/'dsw' and 'vcproj'/'sln' for 
MSVC 7 (without quotes and dot). When specifying --htmlents, in all the 
strings that appear in project files ", < and > are replaced with the HTML 
entities &quot;, &lt; and &gt; as needed for the XML'ish MSVC 7 project files.

A project file is generated when --project is specified.  The type of project 
represented by the project file is selected using the --template option.  The 
template can be one of "appcon", "plugin", "library", or "group"; which 
represent a console application, a dynamic link library (DLL), a static 
library (LIB), or an group project, respectively. The "group" project type is 
used for creating pseudo-dependency targets within a workspace but does not 
actually generate any resources.

Template files are loaded from the current working directory or from a named
template directory.  The template files appcon.tpl, plugin.tpl,
library.tpl, and group.tpl correspond to the project types available via the
--template option.  Template files may contain the variables \%project\%,
\%target\%, \%makefile\%, \%groups\%, \%libs\%, \%lflags\%, and \%cflags\%.
The variables \%project\%, \%target\%, \%libs\%, \%flags\%, and \%cflags\%
are replaced with values specified via the --projname, --target, --library,
--lflags, and --cflags options, respectively.  The replacement value for
\%makefile\% is the same as the name of the generated project file with the
exception that .mak is substituted for the suffix.

The template prjgroup.tpi is used multiple times to build a value for the
\%groups\% variable mentioned above.  This template is used to create the
file groups "Source Files", "Header Files", and "Resource Files", as needed,
within the generated project file.  This template may contain the variables
\%group\% and \%files\%.  The \%group\% variable is automatically replaced
by the name of the group being generated (for instance "Source Files").

The template prjfile.tpi is used multiple times to build a value for the
\%files\% variable mentioned above.  This template is used to specify each
file which makes up a file group (represented by dspgroup.tpi).  This
template may contain the variable \%file\% which represents a file name
provided as an argument to this script during DSP generation.

During project file generation, a couple of workspace fragment files (for 
project, configuration, and dependency information) can also be generated with 
the --fragment option and zero or more --depend options. The generated 
fragment files can later be used to create a complete workspace file 
containing all projects, configuration information and dependency graph for 
the entire project (That is, to create a complete, valid workspace). Note: 
MSVC 6 projects only need the dependency information; the templates for the 
other fragments can be empty.

The dependency fragment file is created from the template dswgroup.tpi.
This template may contain the variables \%project\%, \%dsp\%, and
\%depends\%.  The \%project\% variable has the same meaning as it does when
used with the templates specified via --template.  The \%dsp\% variable is
replaced by the name of the generated DSP file (see --output), except that
the directory portion of the path is stripped off.

The project fragment file is created from the template wsgroup.tpi. This 
template may contain the variables \%project\%, \%projfile\%, and \%guid\%. 
The \%project\% variable has the same meaning as it does when used with the 
templates specified via --template.  The \%projfile\% variable is replaced by 
the name of the generated project file (see --output), except that the 
directory portion of the path is stripped off. \%guid\% contains a unique 
identifier every project in a MSVC7 Solution must have. 

To create configuration fragments, the template wscfg.tpi is used. The only 
variable it may contain is \%guid\%.

The template wsdep.tpi is used multiple times to build a value for the 
\%depends\% variable mentioned above.  This template is used to specify each 
project guid which makes up a dependency group (represented by wsgroup.tpi) 
for a specific project  This template may contain the variable \%guid\%, 
\%depguid\%, \%depnum\% and \%depend\%. \%depguid\% represents a project guid 
upon which the generated project file depends (see --depend), \%depend\% is 
this project's name. \%depnum\% is simply a counter, beginning with 0.

Finally, a workspace file is generated when --workspace is specified. The SLN 
file is created by merging the contents of fragment files into the template 
ws.tpl.  This template may contain the variable \%groups\%, \%depends\% and 
\%configs\%, which are replaced by the collected contents of all fragments 
named as arguments to this script during workspace generation.

Usage: $PROG_NAME [options] <file> ...

Global Options:
    -p --project     Generate a project file.
    -w --workspace   Generate a workspace file.
    -px --projext    Suffix for project files.
    -wx --wsext	     Suffix for workspace files.
    -H --htmlents    Insert HTML entities in strings in project files.
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

Project Options:
<file> Path of a file which belongs to the project represented by this project.
       Any number of files may be specified, or none if the project contains
       no files.  Files with the suffixes .c, .cc, and .cpp are placed in
       the project's "Source Files" group; files with the suffix .h are
       placed in the "Header Files" group; and all other files are placed in
       the "Resource Files" group.  Each mentioned file replaces the
       variable \%file\% in the prjfile.tpi template.

    -N <name>
    --name=<name>
                 The basic name associated with this project file.  This name 
                 is used for automatic generation of other required names (such
                 as output name, target name, fragment name) if those names
                 are not explicitly specified.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated project file.  A 
                 suffix is automatically appended if absent.  If not
                 specified, then the name given with --name is used as the
                 output name and the file is written to the current working
                 directory.
    -t <type>
    --template=<type>
                 Specifies the template type for this project.  The type may be
                 one of "appcon", "plugin", "library", or "group".
                 See the discussion of project generation (above) for an
                 explanation of the various template types.
    -p <name>
    --projname=<name>
                 Specifies the display name of the project for the Microsoft
                 Visual C++ IDE.  This is the replacement value for the
                 \%project\% variable in template files.  If not specified,
                 then the name given with --name is used as the project name.
    -g <name>
    --target=<name>
                 Specifies the name of the actual target generated by this
                 project.  This is the replacement value for the \%target\%
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
    -Y <name>
    --delayload=<name>
    		 Specified the name of a DLL which should be delay loaded.
    		 In prjdelay.tpi, \%delaylib\% is replaced with this value. 
    		 All resulting strings are concatenated and replaced for
    		 \%delaylibs\% in the project template.
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
                 Specifies whether or not to generate workspace fragment files 
                 along with the project file.  Fragment files list the other 
                 projects upon which this project relies and some other stuff 
                 needed for the solution.  If not specified, then no fragments 
                 are generated.  If specified, but no path is given, then the 
                 name given with --name is used as the fragment name. Fragment 
                 files use the suffixes .frag, .frag.pji, .frag.cfi and 
                 .frag.dpi.  This suffixes are added automatically if absent. 
                 Generated fragments can later be incorporated into a 
                 workspace file to collectively define a complete solution for 
                 the entire project. each dependency specified with the --
                 depend option is listed in the generated fragments.
    -D <project>
    --depend=<project>
                 Specifies the name of a project upon which this project 
                 depends. Each project name is written to the workspace 
                 dependency fragment file for this project and is the 
                 replacement value for the \%depend\% variable in the 
                 wsdep.tpi template.  The --depend option may be specified any 
                 number of times, or not at all if the project has no 
                 dependencies (which is often the case for "library" 
                 projects).  This option can only be used in conjunction with 
                 the --fragment option.
                 
Workspace Options:
<file> Path of a fragment file emitted during a project generation phase
       See the discussion of fragment file generation (above) for details.

    -N <name>
    --name=<name>
                 Specifies the base name of the generated workspace file.  A 
                 suffix is automatically appended to generate the final output 
                 name and the file is created in the current working 
                 directory. This option and the --output option are mutually 
                 exclusive.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated DSW file.  A suffix 
                 is automatically appended if absent.  This option and the --
                 name option are mutually exclusive.
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
    process_workspace_options() if $main::opt_workspace;
    process_project_options() if $main::opt_project;
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
create_project() if $main::opt_project;
create_workspace() if $main::opt_workspace;
