Changes to Crystal Space to support collision detection

Changed files in last patch:
src/colldet/collider.h
src/colldet/build.cpp
READMECD.txt

General Notes and TODO List

Eventually the following will need to happen:
All sectors must processed even those which are not visible.
Implementation of Line intersection with Colliders.
Implementation of Sphere (blast radious) intersection with colliders.

History --------------------------------------------------------

Feb 24
- Added extra checking to avoid rounding problems in DJGPP where
  dividing by a small number causes a divide by zero exception.
  Now when a value that is about to be used for a divide is smaller than
  SMALL_EPSILON, SMALL_EPSILON is used instead.  This value may
  have to be adjusts for DJGPP.  Note other implementations now
  suffer from a small precision loss.  Perhaps in the future if I
  determine that this actually fixes the DJGPP case, I may make it
  a fix specific for DJGPP.

Feb 17
- Added capability for collision detection to collect all
  neighbouring sectors and uses them for collision testing
  this works better than only testing the sector where the
  eye is (in fact this is correct way).
  
Jan 13
- Separated moving objects like the player into a seperate subclass
  from collider.  This will make it easier for users to understand
  how to add their own objects.  They should only need to look at
  the Being::collision_detect() code.

- Collider is a generic class.  All objects which participate in
  collisions are Colliders (walls, stairs, bullets, monsters etc.)
  but moving objects need to keep some state information.  To
  serve this purpose there is a Being class (maybe a better name can
  be thought of).  Beings are the ones which should be tested in a
  special loop (see next paragraph).  Beings move around and can
  hit things.  They maybe able to climb a stair case or climb a wall.
  They fall under the effect or gravity and may slip or bounce.
  Under certain conditions they may be able to fly or move through
  a wall etc.  The logic for this should be handled by the function
  Being::collision_detect().  The Bot for instance should subclass
  from Being, or it maybe able to use Being as is.

  Currently the main.cpp or keys.cpp just updates the camera (that
  represents the player) and moves it according to the key press.
  Then in PrepareFrame, collision detection tries to see if the current
  camera position hits something.  If it does, it tries to revert the
  camera into the position it was in the last time collision was called.
  This is basically wrong.  The camera should follow the player and not
  the other way around, this way different camera view would also be
  easier to do (eg 3rd person view) switch a camera onto a Bot or Missile.
  The camera should have an object to be attached to (with its coord sys
  relative to the object it is following).

  We should have some sort of event loop.
  Key presses should affect some object (not the camera), but not 
  directly, they should generate an event for an object (call it a
  move event), this event goes into the queue.  The queue should store
  all events ordered by their time stamp.
  The main loop could look something like this:
     wait for server to respond with system time.
     while (game not over)
	 collect keyboard events create move requests for the appropriete object
          queue these requests stamped with system time.
	 queue incoming network request with their time stamps.
       queue timed trigger events
	 queue sounds based on system time.
       enqueue frame refresh event with time according to desired frame rate.
       while (events in the queue whose time <= system time)
		{
		e = get.event()
		switch(e)
		case move:
			collide(e.o);
		case sound:
			play(e.s);
		case refresh:
			renderframe in current state.
		}

	now collide will decide whether the object 'o' can complete the move or not.
	there may be different collide methods for different types of objects.  These
	objects could be derived from the Being class.

- The 'help I've fallen and I can't get up issue'.
  The current collision function in being is too simplistic, it assumes the player
  is wholly within the same sector as the camera.  This may not be true, as is the
  case with falling in the 'street' of the large level.  At any moment, the eye (camera)
  is in only one sector, but the rest of the body could in any number of other sectors
  depending on how large the body and how small the sectors are.  And it would need
  to test for collisions in all these sectors to ensure correct CD.  So we need a good
  (ie fast) method for finding all sectors withing a bounding box, if we have this we
  are ok.  (PS the Collider object is fine CD works as long as all the relevant sectors
  are checked).
  I am open to suggestions to find this set of sectors which lie within the objects
  bbox.  Once we have this set the Collider can perform the fine grained CD.

Dec 6:
- Finished conversion to native classes.
- Eliminated all original files.

Dec 3:
- Implemented small performance enhancement in Vector3::operator/
- Implemented recursive split in native matrix/vector functionality
  and tested it.

Nov 22:
- Implemented Eigen vector calculation function in core CS
  matrix functionality.
- Implemented the Accum and Moment classes to use native CS
  matrix/vector functionality.

Oct 22:
- Renamed directory to colldet.
- Removed VTYPE define from makefiles, will define in header
  file instead.

Oct 18:
- Implemented link to collision system via ColliderP class.
  Collision detection system is now almost completely independent
  from rest of CS classes.
- Rewritten version of RAPID now compiles and links, it has
  all matrix and vector logic replaced with native CS code.
- Still needs to be completely debugged.

Oct 17:
- Collision detection is now on by default if it is compiled into
  the system.  Use -nocolldet to turn it off.  To temporarily
  disable collision detection while the program runs you can still
  use '*'.
- Added CD initialization for MultiSectorObject.
- Added CD testing for MSOs.  Sydney now get some respect and
  you cannot walk through her any more.

Oct 12:
- Converted RAPID from double to float.  RAPID sample client file
  is about 65% faster and should use about 30% less memory:
  double: 2.290u 0.070s = 2.99
  float:  1.770u 0.020s = 1.79
- created collision subdirectory, moved RAPID code here, uses
  standard makefiles to compile now.
- Removed uneeded files from collision dir.
- Renamed files to CS file conventions.
- Created Collider class, moved collision detect info from PolygonSet
  to Collider class.  PS and 3DSprite now have a member Collider.
- Remove CD info from Camera.
- Remove CD info from World.
- Remove CD info from Sector.
- Added support for csSprite3D.
- Implement collision reaction in walktest.
  - Camera can not walk through walls.
  - Will fall if not on floor.
  - Will climb inclines.
  - You can climb the stairs in large.zip if you look up a bit.
  - There are situations where you can get stuck, or fall out of the level.
- Performance doesn't show any degradation if collision detection is on
  or off.
- Enhanced console display.  This is currently what is shown:
  CD nnn cam pos xxx yyy zzz F C [names][names]
  nnn           - the number of collisions counted.
  xxx yyy zzz   - camera position
  F             - F if falling, blank otherwise.
  C             - C if climbing blank otherwise.

Oct 11:

- Reimplementing RAPID.
- Uses native Vector3 and Matrix3 classes and native calls
  to manipulate these.
- Eliminating unneeded functionality from RAPID.
- Eliminating copy of triangles in RAPID.
- Adding support for Sphere, Segment, Axis Aligned and 
  coarser Oriented Bounding Box collision testing.
- Integrated as a subdirectory of src called collide.
- Uses standard CS makefiles to compile itself.
- NOTE these changes will be released in a later patch.
  There is a lot of testing left to do here.

Oct 4
- Collision detection works with VCollide.  However VCollide assumes
  all objects can collide and anything that cannot collide has to be explicitly
  turned off.  This is too much hassle.
- cs.mak Removed VCollide and only use on RAPID.

Sept 26

- Modified vcollide/src/Makefile to define NULL=0 cause compile error otherwise.
- cs.mak enhanced to build vcollide library with 'make cd'.
  This is needed if you want to compile CS with collision detection.
- Removed CD code from general/csview.cpp and moved it to
  general/walktest/main.cpp and general/walktest/keys.cpp.
  Collision detection is now called at the start of PrepareFrame().
  Initialization is moved from the world constructor
  to main as well.
- CD can now be turned on at runtime.
- Added COLLDET keyword to cryst.cfg
- Added the command line -colldet/nocolldet.
- Added key '*' to toggle collision detection.  Note that for
  collision detection to work it must be turned on at startup
  to allow the OBBs to be precomputed.
- Added debugging info on console.
- Movement of camera is now updated in CD.
- Camera dimension is 1.2x0.5x0.3.
- Updated documentation.


Sept 17
- cs.mak Added definitions DO_CD, CD_PATH, CD_INC_PATH, CD_LIB_PATH
  which should point to the vcollide directory.
  assumes that vcollide is in /src/../vcollide
- src/engine/basic/polyset.{h,cpp}
  Added API to interface with VCollide routines.
  Added cd_id attribute.
  Performs double/float conversions because VCollide deals with floats.
- src/engine/objects/thing.{h,cpp}
  Added initialization.
  Add update to PolygonSet::transform base
  on obj ReversibleTransform (geometry/math3d.h)
- src/engine/sector.{h,cpp}
- Added initialization for sector and its things.

- src/engine/world.{h,cpp}
  Added control that activates the sector and all the things in it
  for collision detection.

- general/csview.cpp
  Added collide calculation and result testing for each frame.
  WRONG PLACE FOR THIS!

