/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_ENGSEQ_H__
#define __CS_IVARIA_ENGSEQ_H__

#include "csutil/scf.h"

/**\file
 */


struct iCamera;
struct iLight;
struct iMeshWrapper;
struct iObject;
struct iSector;
struct iSharedVariable;

class csBox3;
class csColor;
class csSphere;
class csVector3;

struct iSequence;
struct iSequenceManager;
struct iSequenceTrigger;
struct iSequenceWrapper;

//@{
/**
 * Operations for AddConditionLightChange().
 */
enum
{
  CS_SEQUENCE_LIGHTCHANGE_NONE = 0,
  CS_SEQUENCE_LIGHTCHANGE_LESS = 1,
  CS_SEQUENCE_LIGHTCHANGE_GREATER = 2
};
//@}

SCF_VERSION (iParameterESM, 0, 0, 1);

/**
 * This interface is a parameter resolver. The operations
 * in the engine sequence manager use instances of this class
 * to get the required object (mesh, light, material, ...).
 * The engine sequence manager itself currently provides two ready-made
 * implementations of this resolver:
 * <ul>
 * <li>iEngineSequenceParameters->CreateParameterESM() which will create
 *     a resolver that gets the requested parameter from the given
 *     iEngineSequenceParameters instance. This is useful for sequences
 *     where you don't know in advance on what objects you will use it.
 * <li>iEngineSequenceManager->CreateParameterESM() which will give
 *     a resolver that returns a constant value. This is useful for
 *     operations where the object to operate on is known in advance.
 * </ul>
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Own implementions
 *   <li>iEngineSequenceParameters::CreateParameterESM()
 *   <li>iEngineSequenceManager::CreateParameterESM()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iSequenceWrapper
 *   </ul>
 */
struct iParameterESM : public iBase
{
  /**
   * Get the value based on userdata which is given to the
   * operations. If IsConstant() returns true then the params
   * parameter will not be used!
   * \param params Is given to the operations as userdata.
   */
  virtual iBase* GetValue (iBase* params = 0) const = 0;

  /**
   * Returns true if the value is constant and immediatelly available
   * upon request. In that case operations can store that value so
   * then don't have to ask for it every time. If this function returns
   * false then the operation MUST call GetValue() every time it
   * wants to do something with the object.
   */
  virtual bool IsConstant () const = 0;
};

SCF_VERSION (iEngineSequenceParameters, 0, 0, 2);

/**
 * An interface for passing on parameters to the engine sequence
 * manager. You can create a ready-made instance of this interface
 * by calling iSequenceWrapper->CreateBaseParameterBlock(). This will
 * create an empty parameter block that specifies the supported parameters
 * (and optional default values) that are relevant for that sequence.
 * When running a sequence later you can call
 * iSequenceWrapper->CreateParameterBlock() to make a clone of the
 * base parameter block and then fill in the values.<br>
 * To use a value from this parameter block you can call CreateParameterESM()
 * which will return a parameter that you can give to an operation.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iSequenceWrapper::CreateBaseParameterBlock()
 *   <li>iSequenceWrapper::CreateParameterBlock ()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iSequenceWrapper::GetBaseParameterBlock ()
 *   <li>iSequenceTrigger::GetParameters ()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iSequenceWrapper
 *   </ul>
 */
struct iEngineSequenceParameters : public iBase
{
  /**
   * Get the number of parameters supported.
   */
  virtual size_t GetParameterCount () const = 0;

  /**
   * Get a parameter.
   */
  virtual iBase* GetParameter (size_t idx) const = 0;

  /**
   * Get a parameter by name.
   */
  virtual iBase* GetParameter (const char* name) const = 0;

  /**
   * Get a parameter index by name.
   */
  virtual size_t GetParameterIdx (const char* name) const = 0;

  /**
   * Get parameter name.
   */
  virtual const char* GetParameterName (size_t idx) const = 0;

  /**
   * Add a parameter. Warning! ONLY call this for setting up the
   * base parameter block. Don't use this to set the values for
   * parameters later on blocks created with CreateParameterBlock()!
   */
  virtual void AddParameter (const char* name, iBase* def_value = 0) = 0;

  /**
   * Set a parameter by index.
   */
  virtual void SetParameter (size_t idx, iBase* value) = 0;

  /**
   * Set a parameter by name.
   */
  virtual void SetParameter (const char* name, iBase* value) = 0;

  /**
   * Create a parameter ESM which keeps a reference to this parameter
   * block and knows how to resolve the specified parameter. Returns
   * 0 if the parameter 'name' is not known in this block. You can use
   * the return of this function to give as an argument for operations.
   */
  virtual csPtr<iParameterESM> CreateParameterESM (const char* name) = 0;
};

SCF_VERSION (iSequenceWrapper, 0, 3, 0);

/**
 * A sequence wrapper. This objects holds the reference
 * to the original sequence and also implements iObject.
 * Basically a sequence corresponds to a series of operations
 * that are time based and can be scheduled on the sequence manager.
 * This class enhances iSequence with support for custom operations
 * and parameter blocks.
 * <p>
 * Note that many parameters given to the AddOperation functions
 * are of type iParameterESM.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngineSequenceManager::CreateSequence ()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngineSequenceManager::GetSequence()
 *   <li>iEngineSequenceManager::FindSequenceByName()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngineSequenceManager
 *   </ul>
 */
struct iSequenceWrapper : public iBase
{
  /**
   * Query iObject that is implemented by the sequence manager.
   */
  virtual iObject* QueryObject () = 0;

  /**
   * Get the sequence that this wrapper maintains. It is allowed to use
   * the underlying sequence for general sequence operations like adding
   * conditions, operations, and general sequence management. The
   * AddOperationBla() functions provided in this wrapper do nothing
   * more than add custom operations through the regular
   * iSequence->AddOperation().
   */
  virtual iSequence* GetSequence () = 0;

  /**
   * Create a parameter block for this sequence wrapper. After
   * creating this you can initialized the parameters (with names
   * and optional default values). Later on you can use
   * GetSequenceParameters() to get a copy of a parameter block to
   * use for running a sequence.
   */
  virtual iEngineSequenceParameters* CreateBaseParameterBlock () = 0;

  /**
   * Get the pointer to the base parameter block (or 0 if there is
   * no such block).
   */
  virtual iEngineSequenceParameters* GetBaseParameterBlock () = 0;

  /**
   * Create a parameter block which you can then fill in and then
   * give as a parameter running this sequence. This essentially
   * creates a copy of the base parameter block created with
   * CreateBaseParameterBlock(). Modifications on the parameter block
   * returned by this function have no effect on the parameter block
   * which is kept internally. You should only set the values
   * of the given parameter block and not create/remove variables.
   * This function returns 0 if there is no parameter block
   * for this sequence.
   */
  virtual csPtr<iEngineSequenceParameters> CreateParameterBlock () = 0; 

  /**
   * Operation: set a variable to a floating point value.
   * If 'dvalue' is not 0 then that will be used instead of the absolute
   * value. In that case 'dvalue' is added.
   * \param time is the relative time at which this operation will fire.
   * \param var is the variable that will be set by this operation. The
   * value that is set is the 'value' parameter.
   * \param value is the new value for 'var'.
   * \param dvalue is a difference that is added. Only used if it is
   * not 0.
   */
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, float value, float dvalue = 0) = 0;

  /**
   * Operation: set a variable to the contents of another variable.
   * If 'dvalue' is not 0 then that will be used instead of the absolute
   * value. In that case 'dvalue' is added. 'dvalue' has to be a floating
   * point variable for that to work. 'value' can be any type. The type
   * of 'var' will be set to the type of 'value' in that case.
   * \param time is the relative time at which this operation will fire.
   * \param var is the variable that will be set by this operation. The
   * value that is set is the 'value' parameter.
   * \param value is the new value for 'var'.
   * \param dvalue is a difference that is added. Only used if it is not 0.
   */
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, iSharedVariable* value,
		iSharedVariable* dvalue = 0) = 0;

  /**
   * Operation: set a variable to a vector.
   * \param time is the relative time at which this operation will fire.
   * \param var is the variable that will be set by this operation.
   * \param v is the new vector value.
   */
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csVector3& v) = 0;

  /**
   * Operation: set a variable to a color.
   * \param time is the relative time at which this operation will fire.
   * \param var is the variable that will be set by this operation.
   * \param c is the new color value.
   */
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csColor& c) = 0;

  /**
   * Operation: set a material on a mesh.
   * \param time is the relative time at which this operation will fire.
   * \param mesh is a parameter that represents a mesh on which the material
   * should be set. The mesh should support SetMaterialWrapper().
   * \param mat is a parameter that represents the material to set.
   */
  virtual void AddOperationSetMaterial (csTicks time, iParameterESM* mesh,
		  iParameterESM* mat) = 0;

  /**
   * Operation: set a material on a polygon.
   * \param time is the relative time at which this operation will fire.
   * \param polygon is a parameter that represents a polygon on which the
   * material should be set.
   * \param mat is a parameter that represents the material to set.
   */
  virtual void AddOperationSetPolygonMaterial (csTicks time,
  		  iParameterESM* polygon, iParameterESM* mat) = 0;

  /**
   * Operation: set a light color.
   * \param time is the relative time at which this operation will fire.
   * \param light is a parameter representing the light which will
   * be set to the given color.
   * \param color is the new color value.
   */
  virtual void AddOperationSetLight (csTicks time, iParameterESM* light,
		  const csColor& color) = 0;

  /**
   * Operation: fade a light to some color during some time.
   * \param time is the relative time at which this operation will fire.
   * \param light is a parameter representing the light which will
   * be fade to the given color.
   * \param color is the final color value.
   * \param duration is the duration time of the fade. The fade will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current color to destination color.
   */
  virtual void AddOperationFadeLight (csTicks time, iParameterESM* light,
		  const csColor& color, csTicks duration) = 0;

  /**
   * Operation: set dynamic ambient light color.
   * \param time is the relative time at which this operation will fire.
   * \param sector is a parameter representing the sector which will
   * have its ambient set.
   * \param color is the ambient color unless 'colorvar' is not 0.
   * \param colorvar is the variable containing the desired color. This
   * will be used instead of 'color' is not 0.
   */
  virtual void AddOperationSetAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, iSharedVariable *colorvar) = 0;

  /**
   * Operation: fade dynamic ambient light to some color during some time.
   * \param time is the relative time at which this operation will fire.
   * \param sector is a parameter representing the sector which will
   * have its ambient set.
   * \param color is the final ambient value.
   * \param duration is the duration time of the fade. The fade will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current ambient to destination ambient.
   */
  virtual void AddOperationFadeAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, csTicks duration) = 0;

  /**
   * Operation: Delay executation of the rest of the script by a random
   * time between min and max msec.
   * \param time is the relative time at which this operation will fire.
   * \param min is the minimum time to wait starting with 'time'.
   * \param max is the maximum time to wait starting with 'time'.
   */
  virtual void AddOperationRandomDelay (csTicks time, int min, int max) = 0;

  /**
   * Operation: set a mesh color.
   * \param time is the relative time at which this operation will fire.
   * \param mesh is a parameter representing a mesh.
   * \param color is the color to set this mesh too. Not all meshes
   * support this.
   */
  virtual void AddOperationSetMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color) = 0;

  /**
   * Operation: fade a mesh to some color during some time.
   * \param time is the relative time at which this operation will fire.
   * \param mesh is a parameter representing a mesh.
   * \param color is the destination color to fase this mesh too.
   * Not all meshes support this.
   * \param duration is the duration time of the fade. The fade will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current color to destination color.
   */
  virtual void AddOperationFadeMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color, csTicks duration) = 0;

  /**
   * Operation: set a fog color and density.
   * \param time is the relative time at which this operation will fire.
   * \param sector is a parameter representing the sector which will
   * have its ambient set.
   * \param color is the required color value.
   * \param density is the required density.
   */
  virtual void AddOperationSetFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density) = 0;

  /**
   * Operation: fade fog to some color/density during some time.
   * \param time is the relative time at which this operation will fire.
   * \param sector is a parameter representing the sector which will
   * have its ambient set.
   * \param color is the final color value.
   * \param density is the final density.
   * \param duration is the duration time of the fade. The fade will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current fog settings to destination fog settings.
   */
  virtual void AddOperationFadeFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density, csTicks duration) = 0;

  /**
   * Operation: rotate object during some time. After the time has elapsed
   * the rotation will be equal to the given angle here.
   * Axis is 0, 1, or 2 for x, y, or z. If axis is -1 it is not used.
   * \param time is the relative time at which this operation will fire.
   * \param mesh is a parameter representing a mesh.
   * \param axis1 is the first rotation axis (-1, 0, 1, or 2).
   * \param tot_angle1 is the total angle to rotate around axis1.
   * \param axis2 is the second rotation axis (-1, 0, 1, or 2).
   * \param tot_angle2 is the total angle to rotate around axis2.
   * \param axis3 is the third rotation axis (-1, 0, 1, or 2).
   * \param tot_angle3 is the total angle to rotate around axis3.
   * \param offset is added to the rotation transformation so you can
   * rotate an object around a center different from 0,0,0.
   * \param duration is the duration time of the rotate. The rotate will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current orientation to destination orientation.
   */
  virtual void AddOperationRotateDuration (csTicks time, iParameterESM* mesh,
  		int axis1, float tot_angle1,
		int axis2, float tot_angle2,
		int axis3, float tot_angle3,
		const csVector3& offset,
		csTicks duration) = 0;

  /**
   * Operation: move object (mesh or light) during some time. After the time
   * has elapsed the total relative move will be equal to the 'offset'.
   * \param time is the relative time at which this operation will fire.
   * \param mesh is a parameter representing a mesh.
   * \param offset is the relative amount to move.
   * \param duration is the duration time of the move. The move will
   * start at relative time 'time' and will take 'duration' milliseconds
   * to go from current location to destination.
   */
  virtual void AddOperationMoveDuration (csTicks time, iParameterESM* mesh,
		const csVector3& offset, csTicks duration) = 0;

  /**
   * Operation: enable/disable a given trigger.
   * \param time is the relative time at which this operation will fire.
   * \param trigger is a parameter representing a trigger to enable or
   * disable.
   * \param en is true to enable or false to disable.
   */
  virtual void AddOperationTriggerState (csTicks time,
  		  iParameterESM* trigger, bool en) = 0;

  /**
   * Operation: enable checking of trigger state every 'delay'
   * milliseconds (or disable with delay == 0). Use this in
   * combination with AddOperationTestTrigger().
   * \param time is the relative time at which this operation will fire.
   * \param trigger is a parameter representing a trigger to enable or
   * disable.
   * \param delay represents the frequency of checking the trigger.
   */
  virtual void AddOperationCheckTrigger (csTicks time,
  		  iParameterESM* trigger, csTicks delay) = 0;

  /**
   * Operation: test trigger state and run a sequence if trigger
   * is still valid or another sequence if not (both sequences
   * can be 0 in which case nothing is run).
   * Use in combination with AddOperationCheckTrigger().
   * \param time is the relative time at which this operation will fire.
   * \param trigger is a parameter representing a trigger to enable or
   * disable.
   * \param trueSequence is the sequence that will be fired
   * when the trigger succeeds. Can be 0.
   * \param falseSequence is the sequence that will be fired
   * when the trigger does not succeed. Can be 0.
   */
  virtual void AddOperationTestTrigger (csTicks time,
  		  iParameterESM* trigger,
		  iSequence* trueSequence,
		  iSequence* falseSequence) = 0;
};

SCF_VERSION (iSequenceTrigger, 0, 0, 3);

/**
 * A sequence trigger. When all conditions in a trigger are
 * true it will run a sequence. Note that after the succesfull firing
 * of a trigger it will automatically be disabled.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngineSequenceManager::CreateTrigger ()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngineSequenceManager::GetTrigger()
 *   <li>iEngineSequenceManager::FindTriggerByName()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngineSequenceManager
 *   </ul>
 */
struct iSequenceTrigger : public iBase
{
  /**
   * Query object.
   */
  virtual iObject* QueryObject () = 0;

  /**
   * Condition: true if camera is in some sector.
   */
  virtual void AddConditionInSector (iSector* sector) = 0;

  /**
   * Condition: true if camera is in some sector and bbox.
   */
  virtual void AddConditionInSector (iSector* sector,
	const csBox3& box) = 0;

  /**
   * Condition: true if camera is in some sector and sphere.
   */
  virtual void AddConditionInSector (iSector* sector,
	const csSphere& sphere) = 0;

  /**
   * Condition: true if (part of) sector is visible.
   */
  virtual void AddConditionSectorVisible (iSector* sector) = 0;

  /**
   * Condition: true if clicked on a mesh.
   */
  virtual void AddConditionMeshClick (iMeshWrapper* mesh) = 0;

  /**
   * Condition: light change.  Call this to add a trigger
   * which fires a sequence when a light gets darker than 
   * a certain value or lighter than a certain value, or
   * whenever a light changes.
   * \param whichlight represents the light on which we will test.
   * \param oper is one of #CS_SEQUENCE_LIGHTCHANGE_NONE,
   * #CS_SEQUENCE_LIGHTCHANGE_LESS, or #CS_SEQUENCE_LIGHTCHANGE_GREATER
   * depending on the test you want to use.
   * \param color is the color to compare with.
   */
  virtual void AddConditionLightChange (iLight *whichlight, 
				        int oper, const csColor& color) = 0;

  /**
   * Condition: manual trigger. Call this to set add a trigger
   * that requires manual confirmation. The 'Trigger()' function
   * can then be used later to actually do the trigger.
   */
  virtual void AddConditionManual () = 0;

  /**
   * Enable/disable this trigger. Triggers start enabled by
   * default.
   */
  virtual void SetEnabled (bool en) = 0;

  /**
   * Get enabled/disabled state.
   */
  virtual bool IsEnabled () const = 0;

  /**
   * Clear all conditions.
   */
  virtual void ClearConditions () = 0;

  /**
   * Trigger the manual condition.
   */
  virtual void Trigger () = 0;

  /**
   * Set the parameter block to use for the sequence when it is fired.
   */
  virtual void SetParameters (iEngineSequenceParameters* params) = 0;

  /**
   * Get the parameter block.
   */
  virtual iEngineSequenceParameters* GetParameters () const = 0;

  /**
   * Attach the sequence that will be fired when all trigger
   * conditions are valid.
   * \remark \p seq will NOT be IncRef()ed - you'll have to ensure
   * it's not prematurely destructed.
   */
  virtual void FireSequence (csTicks delay, iSequenceWrapper* seq) = 0;

  /**
   * Get the attached sequence.
   */
  virtual iSequenceWrapper* GetFiredSequence () = 0;

  /**
   * Test the conditions of this trigger every 'delay' milliseconds.
   * Use this in combination with CheckState(). If 'delay' == 0
   * then this testing is disabled (default).
   */
  virtual void TestConditions (csTicks delay) = 0;

  /**
   * This function returns true if the trigger conditions are
   * valid. This only works if TestConditions() has been called
   * and it doesn't work immediatelly after TestConditions() because
   * TestConditions() needs to take some time before it actually
   * can retest the conditions.
   */
  virtual bool CheckState () = 0;

  /**
   * Force the sequence of this trigger to be fired right now.
   * Note that this will even fire if the trigger is disabled and
   * conditions are completely ignored.
   * <p>
   * Also calling ForceFire() will NOT cause the trigger to become disabled
   * (as opposed to when a trigger normally fires). So if you want to make
   * sure the trigger does not accidently fire again right after firing it
   * you should disable the trigger (and possibly let the sequence enable
   * it again).
   * <p>
   * Note that ForceFire() still respects the fire delay with which the
   * sequence was registered. If you use 'now' == true then this delay
   * will be ignored and the sequence will be started immediatelly.
   */
  virtual void ForceFire (bool now = false) = 0;
};

SCF_VERSION (iSequenceTimedOperation, 0, 0, 1);

/**
 * A timed operation for the engine sequence manager.
 * This is basically something that needs to run over some period
 * of time. The 'elapsed' value that needs to be implemented by
 * subclasses will go from 0 to 1. When the time expires (goes beyond
 * 1) then the operation will be deleted automatically.
 * Timed operations are usually fired from within a sequence operation
 * (iSequenceOperation).
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>Application specific. Instances of this are also created
 *       internally by the engine sequence manager plugin.
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngineSequenceManager::FireTimedOperation()
 *   </ul>
 */
struct iSequenceTimedOperation : public iBase
{
  /**
   * Do the operation. 'time' will be between 0 and 1.
   */
  virtual void Do (float time, iBase* params) = 0;
};

SCF_VERSION (iEngineSequenceManager, 0, 0, 3);

/**
 * Sequence manager specifically designed for working on
 * the engine.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Engine Sequence Manager plugin (crystalsapce.utilities.sequence.engine)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iEngineSequenceManager : public iBase
{
  /**
   * Get a pointer to the underlying sequence manager that
   * is being used.
   */
  virtual iSequenceManager* GetSequenceManager () = 0;

  /**
   * Set the camera to use for some of the features (like clicking
   * on mesh objects). If this is not set then those features will
   * not be available.
   */
  virtual void SetCamera (iCamera* camera) = 0;

  /**
   * Get the camera that is used for some features.
   */
  virtual iCamera* GetCamera () = 0;
  
  /**
   * Create a parameter ESM for a constant value.
   */
  virtual csPtr<iParameterESM> CreateParameterESM (iBase* value) = 0;

  //-----------------------------------------------------------------------

  /**
   * Create a new trigger with a given name.
   */
  virtual csPtr<iSequenceTrigger> CreateTrigger (const char* name) = 0;

  /**
   * Remove trigger from the manager.
   */
  virtual void RemoveTrigger (iSequenceTrigger* trigger) = 0;

  /**
   * Remove all triggers.
   */
  virtual void RemoveTriggers () = 0;

  /**
   * Get the number of triggers.
   */
  virtual size_t GetTriggerCount () const = 0;

  /**
   * Get a trigger.
   */
  virtual iSequenceTrigger* GetTrigger (size_t idx) const = 0;

  /**
   * Get a trigger by name.
   */
  virtual iSequenceTrigger* FindTriggerByName (const char* name) const = 0;

  /**
   * Fire a trigger manually, specifying the name.
   * This will call ForceFire() on the trigger (if one is found). If
   * now == false then the usual delay will be respected. Otherwise
   * the sequence will be run immediatelly without the default delay.
   */
  virtual bool FireTriggerByName (const char *name, bool now = false) const = 0;

  //-----------------------------------------------------------------------

  /**
   * Create a new sequence with a given name.
   */
  virtual csPtr<iSequenceWrapper> CreateSequence (const char* name) = 0;

  /**
   * Remove sequence from the manager.
   */
  virtual void RemoveSequence (iSequenceWrapper* seq) = 0;

  /**
   * Remove all sequences.
   */
  virtual void RemoveSequences () = 0;

  /**
   * Get the number of sequences.
   */
  virtual size_t GetSequenceCount () const = 0;

  /**
   * Get a sequence.
   */
  virtual iSequenceWrapper* GetSequence (size_t idx) const = 0;

  /**
   * Get a sequence by name.
   */
  virtual iSequenceWrapper* FindSequenceByName (const char* name) const = 0;

  /**
   * Run a sequence and don't mess around with triggers.
   */
  virtual bool RunSequenceByName (const char *name,int delay) const = 0;

  //-----------------------------------------------------------------------

  /**
   * Start a timed operation with a given delta (in ticks).
   * The delta has to be interpreted as the amount of time that has
   * already elapsed since the beginning of the timed operation.
   * The params block is increffed for as long as is needed so you
   * can release your reference.
   */
  virtual void FireTimedOperation (csTicks delta, csTicks duration,
  	iSequenceTimedOperation* op, iBase* params = 0) = 0;

  //-----------------------------------------------------------------------
};

#endif // __CS_IVARIA_ENGSEQ_H__

