#!/usr/bin/perl -w
#==============================================================================
#
#    Microsoft Visual C++ project and workspace file generator.
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
my $PROG_VERSION = 9;
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000-2004 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

$main::opt_project = 0;
$main::opt_p = 0;	# Alias for 'project'.
$main::opt_project_extension = '';
$main::opt_P = '';	# Alias for 'project-extension'.
$main::opt_workspace = 0;
$main::opt_w = 0;	# Alias for 'workspace'.
$main::opt_workspace_extension = '';
$main::opt_W = '';	# Alias for 'workspace-extension'.
$main::opt_name = '';
$main::opt_N = '';	# Alias for 'name'.
$main::opt_template = '';
$main::opt_t = '';	# Alias for 'template'.
$main::opt_project_name = '';
$main::opt_xml_protect = 0;
$main::opt_X = 0;	# Alias for 'xml-protect'.
$main::opt_target = '';
$main::opt_g = '';	# Alias for 'target'.
$main::opt_meta_file = '';
$main::opt_M = '';	# Alias for 'meta-file'.
@main::opt_library = ();
@main::opt_debuglibrary = ();
@main::opt_L = ();	# Alias for 'library'.
@main::opt_delaylib = ();
@main::opt_Y = ();	# Alias for 'delaylib'.
@main::opt_lflags = ();
@main::opt_debuglflags = ();
@main::opt_l = ();	# Alias for 'lflags'.
@main::opt_cflags = ();
@main::opt_debugcflags = ();
$main::opt_output = '';
$main::opt_fragment = undef;
@main::opt_depend = ();
@main::opt_D = ();	# Alias for 'depend'.
$main::opt_template_dir = '';
$main::opt_T = '';	# Alias for 'template-dir'.
@main::opt_strip_root = ();
@main::opt_S = ();	# Alias for 'strip-root'.
@main::opt_accept = ();
@main::opt_a = ();	# Alias for 'accept'.
@main::opt_reject = ();
@main::opt_r = ();	# Alias for 'reject'.
@main::opt_response_file = ();
@main::opt_R = ();	# Alias for 'response-file'.
$main::opt_source_root = '.';
$main::opt_build_root = '.';
$main::opt_verbose = 0;
$main::opt_v = 0;	# Alias for 'verbose'.
$main::opt_quiet = 0;
$main::opt_version = 0;
$main::opt_V = 0;	# Alias for 'version'.
$main::opt_help = 0;

my @script_options = (
    'project',
    'p',		# Alias for 'project'.
    'project-extension=s',
    'P=s', 		# Alias for 'project-extension'.
    'workspace',
    'w',		# Alias for 'workspace'.
    'workspace-extension=s',
    'W=s',		# Alias for 'workspace-extension'.
    'name=s',
    'N=s',		# Alias for 'name'.
    'template=s',
    't=s',		# Alias for 'template'.
    'project-name=s',
    'xml-protect',
    'X',		# Alias for 'xml-protect'.
    'target=s',
    'g=s',		# Alias for 'target'.
    'meta-file=s',
    'M=s',		# Alias for 'meta-file'.
    'library=s@',
    'debuglibrary=s@',
    'L=s@',		# Alias for 'library'.
    'delaylib=s@',
    'Y=s@',		# Alias for 'delaylib'.
    'lflags=s@',
    'debuglflags=s@',
    'l=s@',		# Alias for 'lflags'.
    'cflags=s@',
    'debugcflags=s@',
    'output=s',
    'fragment:s',
    'depend=s@',
    'D=s@',		# Alias for 'depend'.
    'template-dir=s',
    'T=s',		# Alias for 'template-dir'.
    'strip-root=s@',
    'S=s@',		# Alias for 'strip-root'.
    'accept=s@',
    'a=s@',		# Alias for 'accept'.
    'reject=s@',
    'r=s@',		# Alias for 'reject'.
    'response-file=s@',
    'R=s@',		# Alias for 'response-file'.
    'source-root=s',
    'build-root=s',
    'verbose!',
    'v!',		# Alias for 'verbose'.
    'quiet!',
    'version',
    'V',		# Alias for 'version'.
    'help'
);

$main::verbosity = 0;
$main::accept_patterns = '';
$main::reject_patterns = '';
$main::makefile = '';
$main::guid = '';
$main::groups = {};
@main::pjf_fragments = ();
@main::dpf_fragments = ();
@main::cff_fragments = ();

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
	'pattern' => '(?i)\.(c|cc|cpp|C|m|mm|M)$'
    },
    'headers'     => {
	'name'    => 'Header Files',
	'pattern' => '(?i)\.(h|hpp|H)$'
    },
    'resources'   => {
	'name'    => 'Resource Files'
    }
};

$main::targets = {
    'appgui' => {
	'suffix' => 'exe'
    },
    'appcon' => {
	'suffix' => 'exe'
    },
    'plugin' => {
	'suffix' => 'dll'
    },
    'library' => {
	'suffix' => 'lib'
    },
    'group' => {
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
# Create a unique GUID from a project name.
#------------------------------------------------------------------------------
sub guid_from_name {
    my $projname = shift;
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
# Protect XML special characters with appropriate XML entity references.
#------------------------------------------------------------------------------
sub xmlize {
    my $result = shift;
    $result =~ s/\"/\&quot\;/g;
    $result =~ s/\</\&lt\;/g;
    $result =~ s/\>/\&gt\;/g;
    return $result;
}

#------------------------------------------------------------------------------
# Protect XML special characters with appropriate XML entity references if the
# --xml-protect option was specified.  If the value is an array reference
# rather than a string, then convert the elements to a single string delimited
# by spaces before applying the XML translation.
#------------------------------------------------------------------------------
sub xmlprotect {
    my $result = shift;
    $result = join(' ',@{$result}) if ref($result) and ref($result) eq 'ARRAY';
    return $main::opt_xml_protect ? xmlize($result) : $result;
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
	$main::project_group_template = load_template('prjgroup', 'tpi');
	$main::project_file_template = load_template('prjfile', 'tpi');
	$main::project_delaylib_template = load_template('prjdelay', 'tpi');
	$main::workspace_project_template = load_template('wsgroup', 'tpi');
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
    my $files = $main::groups->{$group};
    my $files_buffer =
	interpolate_items($files, '%file%', $main::project_file_template);
    interpolate('%name%', $main::opt_name, \$files_buffer);
    interpolate('%sourceroot%', $main::opt_source_root, \$files_buffer);
    interpolate('%buildroot%', $main::opt_build_root, \$files_buffer);
    interpolate('%metafile%', $main::opt_meta_file, \$files_buffer);

    my $group_name = $main::patterns->{$group}->{'name'};
    my $result = $main::project_group_template;
    interpolate('%name%', $main::opt_name, \$result);
    interpolate('%sourceroot%', $main::opt_source_root, \$result);
    interpolate('%buildroot%', $main::opt_build_root, \$result);
    interpolate('%metafile%', $main::opt_meta_file, \$result);
    interpolate('%group%', $group_name, \$result);
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
    my $delaylibs = '';
    my $delaylib;
    foreach $delaylib (@main::opt_delaylib) {
    	$delaylibs .= $main::project_delaylib_template;
    	interpolate('%name%', $main::opt_name, \$delaylibs);
    	interpolate('%delaylib%', $delaylib, \$delaylibs);
    }
    interpolate('%name%',      	xmlprotect($main::opt_name),        \$result);
    interpolate('%upcasename%',	xmlprotect(uc($main::opt_name)),    \$result);
    interpolate('%sourceroot%', xmlprotect($main::opt_source_root), \$result);
    interpolate('%buildroot%',  xmlprotect($main::opt_build_root),  \$result);
    interpolate('%project%',   	xmlprotect($main::opt_project_name),\$result);
    interpolate('%makefile%',   xmlprotect($main::makefile),        \$result);
    interpolate('%target%',     xmlprotect($main::opt_target),      \$result);
    interpolate('%metafile%',   xmlprotect($main::opt_meta_file),   \$result);
    interpolate('%libs%',      	xmlprotect(\@main::opt_library),    \$result);
    interpolate('%debuglibs%', 	xmlprotect(\@main::opt_debuglibrary),\$result);
    interpolate('%delaylibs%', 	xmlprotect($delaylibs), 	    \$result);
    interpolate('%lflags%', 	xmlprotect(\@main::opt_lflags),     \$result);
    interpolate('%debuglflags%',xmlprotect(\@main::opt_debuglflags),\$result);
    interpolate('%cflags%', 	xmlprotect(\@main::opt_cflags),     \$result);
    interpolate('%debugcflags%',xmlprotect(\@main::opt_debugcflags),\$result);
    interpolate('%groups%',     interpolate_project_groups(),       \$result);
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
	interpolate('%name%', $main::opt_name, \$buffer);
	interpolate('%sourceroot%', $main::opt_source_root, \$buffer);
	interpolate('%buildroot%', $main::opt_build_root, \$buffer);
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
    interpolate('%name%', $main::opt_name, \$result);
    interpolate('%sourceroot%', $main::opt_source_root, \$result);
    interpolate('%buildroot%', $main::opt_build_root, \$result);
    interpolate('%project%', $main::opt_project_name, \$result);
    interpolate('%projfile%', basename($main::opt_output), \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    interpolate('%guid%', $main::guid, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Build the contents of workspace configuration group fragment.
#------------------------------------------------------------------------------
sub interpolate_ws_config {
    my $result = $main::workspace_config_template;
    interpolate('%name%', $main::opt_name, \$result);
    interpolate('%sourceroot%', $main::opt_source_root, \$result);
    interpolate('%buildroot%', $main::opt_build_root, \$result);
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
    foreach $fragment (sort(@main::pjf_fragments)) {
	$proj_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::dpf_fragments)) {
	$depends_buffer .= load_file($fragment);
    }
    foreach $fragment (sort(@main::cff_fragments)) {
	$config_buffer .= load_file($fragment);
    }
    my $result = $main::workspace_template;
    interpolate('%name%', $main::opt_name, \$result);
    interpolate('%sourceroot%', $main::opt_source_root, \$result);
    interpolate('%buildroot%', $main::opt_build_root, \$result);
    interpolate('%projects%', $proj_buffer, \$result);
    interpolate('%depends%', $depends_buffer, \$result);
    interpolate('%configs%', $config_buffer, \$result);
    return $result;
}

#------------------------------------------------------------------------------
# Create a DSP/VCPROJ file and optionally a DSW/SLN dependency fragment file.
#------------------------------------------------------------------------------
sub create_project {
    save_file($main::opt_output, interpolate_project());
    print "Generated: $main::opt_output\n" unless quiet();

    if (defined($main::opt_fragment)) {
    	$main::guid = guid_from_name($main::opt_project_name);

    	my $dependencies = interpolate_ws_dependency();
    	my $dpf_frag = $main::opt_fragment;
    	add_suffix ($dpf_frag, 'dpf');
	save_file($dpf_frag, $dependencies);
	print "Generated: $dpf_frag\n" unless quiet();

    	my $pjf_frag = $main::opt_fragment;
    	add_suffix ($pjf_frag, 'pjf');
	save_file($pjf_frag, interpolate_ws_project($dependencies));
	print "Generated: $pjf_frag\n" unless quiet();

    	my $cff_frag = $main::opt_fragment;
    	add_suffix ($cff_frag, 'cff');
	save_file($cff_frag, interpolate_ws_config());
	print "Generated: $cff_frag\n" unless quiet();
    }
}

#------------------------------------------------------------------------------
# Create a DSW/SLN workspace file.
#------------------------------------------------------------------------------
sub create_workspace {
    save_file($main::opt_output, interpolate_workspace());
    print "Generated: $main::opt_output\n\n" unless quiet();
}

#------------------------------------------------------------------------------
# Display DSW/SLN option summary.
#------------------------------------------------------------------------------
sub summarize_workspace_options {
    print "Mode:      workspace\n";
    print "Name:      $main::opt_name\n" if $main::opt_name;
    print "Output:    $main::opt_output\n";
    print "Extension: $main::opt_workspace_extension\n";
}

#------------------------------------------------------------------------------
# Display project option summary.
#------------------------------------------------------------------------------
sub summarize_project_options {
    print <<"EOT";
Mode:        project
Name:        $main::opt_name
Output:      $main::opt_output
Extension:   $main::opt_project_extension
EOT
    print <<"EOT" if $main::opt_source_root;
Source root: $main::opt_source_root
EOT
    print <<"EOT" if $main::opt_build_root;
Build root:  $main::opt_build_root
EOT
    print <<"EOT" if defined($main::opt_fragment);
Fragment:    $main::opt_fragment
EOT
    print <<"EOT";
Template:    $main::opt_template
Project:     $main::opt_project_name
Target:      $main::opt_target
Makefile:    $main::makefile
EOT
    print <<"EOT" if $main::opt_meta_file;
Libraries:   $main::opt_meta_file
EOT
    print <<"EOT" if @main::opt_library;
Libraries:   @main::opt_library
EOT
    print <<"EOT" if @main::opt_lflags;
LFLAGS:      @main::opt_lflags
EOT
    print <<"EOT" if @main::opt_cflags;
CFLAGS:      @main::opt_cflags
EOT
    my @groups = sort(keys(%{$main::groups}));
    print <<"EOT" if @groups;
Groups:      @groups
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
    usage_error("Must specify --project-extension.")
	unless $main::opt_project_extension;
    usage_error("The option --workspace-extension can be used only with " .
	"--workspace") if $main::opt_workspace_extension;
}

#------------------------------------------------------------------------------
# Validate workspace options.
#------------------------------------------------------------------------------
sub validate_workspace_options {
    usage_error("The --template option can be used only with --project.")
	if $main::opt_template;
    usage_error("The --target option can be used only with --project.")
	if $main::opt_target;
    usage_error("The --meta-file option can be used only with --project.")
	if $main::opt_meta_file;
    usage_error("The --library option can be used only with --project.")
	if @main::opt_library;
    usage_error("The --debuglibrary option can be used only with --project.")
	if @main::opt_debuglibrary;
    usage_error("The --lflags option can be used only with --project.")
	if @main::opt_lflags;
    usage_error("The --debuglflags option can be used only with --project.")
	if @main::opt_debuglflags;
    usage_error("The --cflags option can be used only with --project.")
	if @main::opt_cflags;
    usage_error("The --debugcflags option can be used only with --project.")
	if @main::opt_debugcflags;
    usage_error("The --fragment option can be used only with --project.")
	if defined($main::opt_fragment);
    usage_error("The --depend option can be used only with --project.")
	if @main::opt_depend;
    usage_error("The --delay-lib option can be used only with --project.")
	if @main::opt_delaylib;
    usage_error("The --strip-root option can be used only with --project.")
	if @main::opt_strip_root;
    usage_error("The --source-root option can be used only with --project.")
        if @main::opt_source_root;
    usage_error("The --build-root option can be used only with --project.")
        if @main::opt_build_root;
    usage_error("Must specify --name or --output.")
	unless $main::opt_name or $main::opt_output;
    usage_error("Must specify --name or --output, but not both.")
	if $main::opt_name and $main::opt_output;
    usage_error("Must specify --workspace-extension.")
    	unless $main::opt_workspace_extension;
    usage_error("The option --project-extension can be used only with " .
	"--project") if $main::opt_project_extension;
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
# Given an array of pattern strings, synthesize a regular expression which
# checks all patterns in parallel.  If the pattern array is empty, the
# $fallback_pattern is returned.
#------------------------------------------------------------------------------
sub synthesize_pattern {
    my ($patterns, $fallback_pattern) = @_;
    my $composite;
    foreach my $pattern (@{$patterns}) {
	$composite .= '|' if $composite;
	$composite .= $pattern;
    }
    $composite = $fallback_pattern unless $composite;
    return $composite;
}

#------------------------------------------------------------------------------
# Process options which apply globally (workspace or project mode).
#------------------------------------------------------------------------------
sub process_global_options {
    $main::verbosity =  1 if $main::opt_verbose;
    $main::verbosity = -1 if $main::opt_quiet;
    $main::opt_template_dir = '.' unless $main::opt_template_dir;
    $main::accept_patterns = synthesize_pattern(\@main::opt_accept, '.+');
    $main::reject_patterns = synthesize_pattern(\@main::opt_reject, '^$');
}

#------------------------------------------------------------------------------
# Process option aliases.
#------------------------------------------------------------------------------
sub process_option_aliases {
    $main::opt_verbose = 1 if $main::opt_v;
    $main::opt_version = 1 if $main::opt_V;
    $main::opt_project = 1 if $main::opt_p;
    $main::opt_workspace = 1 if $main::opt_w;
    $main::opt_project_extension = $main::opt_P
	unless $main::opt_project_extension;
    $main::opt_workspace_extension = $main::opt_W
	unless $main::opt_workspace_extension;
    $main::opt_name = $main::opt_N unless $main::opt_name;
    $main::opt_target = $main::opt_g unless $main::opt_target;
    $main::opt_meta_file = $main::opt_M unless $main::opt_meta_file;
    $main::opt_template = $main::opt_t unless $main::opt_template;
    $main::opt_template_dir = $main::opt_T unless $main::opt_template_dir;
    $main::opt_xml_protect = 1 if $main::opt_X;
    push(@main::opt_library, @main::opt_L);
    push(@main::opt_delaylib, @main::opt_Y);
    push(@main::opt_lflags, @main::opt_l);
    push(@main::opt_depend, @main::opt_D);
    push(@main::opt_strip_root, @main::opt_S);
    push(@main::opt_accept, @main::opt_a);
    push(@main::opt_reject, @main::opt_r);
    push(@main::opt_response_file, @main::opt_R);
}

#------------------------------------------------------------------------------
# Filter an input list based upon --accept and --reject options.
#------------------------------------------------------------------------------
sub filter {
    return grep(/$main::accept_patterns/i && !/$main::reject_patterns/i, @_);
}

#------------------------------------------------------------------------------
# Massage paths from the command-line by stripping roots specified via
# --strip-root and by translating forward slashes to backward slashes.
#------------------------------------------------------------------------------
sub massage_paths {
    my @infiles = @_;
    my @outfiles;
    foreach my $file (@infiles) {
	$file =~ tr:/:\\:;
	foreach my $root (@main::opt_strip_root) {
	    last if $file =~ s/^$root//;
	}
	push(@outfiles, $file);
    }
    return @outfiles;
}

#------------------------------------------------------------------------------
# Read a response file and return a list of the contained items.
#------------------------------------------------------------------------------
sub read_response_file {
    my $path = shift;
    my $line;
    my @items;
    open(FILE, "<$path") or fatal("Unable to open response file $path: $!");
    while ($line = <FILE>) {
	$line =~ s/^\s+//;
	$line =~ s/#.*$//;
	$line =~ s/\s+$//;
	next if $line =~ /^$/;
	push(@items, $line);
    }
    close(FILE);
    return @items;
}

#------------------------------------------------------------------------------
# Return a list of input files specified as arguments on the command-line and
# via response files.
#------------------------------------------------------------------------------
sub input_files {
    my @items;
    my $response_file;
    foreach $response_file (@main::opt_response_file) {
	push(@items, read_response_file($response_file));
    }
    push(@items, @ARGV);
    return @items;
}

#------------------------------------------------------------------------------
# Process DSP/VCPROJ project command-line options.
#------------------------------------------------------------------------------
sub process_project_options {
    $main::opt_output = $main::opt_name unless $main::opt_output;
    add_suffix($main::opt_output, $main::opt_project_extension);
    ($main::makefile = basename($main::opt_output)) =~ s/(?i)\.dsp$/\.mak/;
    $main::opt_project = $main::opt_name unless $main::opt_project;

    my $target_ext = $main::targets->{$main::opt_template}->{'suffix'};
    $main::opt_target = $main::opt_name unless $main::opt_target;
    add_suffix($main::opt_target, $target_ext) if $target_ext;

    $main::opt_project_name = $main::opt_name unless $main::opt_project_name;

    $main::opt_fragment = $main::opt_name
	if defined($main::opt_fragment) and !$main::opt_fragment;

    my @libraries;
    my $library;
    foreach $library (@main::opt_library) {
	add_suffix($library, 'lib');
	push(@libraries, $library);
    }
    @main::opt_library = @libraries;

    my %depends;
    my $depend;
    foreach $depend (filter(@main::opt_depend)) {
	remove_suffix($depend, $main::opt_project_extension);
	if (!exists ($depends{$depend})) { $depends{$depend} = 1; }
    }
    @main::opt_depend = keys(%depends);

    $main::opt_source_root =~ tr:/:\\:;
    $main::opt_build_root =~ tr:/:\\:;

    my @roots;
    my $root;
    foreach $root (@main::opt_strip_root) {
	$root =~ tr:/:\\:;
	push(@roots, quotemeta($root));
    }
    @main::opt_strip_root = @roots;

    my @files = massage_paths(filter(input_files()));
    ($main::opt_meta_file) = massage_paths($main::opt_meta_file);
    if ($main::opt_meta_file) {
	my $metafile_rx = quotemeta($main::opt_meta_file);
	push(@files, $main::opt_meta_file) if (!grep(/$metafile_rx/, @files));
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
# Process DSW/SLN workspace command-line options.
#------------------------------------------------------------------------------
sub process_workspace_options {
    $main::opt_name = $main::opt_output unless $main::opt_name;
    remove_suffix($main::opt_name, $main::opt_workspace_extension);
    $main::opt_output = $main::opt_name unless $main::opt_output;
    add_suffix($main::opt_output, $main::opt_workspace_extension);

    my $fragment;
    foreach $fragment (filter(input_files())) {
	my $pjf_frag = $fragment;
	add_suffix($pjf_frag, 'pjf');
	push(@main::pjf_fragments, $pjf_frag);

	my $dpf_frag = $fragment;
	add_suffix($dpf_frag, 'dpf');
	push(@main::dpf_fragments, $dpf_frag);

	my $cff_frag = $fragment;
	add_suffix($cff_frag, 'cff');
	push(@main::cff_fragments, $cff_frag);
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
appropriate templates.  The command line options are the same for each version.
The MSVC version for which to generate files is controlled by providing a
suitable set of templates via --template-dir; and via --project-extension and
--workspace-extension to set the filename suffixes for the generated files.
For MSVC 6, 'dsp' should be used as the suffix for project files, and 'dsw' for
workspace files.  For MSVC 7, use 'vcproj' and 'sln' for project and workspace
files, respectively.  The --xml-protect option should be used when creating
MSVC 7 project files in order to ensure that special characters (such as ", <,
and >) get encoded via XML character references.  This is required since MSVC 7
files are stored in XML format.

A project file is generated when --project is specified.  The type of project
represented by the project file is selected using the --template option.  The
template can be one of "appgui", "appcon", "plugin", "library", or "group";
which represent a GUI application, a console application, a dynamic link
library (DLL), a static library (LIB), or a group project, respectively.  The
"group" project type is used for creating pseudo-dependency targets within a
workspace but does not actually generate any resources.

Template files are loaded from the current working directory or from the
directory named via --template-dir.  The template files appgui.tpl, appcon.tpl,
plugin.tpl, library.tpl, and group.tpl correspond to the project types
available via the --template option.  Template files may contain the variables
\%name\%, \%project\%, \%target\%, \%metafile\%, \%makefile\%, \%groups\%,
\%libs\%, \%debuglibs\%, %delaylibs%, \%lflags\%, \%debuglflags\%, \%cflags\%
and \%debugcflags\%.  The variables \%name\%, \%project\%, \%target\%,
\%libs\%, \%debuglibs\%, \%cflags\%, \%debugcflags\%, \%lflags\%,
\%debuglflags\%, \%sourceroot\%, and \%buildroot\% are replaced with values
specified via the --name, --project-name, --target, --library, --debuglibrary,
--cflags, --debugcflags, --lflags, --debuglflags, --source-root, and
--build-root options, respectively.  The \%name\%, variable can be used in any
template file.  Additionally, \%sourceroot\%, and \%buildroot\% can be used in
any project-related (but not workspace-related) template.  The replacement
value for \%makefile\% is the same as the name of the generated project file
except that .mak is substituted for the suffix.

The template prjgroup.tpi is used multiple times to build a value for the
\%groups\% variable mentioned above.  This template is used to create the file
groups "Source Files", "Header Files", and "Resource Files", as needed, within
the generated project file.  This template may contain the variables \%group\%
and \%files\%, in addition to the variables available to all templates.  The
\%group\% variable is automatically replaced by the name of the group being
generated (for instance "Source Files").

The template prjfile.tpi is used multiple times to build a value for the
\%files\% variable mentioned above.  This template is used to specify each file
which makes up a file group (represented by prjgroup.tpi).  This template may
contain the variable \%file\% which represents a file name provided as an
argument to this script during DSP/VCPROJ generation, as well as the globally
available variables.

The template prjdelay.tpi is used multiple times to bulid a value for the
\%delaylibs\% variable mentioned above.  This template is used to specify each
delayed-load library given via the --delaylib option.  The entire content of
this template should be placed on a single line and should not end with a line
terminator.  This template may contain the variable \%delaylib\%, as well as
the globally available variables.  The value of \%delaylib\% will be a name of
a DLL specified with --delaylib.

During project file generation, a couple of workspace fragment files (for
project, configuration, and dependency information) can also be generated with
the --fragment option and zero or more --depend options.  The generated
fragment files can later be used to create a complete workspace file containing
all projects, configuration information, and dependency graph for the entire
project (to wit, to create a complete, valid workspace referencing all the
project files generated individually).  Note: MSVC 6 projects require only the
dependency information; the templates for the other fragments may be empty.

The project fragment file is created from the template wsgroup.tpi.  This
template may contain the variables \%project\%, \%projfile\%, \%depends\%, and
\%guid\%, as well as the globally available variables.  The \%project\%
variable has the same meaning as it does when used with the templates specified
via --template.  The \%projfile\% variable is replaced by the name of the
generated project file (see --output), except that the directory portion of the
path is stripped off.  The \%depends\% variable contains a collection of
inter-project dependency information for projects contained in the workspace.
The value of \%guid\% is a unique identifier which is required by every MSVC 7
project.  This value is composed automatically from the project name.

To create configuration fragments, the template wscfg.tpi is used.  It may
contain the variable \%guid\%, in addition to the globally available variables.
This template is required only by MSVC 7, but it must also be present when
creating MSVC 6 files, though it may left empty.

The template wsdep.tpi is used multiple times to build a value for the
\%depends\% variable mentioned above.  This template is used to specify
inter-project dependencies within a workspace file.  This template may contain
the variables \%guid\%, \%depguid\%, \%depnum\%, \%depend\%, and the globally
available variables.  \%guid\% is the unique project identifier discussed
above.  \%depguid\% is the unique project identifier of a project upon which
this project depends (see --depend), \%depend\% is the name of a project upon
which this project depends (see --depend).  Each project named by --depend is
also assigned a small number (starting at zero with the first --depend
encountered).  When processing the named dependency, the associated number is
available as \%depnum\%.

Finally, a workspace file is generated when --workspace is specified.  The
DSW/SLN file is created by merging the contents of fragment files into the
template ws.tpl.  This template may contain the variable \%projects\%,
\%depends\% and \%configs\%, which are replaced by the collected contents of
all project, dependency, and configuration fragments named as arguments to this
script for workspace synthesis.

Usage: $PROG_NAME [options] <file> ...

Global Options:
    -p --project Generate a project file.
    -w --workspace
                 Generate a workspace file.
    -X --xml-protect
                 Use XML character references in strings inserted into
                 generated files in place of "special" characters.
    -T <path>
    --template-dir=<path>
                 Specifies the directory where the template files reside.  If
                 not specified, then template files are assumed to exist in the
                 current working directory.
    -a <pattern>
    --accept=<pattern>
                 Specifies a Perl regular-expression used as a filter against
                 each named <file>.  Filenames which match the pattern will be
                 included in the generated workspace or project unless
                 overriden by --reject.  The --accept option may be given any
                 number of times in order to specify any number of patterns.
                 This is a useful option for clients unable to filter the list
                 filenames themselves.  Example: --accept='\\.cc\$'
    -r <pattern>
    --reject=<pattern>
                 Specifies a Perl regular-expression used as a filter against
                 each named <file>.  Filenames which match the pattern will not
                 be included in the generated workspace or project.
                 Reject-patterns override accept-patterns.  The --reject option
                 may be given any number of times in order to specify any
                 number of patterns.  This is a useful option for clients
                 unable to filter the list of filenames themselves.
                 Example: --reject='\\.txt\$'
    -R <path>
    --response-file=<path>
                 Specifies a file containing pathnames, one per line, which are
                 treated exactly as if they had been mentioned on the
                 command-line as <file>.  The --response-file option may be
                 given any number of times, and is allowed in combination with
                 <file> arguments actually specified on the command-line.
                 Comments in the response file begin with '#' and extend to the
                 end of line.
    -v --verbose Emit informational messages about the processing.  Can be
                 negated with --noverbose.  Deafult is --noverbose.
    -q --quiet   Suppress all output except for error messages.  Can be
                 negated with --noquiet.  Default is --noquiet.
    -V --version Display the version number of $PROG_NAME.
    -h --help    Display this usage message.

Project Options:
    <file>       Path of a file which belongs to the project represented by
                 this project.  Any number of files may be specified, or none
                 if the project contains no files.  Files with the suffixes .c,
                 .cc, .cpp, .C, .m, .mm, and .M are placed in the project's
                 "Source Files" group; files with the suffixes .h, .hpp, and .H
                 are placed in the "Header Files" group; and all other files
                 are placed in the "Resource Files" group.  Each mentioned file
                 replaces the variable \%file\% in the prjfile.tpi template.
    -N <name>
    --name=<name>
                 The basic name associated with this project file.  This name
                 is used for automatic generation of other required names (such
                 as output name, target name, fragment name) if those names are
                 not explicitly specified.  It is also available in template
                 files as the \%name\% variable.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated project file.  A
                 suffix (see --project-extension) is automatically appended if
                 absent.  If this option is not specified, then the name given
                 with --name (plus extension) is used as the output name and
                 the file is written to the current working directory.
    -P <ext>
    --project-extension=<ext>
                 Use <ext> as suffix for project file.  The extension 'dsp'
                 should be used for MSVC 6 project files; and 'vcproj' for
                 MSVC 7 project files.
    -t <type>
    --template=<type>
                 Specifies the template type for this project.  The type may be
                 one of "appgui", "appcon", "plugin", "library", or "group".
                 See the discussion of project generation (above) for an
                 explanation of the various template types.
    -p <name>
    --project-name=<name>
                 Specifies the display name of the project for the Microsoft
                 Visual C++ IDE.  This is the replacement value for the
                 \%project\% variable in template files.  If not specified,
                 then the name given with --name is used as the project name.
    -g <name>
    --target=<name>
                 Specifies the name of the actual target generated by this
                 project.  This is the replacement value for the \%target\%
                 variable in the template files.  For applications, this is the
                 name of the resultant executable (EXE); for plugins it is the
                 dynamic link library (DLL); and for libraries, it is the
                 static library (LIB).  If not specified, then the name given
                 with --name is used as the target name.  If the name does not
                 end with an appropriate suffix (one of .exe, .dll, or .lib),
                 then the suffix is added automatically.
    -M <path>
    --meta-file=<path>
                 Specifies the path of a file containing meta-data related to
                 this target.  This is the replacement value for the
                 \%metafile\% variable in template files.  For convenience, the
                 named file is also inserted into the "Resource Files" group
                 within the generated project file.  If not specified, then no
                 meta-data file is associated with the generated project.  The
                 meta-data file for a "plugin", for example, might contain
                 information about the interfaces published by the plugin
                 module.  How the project utilizes this information is specific
                 to the template, however one typical scheme is to copy the
                 meta-data file alongside the generated target.  It is common
                 for the meta-data file to have the same base name as the
                 generated target, but a different file extension.
    -L <name>
    --library=<name>
                 Specifies the name of an extra Windows library with which the
                 project should be linked in addition to those which are
                 already mentioned in the template file.  The named library
                 will become part of the replacement value for the \%libs\%
                 variable in the template files.  Typically, libraries are only
                 specified for executable and plug-in templates.  A .lib suffix
                 is added to the name automatically if absent.  The --library
                 option may be given any number of times to specify any number
                 of additional libraries, or not at all if no additional
                 libraries are required.  The --library option differs from the
                 --depend option in that it refers to libraries which exist
                 outside of the project graph, whereas --depend always refers
                 to projects which are members of the project graph.
    --debuglibrary=<name>
                 Specifies the name of an extra Windows library with which the
                 project should be linked in addition to those which are
                 already mentioned in the template file.  The named library
                 will become part of the replacement value for the
		 \%debuglibs\% variable in the template files. This can be used
		 if another set of libraries is to be used for debug purposes.
    -Y <name>
    --delaylib=<name>
                 Specifies the name of a DLL which should be delay-loaded.  In
                 prjdelay.tpi, \%delaylib\% is replaced with this value.  The
                 concatentation of each invocation of prjdelay.tpi becomes the
                 value of the \%delaylibs\% variable in the project template
                 (see --template).  The --delaylib option may be given any
                 number of times to specify any number of delay-loaded DLLs, or
                 not at all if no such DLLs are required.
    -l <flags>
    --lflags=<flags>
                 Specifies extra linker options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%lflags\% variable in
                 the project template files.  Typically, linker options are
                 only specified for executable and plug-in templates.  The
                 --lflags option may be specified any number of times, in which
                 case the effects are cumulative, or not at all if no extra
                 linker options are required.
    --debuglflags=<flags>
                 Specifies extra linker options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%debuglflags\% variable
		 in the project template files.  This can be used if another
		 set of flags is to be used for debug purposes.
    -c <flags>
    --cflags=<flags>
                 Specifies extra compiler options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%cflags\% variable in
                 the project template files.  The --cflags option may be
                 specified any number of times, in which case the effects are
                 cumulative, or not at all if no extra compiler options are
                 required.  As an example, a pre-processor macro named
                 __FOOBAR__ can be defined with: --cflags='/D "__FOOBAR__"'
    --cflags=<flags>
                 Specifies extra compiler options which should be used in
                 addition to those already mentioned in the template file.
                 This is the replacement value for the \%debugcflags\%
		 variable in the project template files.  This can be used
		 if another set of flags is to be used for debug purposes.
    --source-root=<path>
                 Specifies a (typically relative) path from the location where
                 the project files will reside back to the root of the source
                 tree in which they live. This is the replacement value for the
                 \%sourceroot\% variable in project template files.  This
                 information might be useful, for instance, when composing MSVC
                 /I directives for specifying header locations.  Example: /I
                 "%sourceroot\\include"
    --build-root=<path>
                 Specifies a (typically relative) path from the location where
                 the project files will reside to the root of the build tree in
                 which built targets may be placed.  This is the replacement
                 value for the \%buildroot\% variable in project template
                 files.  This information might be useful, for instance, when
                 composing MSVC post-build rules if built files need to be
                 copied to some final resting place.
    -f
    -f <path>
    --fragment
    --fragment=<path>
                 Specifies whether or not to generate workspace fragment files
                 along with the project file.  Fragment files list the other
                 projects upon which this project relies, as well as other
                 information needed to fully synthesize workspace files.  If
                 not specified, then no fragments are generated.  If specified,
                 but no path is given, then the name given with --name is used
                 as the fragment name.  Fragment files are given the suffixes
                 .pjf, .cff and .dpf.  The suffixes are added automatically to
                 the specified path.  Generated fragments can later be
                 incorporated into a workspace file collectively to define a
                 complete workspace for the entire project.  Each dependency
                 specified with the --depend option is listed in the generated
                 depedency fragment.
    -D <project>
    --depend=<project>
                 Specifies the name of a project upon which this project
                 depends.  Each project name is written to the workspace
                 dependency fragment file for this project, and is the
                 replacement value for the \%depend\% variable in the wsdep.tpi
                 template.  The --depend option may be specified any number of
                 times, or not at all if the project has no dependencies (which
                 is often the case for "library" projects).  Dependencies are
                 subject to filtering by --accept and --reject.  This option
                 can be used only in conjunction with the --fragment option.
    -S <prefix>
    --strip-root=<prefix>
                 It is generally wise for the source, header, and resource
                 files mentioned by the generated project file, and referenced
                 by the \%file\% interpolation variable, to be referenced by
                 paths relative to the root (or some other location within) the
                 project hierarchy, rather than by absolute paths.  This allows
                 the entire project source tree, along with the contained
                 project files, to be moved from location to location without
                 breakage.  Typically, this is accomplished by providing
                 relative pathnames for the files mentioned on the command-line
                 when the --project option is specified.  Alternately, if
                 absolute pathnames are given, then the --strip-root option can
                 be used to remove a prefix portion of each mentioned file.
                 The --strip-root option may be specified any number of times,
                 providing a different prefix on each occassion, or not at all,
                 if all mentioned files are specified via relative pathnames.

Workspace Options:
    <file>       Path of a fragment file emitted during a project generation
                 phase.  This is the basename for fragment files, as specified
                 via the --fragment option during project file generation.  The
                 name should not have a .pjf, .cff, or .dpf extension; the
                 appropriate extension will be added automatically to the
                 basename in order to locate the actual fragment files.  See
                 the discussion of fragment file generation (above) for
                 details.
    -N <name>
    --name=<name>
                 Specifies the base name of the generated workspace file.  A
                 suffix (see --workspace-extension) is automatically appended
                 to generate the final output name and the file is created in
                 the current working directory.  This option and the --output
                 option are mutually exclusive.
    -o <path>
    --output=<path>
                 Specifies the full path of the generated DSW/SLN workspace
                 file.  A suffix (see --workspace-extension) is automatically
                 appended if absent.  This option and the --name option are
                 mutually exclusive.
    -W <ext>
    --workspace-extension=<ext>
                 Use <ext> as suffix for workspace file.  The extension 'dsw'
                 should be used for MSVC 6 workspace files; and 'sln' for
                 MSVC 7 workspace files.
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
    print STDERR "ERROR: Use the --help option for instructions.\n";
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
