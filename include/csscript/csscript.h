/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef csScript_H
#define csScript_H

#include "csgeom/math3d.h"
#include "csobject/csobject.h"

class csScript;
class csRunScript;
class csLight;
class csWorld;
class csCamera;

/**
 * Possible event types for scripts.
 */
enum csScriptEvent
{
  csScriptEventNone = 0,
  csScriptEventActivate,
  csScriptEventCollision,
  csScriptEventEnter,
  csScriptEventDamaged,
  csScriptEventTimer
};

/**
 * The API for the language.
 * There is only one instance of this class.
 */
class LanguageLayer
{
private:
  ///
  csWorld* world;
  ///
  csCamera* camera;
  /// Linked list of all running scripts.
  csRunScript* first_run;

public:
  ///
  LanguageLayer (csWorld* world, csCamera* camera);
  ///
  ~LanguageLayer ();

  /// Link a new running script into the system.
  void link_run (csRunScript* r);

  /// Step all the running scripts.
  void step_run ();

  /// Print a message on screen.
  void message (char* msg);

  /// Find a named object given some parent object.
  csObject* find_object (csObject* parent, char* name);

  /// Start enumeration of all objects in some parent object.
  csObject* first_object (csObject* parent);
  /// Next enumeration of all objects in some parent object.
  csObject* next_object (csObject* parent, csObject* obj);

  /**
   * Get the type of an object (one of CS_STATLIGHT, CS_SECTOR,
   * CS_THING, CS_COLLECTION, CS_WORLD, CS_SCRIPTRUN, CS_SCRIPT,
   * ...).
   */
  const csIdType& object_type (csObject* obj);

  /// Get the name of an object.
  const char* object_name (csObject* obj);

  /// Get the world object.
  csObject* get_world ();

  /// Get information about the camera.
  csObject* camera_sector ();
  /// Get information about the camera.
  csMatrix3 camera_matrix ();
  /// Get information about the camera.
  csVector3 camera_vector ();

  /// Transform an object given some matrix and/or vector.
  void transform (csObject* obj, csMatrix3& m, csVector3& v);
  /// Transform an object given some matrix and/or vector.
  void transform (csObject* obj, csMatrix3& m);
  /// Transform an object given some matrix and/or vector.
  void transform (csObject* obj, csVector3& v);
  /// Transform an object given some matrix and/or vector.
  void transform (csObject* obj, float dx, float dy, float dz);
  /// Transform an object given some matrix and/or vector.
  void transform_rot_x (csObject* obj, float angle);
  /// Transform an object given some matrix and/or vector.
  void transform_rot_y (csObject* obj, float angle);
  /// Transform an object given some matrix and/or vector.
  void transform_rot_z (csObject* obj, float angle);

  /// Fill a transformation matrix to do a rotation around some angle.
  void matrix_rot_x (csMatrix3& m, float angle);
  /// Fill a transformation matrix to do a rotation around some angle.
  void matrix_rot_y (csMatrix3& m, float angle);
  /// Fill a transformation matrix to do a rotation around some angle.
  void matrix_rot_z (csMatrix3& m, float angle);

  /**
   * Set the intensities/color of a light object
   * (Note that class csLight is also a csObject).
   */
  void set_light_intensity (csLight* light, float l1, float l2, float l3);
  /// Get the intensities/color of a light object.
  float get_lightmap1_intensity (csLight* light);
  /// Get the intensities/color of a light object.
  float get_lightmap2_intensity (csLight* light);
  /// Get the intensities/color of a light object.
  float get_lightmap3_intensity (csLight* light);

  /// Show some object.
  void show (csObject* obj);
  /// Hide some object.
  void hide (csObject* obj);

  /**
   * Send an event to another object (sending an event to an
   * object will cause the event to be sent to all running
   * scripts on the object, sending an event to a running
   * script will only send the event to that script instance).
   */
  void send_event (csObject* object, csScriptEvent event);

  /// Find a script (note that class Script is also a csObject).
  csScript* find_script (char* name);

  /// Run the script (create a new csRunScript) on some object.
  csRunScript* run_script (csScript* script, csObject* attached);

  /// Suspend a script for a specific time (in milliseconds).
  void suspend (csRunScript* run, long milli);

  // ... and so on ...
};

/**
 * Every script gets an instance of this class. It is responsible
 * for loading the script and possibly compiling it into some
 * internal form.
 */
class csScript : public csObject
{
protected:
  ///
  LanguageLayer* layer;

public:
  /**
   * Create the script with the given name. Where and how the script is
   * loaded depends on the language used for the script.
   */
  csScript (LanguageLayer* layer);
  ///
  virtual ~csScript ();

  ///
  LanguageLayer* get_layer () { return layer; }

  /**
   * Prepare the script for usage (compilation to byte code for
   * example).
   */
  virtual void prepare () = 0;

  /**
   * Create an instance of a running script.
   */
  virtual csRunScript* run_script (csObject* attached) = 0;

  // Create an instance of a static script info object.
  //virtual StaticScriptInfo* create_static_info () = 0;

  CSOBJTYPE;
};

/**
 * For every running script there is an instance of this class.
 */
class csRunScript : public csObject
{
protected:
  ///
  LanguageLayer* layer;
  ///
  csRunScript* nextR;
  ///
  csRunScript* prevR;
  ///
  csScript* script;
  /// The object for which this script is run.
  csObject* attached;

public:
  ///
  csRunScript (csScript* script, csObject* attached);
  ///
  virtual ~csRunScript ();

  ///
  void set_next (csRunScript* n) { nextR = n; }
  ///
  void set_prev (csRunScript* p) { prevR = p; }
  ///
  csRunScript* get_next () { return nextR; }
  ///
  csRunScript* get_prev () { return prevR; }

  ///
  csScript* get_script () { return script; }
  ///
  csObject* get_attached () { return attached; }
  ///
  LanguageLayer* get_layer () { return layer; }

  /// Initialize the running script.
  virtual void init () = 0;
  /// Perform one step of the script.
  virtual bool step () = 0;
  /// Deliver an event to the script.
  virtual void deliver_event (csScriptEvent event) = 0;

  CSOBJTYPE;
};

#endif

