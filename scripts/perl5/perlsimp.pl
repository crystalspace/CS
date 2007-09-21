#!/usr/bin/perl
#
# This script is meant for testing and demonstration of the Crystal Space
# Perl bindings. It is a reimplementation of the simple1 application.

# Pragmas to enforce good coding style
use strict;
use warnings;

# Include directories where the Crystal Space Perl module might be found
BEGIN { unshift @INC,
	"$ENV{CRYSTAL}/scripts/perl5" if exists $ENV{CRYSTAL} }
use lib '/usr/local/share/crystalspace/bindings',
	'/usr/share/crystalspace/bindings',
	'./scripts/perl5';

# Load the Crystal Space Perl module and import symbols
use cspace qw(csInitializer csView csevKeyboardDown csevFrame
  csVector3 csColor csOrthoTransform csXRotMatrix3 csYRotMatrix3
  CS_QUERY_REGISTRY SCF_QUERY_INTERFACE CSDRAW_3DGRAPHICS CS_POLYRANGE_LAST);

# Initialize the Crystal Space environment
my $objreg = csInitializer::CreateEnvironment( [$0, @ARGV] );
die 'CreateEnvironment failed' unless defined $objreg;

# Load plugins into Crystal Space
csInitializer::RequestPlugins($objreg,
  cspace::CS_REQUEST_VFS(),
  cspace::CS_REQUEST_OPENGL3D(),
  cspace::CS_REQUEST_ENGINE(),
  cspace::CS_REQUEST_FONTSERVER(),
  cspace::CS_REQUEST_IMAGELOADER(),
  cspace::CS_REQUEST_LEVELLOADER(),
  cspace::CS_REQUEST_REPORTER(),
  cspace::CS_REQUEST_REPORTERLISTENER())
  or die 'RequestPlugins failed';

# If the commandline switch '-help' was passed, then print help and exit
if (cspace::csCommandLineHelper::CheckHelp($objreg))
{
  cspace::csCommandLineHelper::Help($objreg);
  exit 0;
}

# Register our event handler with the Crystal Space event queue
csInitializer::SetupEventHandler($objreg, \&HandleEvent,
[ 'crystalspace.input.keyboard', 'crystalspace.frame' ])
  or die 'SetupEventHandler failed';

# Finish initializing Crystal Space environment
csInitializer::OpenApplication($objreg)
  or die 'OpenApplication failed';

# Get references to some important Crystal Space objects we will be using
my $gfx3d = CS_QUERY_REGISTRY($objreg, 'iGraphics3D');
die 'No iGraphics3D!' unless defined $gfx3d;
my $engine = CS_QUERY_REGISTRY($objreg, 'iEngine');
die 'No iEngine!' unless defined $engine;
my $vclock = CS_QUERY_REGISTRY($objreg, 'iVirtualClock');
die 'No iVirtualClock!' unless defined $vclock;
my $kbd = CS_QUERY_REGISTRY($objreg, 'iKeyboardDriver');
die 'No iKeyboardDriver!' unless defined $kbd;
my $loader = CS_QUERY_REGISTRY($objreg, 'iLoader');
die 'No iLoader!' unless defined $loader;

# Setup the world
$engine->SetLightingCacheMode(0);
my $sector = CreateRoom();

# Setup the camera
my $rotX = 0; my $rotY = 0;
my $view = new csView ($engine, $gfx3d);
my $gfx2d = $gfx3d->GetDriver2D;
$view->SetRectangle(0, 0, $gfx2d->GetWidth, $gfx2d->GetHeight);
$view->GetCamera->SetSector($sector);
$view->GetCamera->GetTransform->SetOrigin(new cspace::csVector3 (0, 5, -3));

print "Crystal Space is go with Perl5...\n";

my $status = cspace::csDefaultRunLoop($objreg);
# Here the application runs until csInitializer::CloseApplication is
# called. Then the runloop will return control to this script.
warn 'RunLoop exited with error status.' unless $status;

# Release references to Crystal Space objects (except $objreg)
undef $_ for ($gfx3d, $gfx2d, $engine, $vclock, $kbd, $loader, $view, $sector);

# After releasing our object references we are ready to clean up the
# Crystal Space environment prior to exit.
print "Crystal Space is closing down.\n";
csInitializer::DestroyApplication($objreg);

exit 0;

sub HandleEvent
{
  my $event = shift;

  if ($event->GetName == csevKeyboardDown($objreg))
  {
    my $key = cspace::csKeyEventHelper::GetCookedCode($event);

    if ($key == cspace::CSKEY_ESC())
    {
      csInitializer::CloseApplication($objreg);
    }
  }
  elsif ($event->GetName == csevFrame($objreg))
  {
    my $ticks = $vclock->GetElapsedTicks;
    my $speed = ($ticks / 1000) * 0.03 * 20;

    my $cam = $view->GetCamera;

    if ($kbd->GetKeyState(cspace::CSKEY_SHIFT()))
    {
      if ($kbd->GetKeyState(cspace::CSKEY_RIGHT()))
        { $cam->Move(cspace::CS_VEC_RIGHT() * 4 * $speed) }
      if ($kbd->GetKeyState(cspace::CSKEY_LEFT()))
        { $cam->Move(cspace::CS_VEC_LEFT() * 4 * $speed) }
      if ($kbd->GetKeyState(cspace::CSKEY_UP()))
        { $cam->Move(cspace::CS_VEC_UP() * 4 * $speed) }
      if ($kbd->GetKeyState(cspace::CSKEY_DOWN()))
        { $cam->Move(cspace::CS_VEC_DOWN() * 4 * $speed) }
    }
    else
    {
      if ($kbd->GetKeyState(cspace::CSKEY_RIGHT())) { $rotY += $speed }
      if ($kbd->GetKeyState(cspace::CSKEY_LEFT()))  { $rotY -= $speed }
      if ($kbd->GetKeyState(cspace::CSKEY_PGUP()))  { $rotX += $speed }
      if ($kbd->GetKeyState(cspace::CSKEY_PGDN()))  { $rotX -= $speed }
      if ($kbd->GetKeyState(cspace::CSKEY_UP()))
        { $cam->Move(cspace::CS_VEC_FORWARD() * 4 * $speed) }
      if ($kbd->GetKeyState(cspace::CSKEY_DOWN()))
        { $cam->Move(cspace::CS_VEC_BACKWARD() * 4 * $speed) }
    }

    my $rot = new csXRotMatrix3 ($rotX)
	    * new csYRotMatrix3 ($rotY);
    my $tf = new csOrthoTransform ($rot, $cam->GetTransform->GetOrigin);
    $cam->SetTransform($tf);

    $gfx3d->BeginDraw($engine->GetBeginDrawFlags | CSDRAW_3DGRAPHICS());
    $view->Draw;
    $gfx3d->FinishDraw;
    $gfx3d->Print(undef);
  }
  else { return 0 }

  return 1;
}

sub CreateRoom
{
  $loader->LoadTexture('stone', '/lib/std/stone4.gif')
    or warn 'Failed to load texture';

  my $material = $engine->GetMaterialList->FindByName('stone');

  my $sector = $engine->CreateSector('room');
  my $walls = $engine->CreateSectorWallsMesh($sector, 'walls');
  my $walls_obj = $walls->GetMeshObject;
  my $walls_fact = $walls_obj->GetFactory;
  my $walls_state = SCF_QUERY_INTERFACE
			($walls_fact, 'iThingFactoryState');
  $walls_state->AddInsideBox(new csVector3 (-5, 0, -5),
			     new csVector3 (5, 20, 5));
  $walls_state->SetPolygonMaterial(CS_POLYRANGE_LAST(), $material);
  $walls_state->SetPolygonTextureMapping(CS_POLYRANGE_LAST(), 3);

  my @lights = ($engine->CreateLight(undef, new csVector3 (-3, 5, 0),
					10, new csColor (1, 0, 0)),
		$engine->CreateLight(undef, new csVector3 (3, 5, 0),
					10, new csColor (0, 0, 1)),
		$engine->CreateLight(undef, new csVector3 (0, 5, -3),
					10, new csColor (0, 1, 0)));
  $sector->GetLights->Add($_) foreach @lights;

  $engine->Prepare;

  return $sector;
}
