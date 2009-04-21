/*
  Copyright (C) 2007 by Scott Johnson

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this application; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef	_CS_COLLADA_CONVERTOR_H_
#define	_CS_COLLADA_CONVERTOR_H_

#include "csutil/hash.h"
#include "ivaria/collada.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csgeom/trimesh.h"
#include "csColladaClasses.h"


CS_PLUGIN_NAMESPACE_BEGIN (ColladaConvertor)
{

  class csColladaEffect;

  /// The default type for mesh(fact) <plugin> tags
#define CS_COLLADA_GENMESHFACT_PLUGIN_TYPE "crystalspace.mesh.loader.factory.genmesh"
#define CS_COLLADA_GENMESH_PLUGIN_TYPE "crystalspace.mesh.loader.genmesh"
#define CS_COLLADA_TERRAIN2FACT_PLUGIN_TYPE "crystalspace.mesh.loader.factory.terrain2"
#define CS_COLLADA_TERRAIN2_PLUGIN_TYPE "ccrystalspace.mesh.loader.terrain2"

  /** 
  * This class implements the iColladaConvertor interface.  It is used as a conversion utility
  * between files in the COLLADA digital interchange format, and Crystal Space Library and/or
  * map files.
  *
  * \remarks This class requires writeable XML documents, and thus utilizes the TinyXML plugin.
  *      The TinyXML plugin will be loaded on initialization of this plugin.
  */

  class csColladaConvertor : public scfImplementation2<csColladaConvertor,iColladaConvertor,iComponent>
  {
    friend class csColladaAccessor;
    friend class csColladaMesh;
    friend class csColladaEffect;
    friend class csColladaEffectProfile;
    friend class csColladaMaterial;

  private:

    // =============== Conversion System Attributes ===============

    /// A smart pointer to the document system
    iDocumentSystem* docSys;

    /// A smart pointer to the virtual file system
    csRef<iVFS> fileSys;

    /// Whether or not we have warnings turned on. Warnings are off by default.
    bool warningsOn;

    /// Whether or not each scene is considered a sector.
    bool sectorScene;

    /// A pointer to the object registry
    iObjectRegistry* obj_reg;

    /// A list of COLLADA effects
    csArray<csColladaEffect> effectsList;

    /// The last used effect id for GenerateEffectID()
    int lastEffectId; 


    // =============== Crystal Space Attributes ===============

    /// A smart pointer to the Crystal Space document we will be working on in memory 
    csRef<iDocument> csFile;

    /// A smart pointer to the 'world' or 'library' node
    csRef<iDocumentNode> csTopNode;

    /// Whether or not the Crystal Space file has been loaded and is ready
    bool csOutputReady;

    /// The output file type.  Initially, this is set to CS_FILE_NONE.
    csColladaFileType outputFileType;

    // =============== COLLADA Attributes ===============

    /// A smart pointer to the COLLADA document we will be working from in memory
    csRef<iDocument> colladaFile;

    /// Whether or not the COLLADA file has been loaded and is ready
    bool colladaReady;

    /// A smart pointer to the <COLLADA> element
    csRef<iDocumentNode> colladaElement;

    /// An array of materials referenced in the COLLADA document
    csArray<csColladaMaterial> materialsList;

    /// Array of camera IDs.
    csArray<csString> cameraIDs;

    /// Hash of lights.
    csHash<csColladaLight, csString> lights;

    /// Names and targets of all portal objects.
    csArray<csString> portalNames;
    csArray<csString> portalTargets;

    /// User properties of each sector.
    csHash<csStringArray, csString> sectorProps;

    /// User properties of each mesh.
    csHash<csStringArray, csString> meshProps;

    // =============== Internal Functions ===============
    void WriteSectorInfo(iDocumentNode* sector);
    void WriteCameraInfo(iDocumentNode* sector, size_t camera);
    csString GetMeshFactType(csString name);
    csString GetMeshType(csString name);

    // =============== Basic Utility Functions ===============
  public:

    /**
    * Initializes the plugin.
    * 
    * \warning This will reload the iDocumentSystem interface so that it uses the TinyXML
    *     plugin as an implementation.
    */
    virtual bool Initialize (iObjectRegistry*);

    /**
    * \brief Initialization routine for the output document.
    *
    * Constructs a new Crystal Space document.  This function requires that 
    * SetOutputFileType(csColladaFileType filetype) has already been called.
    * 
    * \returns true, if initialization went ok; false otherwise
    */
    bool InitializeCrystalSpaceDocument();

    // =============== Error Reporting and Handling Functions ===============
    /**
    * Report various things back to the application
    */
    void Report(int severity, const char* msg, ...);

    /** Outputs an array.
    *
    */
    //void ReportArray(int severity, csArray<csVector3>& ar);
    //void ReportArray(int severity, csArray<csVector2>& ar);

    /**
    * Turn debugging warnings on or off.  This will turn on all possible debug information for the 
    * plugin. It also will check to verify that files and data structures conform to specified standards.
    *
    * \param toggle If true, turns on debug warnings.
    * 
    * \notes Debug warnings are off by default.
    */
    void SetWarnings(bool toggle);

    /**
    * Set if each scene is an entire sector.
    * Else the top level objects in each scene are considered a sector.
    *
    * \param toggle If true, each scene is considered a sector.
    */
    void SetSectorScene(bool toggle);

    /**
    * Checks for validity of the file name to see if it conforms to COLLADA standards.
    */
    void CheckColladaFilenameValidity(const char* str);

    /**
    * Checks for validity of the COLLADA file.
    *
    * Right now, this only checks to see if the file is valid XML.
    * @todo Add some abilities to validate the XML.
    */
    const char* CheckColladaValidity(iFile *file);

    // =============== Accessor Functions =============== 

    csRef<iDocument> GetCrystalDocument() { return csFile; }
    csRef<iDocument> GetColladaDocument() { return colladaFile; }

    /** Get the index of the specified effect in the effects list
    */
    size_t GetEffectIndex(const csColladaEffect& effect);

    /** Get the effect at a particular index in the effects list
    */
    csColladaEffect& GetEffect(size_t index);

    csColladaMaterial* FindMaterial(const char* accessorString);

  private:

    /** \brief Returns a <source> element
    *
    * Retrieves a <source> element associated with a given element.
    *
    * \param name The name of the source element (id) to retrieve.
    * \param parent The parent node
    *
    * \returns The source element associated with given element.
    */
    static csRef<iDocumentNode> GetSourceElement(const char* name, iDocumentNode* parent);

  public:

    // =============== Mutator Functions =============== 

    virtual const char* SetOutputFiletype(csColladaFileType filetype);

    // =============== Normal Class Functions =============== 

    /// Constructor
    csColladaConvertor(iBase* parent);

    /// Destructor
    virtual ~csColladaConvertor();

    virtual const char* Load(const char *str);
    virtual const char* Load(iString *str); 
    virtual const char* Load(iFile *file);
    virtual const char* Load(iDataBuffer *db);
    virtual const char* Write(const char* filepath);

    // =============== Conversion Functions ===============

    virtual const char* Convert();
    virtual bool ConvertGeometry(iDocumentNode *geometrySection);
    virtual bool ConvertEffects();
    virtual bool ConvertRiggingAnimation(iDocumentNode *riggingSection);
    virtual bool ConvertPhysics(iDocumentNode *physicsSection);
    virtual bool ConvertScene(iDocumentNode *camerasSection, iDocumentNode *lightsSection, iDocumentNode *visualScenesSection);

    // =============== Shader functions ===================
    virtual int CheckForNormalMap(csRef<iDocumentNode> texture);
    virtual bool CheckForNormalMap3DSMaxVersion(csRef<iDocumentNode> texture);
    virtual void CreateShaderVarNodes(csRef<iDocumentNode> newMaterial, csRef<iDocumentNode> effect);
    virtual void CreateShaderVarNode(csRef<iDocumentNode> newMaterial, csString name, csString value);
    virtual bool HasNormalMap(csRef<iDocumentNode> effect, csString samplerID);
    virtual bool HasHeightMap(csRef<iDocumentNode> effect, csString samplerID);
    virtual bool HasSpecMap(csRef<iDocumentNode> effect, csString samplerID);


  }; /* End of class csColladaConvertor */


} /* End of ColladaConvertor namespace */
CS_PLUGIN_NAMESPACE_END(ColladaConvertor)

#endif


