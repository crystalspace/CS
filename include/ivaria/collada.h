/*
    Copyright (C) 2007 by Scott Johnson

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this application; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _CS_IVARIA_COLLADA_H
#define _CS_IVARIA_COLLADA_H

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

// Forward Declarations
struct iString;
struct iFile;
struct iDataBuffer;
struct iDocumentNode;

/**
 * Possible file types for the COLLADA plugin to accept.
 */

enum csColladaFileType {

  /// A Crystal Space library file
  CS_LIBRARY_FILE = 1,

  /// A Crystal Space map file
  CS_MAP_FILE,

  /// No file type.  This is for an initialization value.  Do not use.
  CS_NO_FILE
};

enum csColladaNumericType {

  /// A type representing the COLLADA integer format
  CS_COLLADA_INTEGER = 1,

  /// A Type representing the COLLADA floating point format
  CS_COLLADA_FLOAT
};

enum csColladaEffectProfileType {
  /// The common profile
  CS_COLLADA_PROFILE_COMMON = 1,

  /// nVidia's cg shading language
  CS_COLLADA_PROFILE_CG,

  /// OpenGL's GL Shading Language (GLSL)
  CS_COLLADA_PROFILE_GLSL

};

/**
 * Representation of a convertor from COLLADA files to Crystal Space files.
 *
 * Main creators of instances implementing this interface:
 * - Crystal Space COLLADA Conversion System (
 *   crystalspace.utilities.colladaconvertor)
 *
 * Main ways to get pointers to this interface:
 * - CS_QUERY_REGISTRY()
 */

struct iColladaConvertor : public virtual iBase
{
  SCF_INTERFACE(iColladaConvertor, 1, 0, 0);
    
  /** 
   * Load a COLLADA file from a null-terminated C-string into the COLLADA
   * Conversion System
   *
   * \param str A string containing the location of the file to be loaded in VFS
   *
   * \return 0 if everything is ok; otherwise an error message
   * \remarks This will replace the current file being used to read data
   *          from.
   */
  virtual const char* Load(const char *str) = 0;
   
   /** 
   * Load a COLLADA file from an iString object into the COLLADA Conversion
   * System
   *
   * \param str An iString containing the location of the file to be loaded in
   * VFS
   * \return 0 if everything is ok; otherwise an error message
   * \remarks This will replace the current file being used to read data
   *          from.
   */
  virtual const char* Load(iString *str) = 0;
   
   /** 
   * Load a COLLADA file from an iFile object into the COLLADA Conversion System
   *
   * \param file An iFile object which points to the document to be loaded
   * \return 0 if everything is ok; otherwise an error message
   * \remarks This will replace the current file being used to read data
   *          from.
   * \warning This version of the Load function assumes that the caller created
   *          the iFile object, and thus it is the caller's responsibility to
   *          close/destroy the object.
   */
  virtual const char* Load(iFile *file) = 0;

   /** 
   * Load a COLLADA file from an iDataBuffer object into the COLLADA Conversion System
   *
   * \param db An iDataBuffer object which contains a document to be loaded
   * \return 0 if everything is ok; otherwise an error message
   * \remarks This will replace the current file being used to read data
   *          from.
   */
  virtual const char* Load(iDataBuffer *db) = 0;

  /** \brief Sets the Crystal Space output file type.
   *
   * This function is designed to tell the COLLADA Conversion System what type
   * of file will be written to.  
   * \param filetype The type of file to be written out.  Will be one of:
   *  - CS_LIBRARY_FILE, a Crystal Space library file
   *  - CS_MAP_FILE, a Crystal Space world file
   * \return 0 if everything is ok; otherwise an error message
   * \remarks This function should be called before beginning the conversion
   * process. If it has not been called, the conversion functions will return
   * an error message.
   */
  virtual const char* SetOutputFiletype(csColladaFileType filetype) = 0;

  /**
   * \brief Writes the converted Crystal Space file out to disk
   *
   * This is used to write the Crystal Space file out to disk, once
   * a conversion process has been completed.
   * 
   * \param filepath The path in VFS where the file should be written to
   *
   * \return 0 if operation completed successfully; otherwise an error message
   *
   * \remarks This operation does not check to determine if the COLLADA file
   * has been converted, or if the Crystal Space document holds anything of
   * value. It Merely writes the document out to disk.
   */
  virtual const char* Write(const char* filepath) = 0;

  /**
   * Returns the Crystal Space Document 
   */
  virtual csRef<iDocument> GetCrystalDocument() = 0;

  /**
   * Returns the Collada Document
   */
  virtual csRef<iDocument> GetColladaDocument() = 0;

 /**
  * \brief Converts the loaded COLLADA file into the loaded Crystal Space file.
  *
  * This function will completely convert a loaded COLLADA file into Crystal
  * Space format. It is required that both a COLLADA file must be loaded, and
  * that a Crystal Space document must be ready. It will convert to the
  * document type specified when the Crystal Space document was loaded.
  * 
  * This function essentially calls all of the other convert functions and
  * then finalizes the iDocument so it can be completely written out to a file.
  *
  * \return 0 is everything is ok; otherwise an error message
  *
  * \remarks If debugging warnings are enabled, this function will display
  *          error messages in the console window.  Otherwise, error messages
  *          will only be available through the return value.
  * \warning An error will result if SetOutputFileType() is not called prior to 
  *          this function.
  *
  * \sa ConvertGeometry(iDocumentNode *geometrySection)
  * \sa ConvertLighting(iDocumentNode *lightingSection)
  * \sa ConvertTextureShading(iDocumentNode *textureSection)
  * \sa ConvertRiggingAnimation(iDocumentNode *riggingSection)
  * \sa ConvertPhysics(iDocumentNode *physicsSection)
  * \sa Write(const char* path)
  * \sa Load(iFile *file, csColladaFileType typeEnum)
  */
  virtual const char* Convert() = 0;
  
  /**
   * \brief Converts the geometry section of the COLLADA file
   * 
   * This converts the <mesh> elements of a COLLADA file to Crystal Space
   * format.  The function will update the internal Crystal Space document,
   * which can then be used to write out to a file.
   *
   * @param geometrySection A pointer to the <library_geometries> element,
   *        in the COLLADA document.
   */
  virtual bool ConvertGeometry(iDocumentNode *geometrySection) = 0;
  
  /**
   * \brief Converts the effects (COLLADA FX) section of the COLLADA file
   * 
   * This converts the COLLADA FX portion of the COLLADA file to Crystal
   * Space format.  Specifically, it converts the <library_images> elements
   * to textures and all <effect> elements to Crystal Space materials and shaders.
   * Currently, only <profile_COMMON> is supported, but support for <profile_GLSL>
   * and <profile_CG> are coming.
   *
   * @returns true on success, false otherwise
   */
  virtual bool ConvertEffects() = 0;

  /**
   * Converts the scene section of the COLLADA file.
   * 
   * This function converts the <library_cameras>, <library_lights>, <library_nodes>,
   * and <library_visual_scenes> sections of the COLLADA document.  Note that
   * unlike the other conversion functions, this function requires multiple 
   * arguments to be passed for conversion - one for each of these library elements.
   *
   * @param camerasSection A pointer to the iDocumentNode representing the 
   *        <library_cameras> element of the COLLADA document.
   *
   * @param lightsSection A pointer to the iDocumentNode representing the 
   *        <library_lights> element of the COLLADA document.
   *
   * @param nodesSection A pointer to the iDocumentNode representing the 
   *        <library_nodes> element of the COLLADA document.
   *
   * @param visualScenesSection A pointer to the iDocumentNode representing the 
   *        <library_visual_scenes> element of the COLLADA document.
   *
   * @returns true on success, false otherwise
   *
   * @sa ConvertEffects(iDocumentNode *effectsSection)
   */
  virtual bool ConvertScene(iDocumentNode *camerasSection, iDocumentNode *lightsSection, iDocumentNode *visualScenesSection) = 0;

  /**
   * Converts the rigging and animation sections of the COLLADA file
   */
  virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection) = 0;
  
  /**
   * Converts the physics section of the COLLADA file
   */
  virtual bool ConvertPhysics(iDocumentNode *physicsSection) = 0;

  /**
   * Turn debugging warnings on or off.  
   * 
   * This will turn on all possible debug information for the plugin.  
   * It also will check to verify that files and data structures conform 
   * to specified standards.
   *
   * @param toggle If true, turns on debug warnings.
   * 
   * @notes Debug warnings are off by default.  There is a lot of information
   *        given by turning debugging warnings on.  It is suggested that it
   *        it remains off unless there appears to be a problem with the plugin.
   */
  virtual void SetWarnings(bool toggle=false) = 0;

  /**
   * Set if each scene is an entire sector.
   * Else the top level objects in each scene are considered a sector.
   *
   * @param toggle If true, each scene is considered a sector.
   */
  virtual void SetSectorScene(bool toggle) = 0;
};

#endif

