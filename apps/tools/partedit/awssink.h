#ifndef __AWS_SINK_H__
#define __AWS_SINK_H__

#include "iaws/aws.h"
#include "iutil/string.h"

class csString;


typedef struct st_EmitterState {
    int particle_count,particle_max_age;
    bool lighting;
    bool rectangular_particles;
    float rect_w,rect_h;
    int reg_number;
    float reg_radius;
    bool using_bounding_box;
    float bbox_minx,bbox_miny,bbox_minz;
    float bbox_maxx,bbox_maxy,bbox_maxz;
    bool alpha_blend;
} EmitterState;

typedef struct st_Emitter3DState {
    csVector3 fixed_position;
    float fixed_weight;
    csVector3 box_min,box_max;
    float box_weight;
    csVector3 sphere_center;
    float sphere_min,sphere_max;
    float sphere_weight;
    csVector3 cone_origin;
    float cone_elevation,cone_azimuth,cone_aperture,cone_min,cone_max;
    float cone_weight;
    csVector3 line_start,line_end;
    float line_weight;
    csVector3 cylinder_start,cylinder_end;
    float cylinder_min,cylinder_max;
    float cylinder_weight;
    csVector3 spheretangent_center;
    float spheretangent_min,spheretangent_max;
    float spheretangent_weight;
    csVector3 cylindertangent_start,cylindertangent_end;
    float cylindertangent_min,cylindertangent_max;
    float cylindertangent_weight;
} Emitter3DState;

typedef struct st_AttractorState {
  Emitter3DState e3d_state;
  float force;
} AttractorState;

enum Sections {
  SECTION_GRAPHIC = 0,
  SECTION_EMITTER_STATE,
  SECTION_INIT_POSITION,
  SECTION_INIT_SPEED,
  SECTION_INIT_ACCELERATION,
  SECTION_FIELD_SPEED,
  SECTION_FIELD_ACCELERATION,
  SECTION_ATTRACTOR,
  SECTION_AGING_MOMENTS,
  SECTION_LOADSAVE,
  SECTION_FINAL_SECTION
};

enum Emitters {
  EMITTER_NONE = 0,
  EMITTER_POINT,
  EMITTER_LINE,
  EMITTER_BOX,
  EMITTER_SPHERE,
  EMITTER_CYLINDER,
  EMITTER_CONE,
  EMITTER_SPHERETANGENT,
  EMITTER_CYLINDERTANGENT,
  EMITTER_MIX
};

#define SECTION_COUNT SECTION_FINAL_SECTION

class awsSink
{

  iAws     *wmgr;
  csRef<iAwsSink> sink;
  static awsSink *asink;
  csRef<iVFS> vfs;

  
  struct st_GraphicSelectionData {
    ///  The iAwsComponent interface to the to the window for selecting the graphics file.
    iAwsComponent *iawscomponent_GraphicSelection;
    ///  The iAwsComponent interface to the to the window for selecting the graphics file.
    iAwsComponent *iawscomponent_GraphicFileList;
    /// The iAwsComponent interface to the to the text box containing the current graphics filter
    iAwsComponent *iawscomponent_GraphicFilter;
    csRef<scfString> filter;
    csRef<scfString> currentdirectory;
    csRef<iString> currentfilepath;
  } GraphicSelectionData;


  struct st_EmitterStateData {
    ///  The iAwsComponent interface to the to the window for setting emitter state parameters.
    iAwsComponent *iawscomponent_EmitterState;
    iAwsComponent *iawscomponent_ParticleCount;
    iAwsComponent *iawscomponent_ParticleMaxAge;
    iAwsComponent *iawscomponent_Lighting;
    iAwsComponent *iawscomponent_AlphaBlend;
    iAwsComponent *iawscomponent_RectParticlesRadio;
    iAwsComponent *iawscomponent_RegParticlesRadio;
    iAwsComponent *iawscomponent_RectParticlesWidth;
    iAwsComponent *iawscomponent_RectParticlesHeight;
    iAwsComponent *iawscomponent_RegParticlesNumber;
    iAwsComponent *iawscomponent_RegParticlesRadius;
    iAwsComponent *iawscomponent_UseBoundingBox;
    iAwsComponent *iawscomponent_BBoxMinX;
    iAwsComponent *iawscomponent_BBoxMinY;
    iAwsComponent *iawscomponent_BBoxMinZ;
    iAwsComponent *iawscomponent_BBoxMaxX;
    iAwsComponent *iawscomponent_BBoxMaxY;
    iAwsComponent *iawscomponent_BBoxMaxZ;
    EmitterState state;
    bool settings_changed;
  } EmitterStateData;



  struct st_InitialPositionData {
    iAwsComponent *iawscomponent_InitialPosition;
    iAwsComponent *iawscomponent_IPFPX;
    iAwsComponent *iawscomponent_IPFPY;
    iAwsComponent *iawscomponent_IPFPZ;
    iAwsComponent *iawscomponent_IPFWeight;
    iAwsComponent *iawscomponent_IPLSX;
    iAwsComponent *iawscomponent_IPLSY;
    iAwsComponent *iawscomponent_IPLSZ;
    iAwsComponent *iawscomponent_IPLEX;
    iAwsComponent *iawscomponent_IPLEY;
    iAwsComponent *iawscomponent_IPLEZ;
    iAwsComponent *iawscomponent_IPLWeight;
    iAwsComponent *iawscomponent_IPBMX;
    iAwsComponent *iawscomponent_IPBMY;
    iAwsComponent *iawscomponent_IPBMZ;
    iAwsComponent *iawscomponent_IPBXX;
    iAwsComponent *iawscomponent_IPBXY;
    iAwsComponent *iawscomponent_IPBXZ;
    iAwsComponent *iawscomponent_IPBWeight;
    iAwsComponent *iawscomponent_IPSCX;
    iAwsComponent *iawscomponent_IPSCY;
    iAwsComponent *iawscomponent_IPSCZ;
    iAwsComponent *iawscomponent_IPSMin;
    iAwsComponent *iawscomponent_IPSMax;
    iAwsComponent *iawscomponent_IPSWeight;
    iAwsComponent *iawscomponent_IPCNOX;
    iAwsComponent *iawscomponent_IPCNOY;
    iAwsComponent *iawscomponent_IPCNOZ;
    iAwsComponent *iawscomponent_IPCNElevation;
    iAwsComponent *iawscomponent_IPCNAzimuth;
    iAwsComponent *iawscomponent_IPCNAperture;
    iAwsComponent *iawscomponent_IPCNMin;
    iAwsComponent *iawscomponent_IPCNMax;
    iAwsComponent *iawscomponent_IPCNWeight;
    iAwsComponent *iawscomponent_IPCYSX;
    iAwsComponent *iawscomponent_IPCYSY;
    iAwsComponent *iawscomponent_IPCYSZ;
    iAwsComponent *iawscomponent_IPCYEX;
    iAwsComponent *iawscomponent_IPCYEY;
    iAwsComponent *iawscomponent_IPCYEZ;
    iAwsComponent *iawscomponent_IPCYMin;
    iAwsComponent *iawscomponent_IPCYMax;
    iAwsComponent *iawscomponent_IPCYWeight;
    iAwsComponent *iawscomponent_IPSTCX;
    iAwsComponent *iawscomponent_IPSTCY;
    iAwsComponent *iawscomponent_IPSTCZ;
    iAwsComponent *iawscomponent_IPSTMin;
    iAwsComponent *iawscomponent_IPSTMax;
    iAwsComponent *iawscomponent_IPSTWeight;
    iAwsComponent *iawscomponent_IPCYTSX;
    iAwsComponent *iawscomponent_IPCYTSY;
    iAwsComponent *iawscomponent_IPCYTSZ;
    iAwsComponent *iawscomponent_IPCYTEX;
    iAwsComponent *iawscomponent_IPCYTEY;
    iAwsComponent *iawscomponent_IPCYTEZ;
    iAwsComponent *iawscomponent_IPCYTMin;
    iAwsComponent *iawscomponent_IPCYTMax;
    iAwsComponent *iawscomponent_IPCYTWeight;



    Emitter3DState state;
    bool settings_changed;
  } InitialPositionData;

  struct st_InitialSpeedData {
  } InitialSpeedData;

  struct st_InitialAccelerationData {
  } InitialAccelerationData;

  struct st_FieldSpeedData {
  } FieldSpeedData;

  struct st_FieldAccelerationData {
  } FieldAccelerationData;

  struct st_AttractorData {
    iAwsComponent *iawscomponent_Attractor;
    iAwsComponent *iawscomponent_ATForce;
    iAwsComponent *iawscomponent_ATFPX;
    iAwsComponent *iawscomponent_ATFPY;
    iAwsComponent *iawscomponent_ATFPZ;
    iAwsComponent *iawscomponent_ATFWeight;
    iAwsComponent *iawscomponent_ATLSX;
    iAwsComponent *iawscomponent_ATLSY;
    iAwsComponent *iawscomponent_ATLSZ;
    iAwsComponent *iawscomponent_ATLEX;
    iAwsComponent *iawscomponent_ATLEY;
    iAwsComponent *iawscomponent_ATLEZ;
    iAwsComponent *iawscomponent_ATLWeight;
    iAwsComponent *iawscomponent_ATBMX;
    iAwsComponent *iawscomponent_ATBMY;
    iAwsComponent *iawscomponent_ATBMZ;
    iAwsComponent *iawscomponent_ATBXX;
    iAwsComponent *iawscomponent_ATBXY;
    iAwsComponent *iawscomponent_ATBXZ;
    iAwsComponent *iawscomponent_ATBWeight;
    iAwsComponent *iawscomponent_ATSCX;
    iAwsComponent *iawscomponent_ATSCY;
    iAwsComponent *iawscomponent_ATSCZ;
    iAwsComponent *iawscomponent_ATSMin;
    iAwsComponent *iawscomponent_ATSMax;
    iAwsComponent *iawscomponent_ATSWeight;
    iAwsComponent *iawscomponent_ATCNOX;
    iAwsComponent *iawscomponent_ATCNOY;
    iAwsComponent *iawscomponent_ATCNOZ;
    iAwsComponent *iawscomponent_ATCNElevation;
    iAwsComponent *iawscomponent_ATCNAzimuth;
    iAwsComponent *iawscomponent_ATCNAperture;
    iAwsComponent *iawscomponent_ATCNMin;
    iAwsComponent *iawscomponent_ATCNMax;
    iAwsComponent *iawscomponent_ATCNWeight;
    iAwsComponent *iawscomponent_ATCYSX;
    iAwsComponent *iawscomponent_ATCYSY;
    iAwsComponent *iawscomponent_ATCYSZ;
    iAwsComponent *iawscomponent_ATCYEX;
    iAwsComponent *iawscomponent_ATCYEY;
    iAwsComponent *iawscomponent_ATCYEZ;
    iAwsComponent *iawscomponent_ATCYMin;
    iAwsComponent *iawscomponent_ATCYMax;
    iAwsComponent *iawscomponent_ATCYWeight;
    iAwsComponent *iawscomponent_ATSTCX;
    iAwsComponent *iawscomponent_ATSTCY;
    iAwsComponent *iawscomponent_ATSTCZ;
    iAwsComponent *iawscomponent_ATSTMin;
    iAwsComponent *iawscomponent_ATSTMax;
    iAwsComponent *iawscomponent_ATSTWeight;
    iAwsComponent *iawscomponent_ATCYTSX;
    iAwsComponent *iawscomponent_ATCYTSY;
    iAwsComponent *iawscomponent_ATCYTSZ;
    iAwsComponent *iawscomponent_ATCYTEX;
    iAwsComponent *iawscomponent_ATCYTEY;
    iAwsComponent *iawscomponent_ATCYTEZ;
    iAwsComponent *iawscomponent_ATCYTMin;
    iAwsComponent *iawscomponent_ATCYTMax;
    iAwsComponent *iawscomponent_ATCYTWeight;

    AttractorState state;
    bool settings_changed;
  } AttractorData;

  struct st_AgingMomentsData {
  } AgingMomentsData;

  struct st_LoadSaveData {
  } LoadSaveData;

  /*  The iAwsComponent interface to the left sidebar (Section Selection Menu)
   *    This is initialized when the .def file is loaded and parsed.
   */
  iAwsComponent *iawscomponent_SectionSelection;

  /*  The iAwsComponent interface to the to the list box in the left sidebar
   *    This is initialized when the .def file is loaded and parsed.
   */
  iAwsComponent *iawscomponent_SectionList;


//  iAwsWindow *test;

  bool SectionState[SECTION_COUNT];

private:

  /// Static callback to handle left sidebar registration during .def file parsing.
  static void RegisterSectionSelection(void *sk, iAwsSource *source);

  /// Static callback to handle left sidebar list box registration during .def file parsing.
  static void FillSectionList(void *sk, iAwsSource *source);

  /// Static callback to handle selection/state changes in the section list box
  static void awsSink::SectionListSelectionChanged(void *sk, iAwsSource *source);



  /// Static callback to handle graphic file selection window registration during .def file parsing.
  static void RegisterGraphicSelection(void *sk, iAwsSource *source);
  /// Static callback to handle graphic file filter text box registration during .def file parsing.
  static void RegisterGraphicFilter(void *sk, iAwsSource *source);
  /// Static callback to handle graphic file list registration during .def file parsing
  static void RegisterGraphicFileList(void *sk, iAwsSource *source);
  /// Static callback to handle graphic file filter change.
  static void AwsSetGraphicFilter(void *sk, iAwsSource *source);
  /// Static callback to handle graphic file selection change
  static void AwsGraphicFileSelected(void *sk, iAwsSource *source);


  /// Static callback to handle emitter state option window registration during .def file parsing.
  static void RegisterEmitterState(void *sk, iAwsSource *source);
  static void RegisterParticleCount(void *sk, iAwsSource *source);
  static void RegisterParticleMaxAge(void *sk, iAwsSource *source);
  static void RegisterLighting(void *sk, iAwsSource *source);
  static void RegisterAlphaBlend(void *sk, iAwsSource *source);
  static void RegisterRectParticlesRadio(void *sk, iAwsSource *source);
  static void RegisterRegParticlesRadio(void *sk, iAwsSource *source);
  static void RegisterRectParticlesWidth(void *sk, iAwsSource *source);
  static void RegisterRectParticlesHeight(void *sk, iAwsSource *source);
  static void RegisterRegParticlesNumber(void *sk, iAwsSource *source);
  static void RegisterRegParticlesRadius(void *sk, iAwsSource *source);
  static void RegisterUseBoundingBox(void *sk, iAwsSource *source);
  static void RegisterBBoxMinX(void *sk, iAwsSource *source);
  static void RegisterBBoxMinY(void *sk, iAwsSource *source);
  static void RegisterBBoxMinZ(void *sk, iAwsSource *source);
  static void RegisterBBoxMaxX(void *sk, iAwsSource *source);
  static void RegisterBBoxMaxY(void *sk, iAwsSource *source);
  static void RegisterBBoxMaxZ(void *sk, iAwsSource *source);

  static void AwsSetParticleCount(void *sk, iAwsSource *source);
  static void AwsSetParticleMaxAge(void *sk, iAwsSource *source);
  static void AwsSetLighting(void *sk, iAwsSource *source);
  static void AwsSetAlphaBlend(void *sk, iAwsSource *source);
  static void AwsSetParticleType(void *sk, iAwsSource *source);
  static void AwsSetRectangularWidth(void *sk, iAwsSource *source);
  static void AwsSetRectangularHeight(void *sk, iAwsSource *source);
  static void AwsSetRegularNumber(void *sk, iAwsSource *source);
  static void AwsSetRegularRadius(void *sk, iAwsSource *source);
  static void AwsSetUseBoundingBox(void *sk, iAwsSource *source);
  static void AwsSetBBoxMinX(void *sk, iAwsSource *source);
  static void AwsSetBBoxMinY(void *sk, iAwsSource *source);
  static void AwsSetBBoxMinZ(void *sk, iAwsSource *source);
  static void AwsSetBBoxMaxX(void *sk, iAwsSource *source);
  static void AwsSetBBoxMaxY(void *sk, iAwsSource *source);
  static void AwsSetBBoxMaxZ(void *sk, iAwsSource *source);

  /// Static callback to handle initial position option window registration during .def file parsing.
  static void RegisterInitialPosition(void *sk, iAwsSource *source);
  static void RegisterIPFPX(void *sk, iAwsSource *source);
  static void RegisterIPFPY(void *sk, iAwsSource *source);
  static void RegisterIPFPZ(void *sk, iAwsSource *source);
  static void RegisterIPFWeight(void *sk, iAwsSource *source);
  static void RegisterIPLSX(void *sk, iAwsSource *source);
  static void RegisterIPLSY(void *sk, iAwsSource *source);
  static void RegisterIPLSZ(void *sk, iAwsSource *source);
  static void RegisterIPLEX(void *sk, iAwsSource *source);
  static void RegisterIPLEY(void *sk, iAwsSource *source);
  static void RegisterIPLEZ(void *sk, iAwsSource *source);
  static void RegisterIPLWeight(void *sk, iAwsSource *source);
  static void RegisterIPBMX(void *sk, iAwsSource *source);
  static void RegisterIPBMY(void *sk, iAwsSource *source);
  static void RegisterIPBMZ(void *sk, iAwsSource *source);
  static void RegisterIPBXX(void *sk, iAwsSource *source);
  static void RegisterIPBXY(void *sk, iAwsSource *source);
  static void RegisterIPBXZ(void *sk, iAwsSource *source);
  static void RegisterIPBWeight(void *sk, iAwsSource *source);
  static void RegisterIPSCX(void *sk, iAwsSource *source);
  static void RegisterIPSCY(void *sk, iAwsSource *source);
  static void RegisterIPSCZ(void *sk, iAwsSource *source);
  static void RegisterIPSMin(void *sk, iAwsSource *source);
  static void RegisterIPSMax(void *sk, iAwsSource *source);
  static void RegisterIPSWeight(void *sk, iAwsSource *source);
  static void RegisterIPCNOX(void *sk, iAwsSource *source);
  static void RegisterIPCNOY(void *sk, iAwsSource *source);
  static void RegisterIPCNOZ(void *sk, iAwsSource *source);
  static void RegisterIPCNElev(void *sk, iAwsSource *source);
  static void RegisterIPCNAzim(void *sk, iAwsSource *source);
  static void RegisterIPCNAper(void *sk, iAwsSource *source);
  static void RegisterIPCNMin(void *sk, iAwsSource *source);
  static void RegisterIPCNMax(void *sk, iAwsSource *source);
  static void RegisterIPCNWeight(void *sk, iAwsSource *source);
  static void RegisterIPCYSX(void *sk, iAwsSource *source);
  static void RegisterIPCYSY(void *sk, iAwsSource *source);
  static void RegisterIPCYSZ(void *sk, iAwsSource *source);
  static void RegisterIPCYEX(void *sk, iAwsSource *source);
  static void RegisterIPCYEY(void *sk, iAwsSource *source);
  static void RegisterIPCYEZ(void *sk, iAwsSource *source);
  static void RegisterIPCYMin(void *sk, iAwsSource *source);
  static void RegisterIPCYMax(void *sk, iAwsSource *source);
  static void RegisterIPCYWeight(void *sk, iAwsSource *source);
  static void RegisterIPSTCX(void *sk, iAwsSource *source);
  static void RegisterIPSTCY(void *sk, iAwsSource *source);
  static void RegisterIPSTCZ(void *sk, iAwsSource *source);
  static void RegisterIPSTMin(void *sk, iAwsSource *source);
  static void RegisterIPSTMax(void *sk, iAwsSource *source);
  static void RegisterIPSTWeight(void *sk, iAwsSource *source);
  static void RegisterIPCYTSX(void *sk, iAwsSource *source);
  static void RegisterIPCYTSY(void *sk, iAwsSource *source);
  static void RegisterIPCYTSZ(void *sk, iAwsSource *source);
  static void RegisterIPCYTEX(void *sk, iAwsSource *source);
  static void RegisterIPCYTEY(void *sk, iAwsSource *source);
  static void RegisterIPCYTEZ(void *sk, iAwsSource *source);
  static void RegisterIPCYTMin(void *sk, iAwsSource *source);
  static void RegisterIPCYTMax(void *sk, iAwsSource *source);
  static void RegisterIPCYTWeight(void *sk, iAwsSource *source);

  static void AwsSetIPFPositionX(void *sk, iAwsSource *source);
  static void AwsSetIPFPositionY(void *sk, iAwsSource *source);
  static void AwsSetIPFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetIPFWeight(void *sk, iAwsSource *source);
  static void AwsSetIPLStartX(void *sk, iAwsSource *source);
  static void AwsSetIPLStartY(void *sk, iAwsSource *source);
  static void AwsSetIPLStartZ(void *sk, iAwsSource *source);
  static void AwsSetIPLEndX(void *sk, iAwsSource *source);
  static void AwsSetIPLEndY(void *sk, iAwsSource *source);
  static void AwsSetIPLEndZ(void *sk, iAwsSource *source);
  static void AwsSetIPLWeight(void *sk, iAwsSource *source);
  static void AwsSetIPBMinX(void *sk, iAwsSource *source);
  static void AwsSetIPBMinY(void *sk, iAwsSource *source);
  static void AwsSetIPBMinZ(void *sk, iAwsSource *source);
  static void AwsSetIPBMaxX(void *sk, iAwsSource *source);
  static void AwsSetIPBMaxY(void *sk, iAwsSource *source);
  static void AwsSetIPBMaxZ(void *sk, iAwsSource *source);
  static void AwsSetIPBWeight(void *sk, iAwsSource *source);
  static void AwsSetIPSCenterX(void *sk, iAwsSource *source);
  static void AwsSetIPSCenterY(void *sk, iAwsSource *source);
  static void AwsSetIPSCenterZ(void *sk, iAwsSource *source);
  static void AwsSetIPSMin(void *sk, iAwsSource *source);
  static void AwsSetIPSMax(void *sk, iAwsSource *source);
  static void AwsSetIPSWeight(void *sk, iAwsSource *source);
  static void AwsSetIPCNOriginX(void *sk, iAwsSource *source);
  static void AwsSetIPCNOriginY(void *sk, iAwsSource *source);
  static void AwsSetIPCNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetIPCNElev(void *sk, iAwsSource *source);
  static void AwsSetIPCNAzim(void *sk, iAwsSource *source);
  static void AwsSetIPCNAper(void *sk, iAwsSource *source);
  static void AwsSetIPCNMin(void *sk, iAwsSource *source);
  static void AwsSetIPCNMax(void *sk, iAwsSource *source);
  static void AwsSetIPCNWeight(void *sk, iAwsSource *source);
  static void AwsSetIPCYStartX(void *sk, iAwsSource *source);
  static void AwsSetIPCYStartY(void *sk, iAwsSource *source);
  static void AwsSetIPCYStartZ(void *sk, iAwsSource *source);
  static void AwsSetIPCYEndX(void *sk, iAwsSource *source);
  static void AwsSetIPCYEndY(void *sk, iAwsSource *source);
  static void AwsSetIPCYEndZ(void *sk, iAwsSource *source);
  static void AwsSetIPCYMin(void *sk, iAwsSource *source);
  static void AwsSetIPCYMax(void *sk, iAwsSource *source);
  static void AwsSetIPCYWeight(void *sk, iAwsSource *source);
  static void AwsSetIPSTCenterX(void *sk, iAwsSource *source);
  static void AwsSetIPSTCenterY(void *sk, iAwsSource *source);
  static void AwsSetIPSTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetIPSTMin(void *sk, iAwsSource *source);
  static void AwsSetIPSTMax(void *sk, iAwsSource *source);
  static void AwsSetIPSTWeight(void *sk, iAwsSource *source);
  static void AwsSetIPCYTStartX(void *sk, iAwsSource *source);
  static void AwsSetIPCYTStartY(void *sk, iAwsSource *source);
  static void AwsSetIPCYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetIPCYTEndX(void *sk, iAwsSource *source);
  static void AwsSetIPCYTEndY(void *sk, iAwsSource *source);
  static void AwsSetIPCYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetIPCYTMin(void *sk, iAwsSource *source);
  static void AwsSetIPCYTMax(void *sk, iAwsSource *source);
  static void AwsSetIPCYTWeight(void *sk, iAwsSource *source);


  /// Static callback to handle attractor option window registration during .def file parsing.
  static void RegisterAttractor(void *sk, iAwsSource *source);
  static void RegisterATForce(void *sk, iAwsSource *source);
  static void RegisterATFPX(void *sk, iAwsSource *source);
  static void RegisterATFPY(void *sk, iAwsSource *source);
  static void RegisterATFPZ(void *sk, iAwsSource *source);
  static void RegisterATFWeight(void *sk, iAwsSource *source);
  static void RegisterATLSX(void *sk, iAwsSource *source);
  static void RegisterATLSY(void *sk, iAwsSource *source);
  static void RegisterATLSZ(void *sk, iAwsSource *source);
  static void RegisterATLEX(void *sk, iAwsSource *source);
  static void RegisterATLEY(void *sk, iAwsSource *source);
  static void RegisterATLEZ(void *sk, iAwsSource *source);
  static void RegisterATLWeight(void *sk, iAwsSource *source);
  static void RegisterATBMX(void *sk, iAwsSource *source);
  static void RegisterATBMY(void *sk, iAwsSource *source);
  static void RegisterATBMZ(void *sk, iAwsSource *source);
  static void RegisterATBXX(void *sk, iAwsSource *source);
  static void RegisterATBXY(void *sk, iAwsSource *source);
  static void RegisterATBXZ(void *sk, iAwsSource *source);
  static void RegisterATBWeight(void *sk, iAwsSource *source);
  static void RegisterATSCX(void *sk, iAwsSource *source);
  static void RegisterATSCY(void *sk, iAwsSource *source);
  static void RegisterATSCZ(void *sk, iAwsSource *source);
  static void RegisterATSMin(void *sk, iAwsSource *source);
  static void RegisterATSMax(void *sk, iAwsSource *source);
  static void RegisterATSWeight(void *sk, iAwsSource *source);
  static void RegisterATCNOX(void *sk, iAwsSource *source);
  static void RegisterATCNOY(void *sk, iAwsSource *source);
  static void RegisterATCNOZ(void *sk, iAwsSource *source);
  static void RegisterATCNElev(void *sk, iAwsSource *source);
  static void RegisterATCNAzim(void *sk, iAwsSource *source);
  static void RegisterATCNAper(void *sk, iAwsSource *source);
  static void RegisterATCNMin(void *sk, iAwsSource *source);
  static void RegisterATCNMax(void *sk, iAwsSource *source);
  static void RegisterATCNWeight(void *sk, iAwsSource *source);
  static void RegisterATCYSX(void *sk, iAwsSource *source);
  static void RegisterATCYSY(void *sk, iAwsSource *source);
  static void RegisterATCYSZ(void *sk, iAwsSource *source);
  static void RegisterATCYEX(void *sk, iAwsSource *source);
  static void RegisterATCYEY(void *sk, iAwsSource *source);
  static void RegisterATCYEZ(void *sk, iAwsSource *source);
  static void RegisterATCYMin(void *sk, iAwsSource *source);
  static void RegisterATCYMax(void *sk, iAwsSource *source);
  static void RegisterATCYWeight(void *sk, iAwsSource *source);
  static void RegisterATSTCX(void *sk, iAwsSource *source);
  static void RegisterATSTCY(void *sk, iAwsSource *source);
  static void RegisterATSTCZ(void *sk, iAwsSource *source);
  static void RegisterATSTMin(void *sk, iAwsSource *source);
  static void RegisterATSTMax(void *sk, iAwsSource *source);
  static void RegisterATSTWeight(void *sk, iAwsSource *source);
  static void RegisterATCYTSX(void *sk, iAwsSource *source);
  static void RegisterATCYTSY(void *sk, iAwsSource *source);
  static void RegisterATCYTSZ(void *sk, iAwsSource *source);
  static void RegisterATCYTEX(void *sk, iAwsSource *source);
  static void RegisterATCYTEY(void *sk, iAwsSource *source);
  static void RegisterATCYTEZ(void *sk, iAwsSource *source);
  static void RegisterATCYTMin(void *sk, iAwsSource *source);
  static void RegisterATCYTMax(void *sk, iAwsSource *source);
  static void RegisterATCYTWeight(void *sk, iAwsSource *source);


  static void AwsSetATForce(void *sk, iAwsSource *source);
  static void AwsSetATFPositionX(void *sk, iAwsSource *source);
  static void AwsSetATFPositionY(void *sk, iAwsSource *source);
  static void AwsSetATFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetATFWeight(void *sk, iAwsSource *source);
  static void AwsSetATLStartX(void *sk, iAwsSource *source);
  static void AwsSetATLStartY(void *sk, iAwsSource *source);
  static void AwsSetATLStartZ(void *sk, iAwsSource *source);
  static void AwsSetATLEndX(void *sk, iAwsSource *source);
  static void AwsSetATLEndY(void *sk, iAwsSource *source);
  static void AwsSetATLEndZ(void *sk, iAwsSource *source);
  static void AwsSetATLWeight(void *sk, iAwsSource *source);
  static void AwsSetATBMinX(void *sk, iAwsSource *source);
  static void AwsSetATBMinY(void *sk, iAwsSource *source);
  static void AwsSetATBMinZ(void *sk, iAwsSource *source);
  static void AwsSetATBMaxX(void *sk, iAwsSource *source);
  static void AwsSetATBMaxY(void *sk, iAwsSource *source);
  static void AwsSetATBMaxZ(void *sk, iAwsSource *source);
  static void AwsSetATBWeight(void *sk, iAwsSource *source);
  static void AwsSetATSCenterX(void *sk, iAwsSource *source);
  static void AwsSetATSCenterY(void *sk, iAwsSource *source);
  static void AwsSetATSCenterZ(void *sk, iAwsSource *source);
  static void AwsSetATSMin(void *sk, iAwsSource *source);
  static void AwsSetATSMax(void *sk, iAwsSource *source);
  static void AwsSetATSWeight(void *sk, iAwsSource *source);
  static void AwsSetATCNOriginX(void *sk, iAwsSource *source);
  static void AwsSetATCNOriginY(void *sk, iAwsSource *source);
  static void AwsSetATCNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetATCNElev(void *sk, iAwsSource *source);
  static void AwsSetATCNAzim(void *sk, iAwsSource *source);
  static void AwsSetATCNAper(void *sk, iAwsSource *source);
  static void AwsSetATCNMin(void *sk, iAwsSource *source);
  static void AwsSetATCNMax(void *sk, iAwsSource *source);
  static void AwsSetATCNWeight(void *sk, iAwsSource *source);
  static void AwsSetATCYStartX(void *sk, iAwsSource *source);
  static void AwsSetATCYStartY(void *sk, iAwsSource *source);
  static void AwsSetATCYStartZ(void *sk, iAwsSource *source);
  static void AwsSetATCYEndX(void *sk, iAwsSource *source);
  static void AwsSetATCYEndY(void *sk, iAwsSource *source);
  static void AwsSetATCYEndZ(void *sk, iAwsSource *source);
  static void AwsSetATCYMin(void *sk, iAwsSource *source);
  static void AwsSetATCYMax(void *sk, iAwsSource *source);
  static void AwsSetATCYWeight(void *sk, iAwsSource *source);
  static void AwsSetATSTCenterX(void *sk, iAwsSource *source);
  static void AwsSetATSTCenterY(void *sk, iAwsSource *source);
  static void AwsSetATSTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetATSTMin(void *sk, iAwsSource *source);
  static void AwsSetATSTMax(void *sk, iAwsSource *source);
  static void AwsSetATSTWeight(void *sk, iAwsSource *source);
  static void AwsSetATCYTStartX(void *sk, iAwsSource *source);
  static void AwsSetATCYTStartY(void *sk, iAwsSource *source);
  static void AwsSetATCYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetATCYTEndX(void *sk, iAwsSource *source);
  static void AwsSetATCYTEndY(void *sk, iAwsSource *source);
  static void AwsSetATCYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetATCYTMin(void *sk, iAwsSource *source);
  static void AwsSetATCYTMax(void *sk, iAwsSource *source);
  static void AwsSetATCYTWeight(void *sk, iAwsSource *source);



public:
  awsSink();
  virtual ~awsSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetWindowManager(iAws *_wmgr);

 

  void FillGraphicFileList();

  const char *GetGraphicFile();
  void SetGraphicFile(const char *filestr);
  const char *GetGraphicFilter();
  void SetGraphicFilter(const char *filterstr);
  const char *GetGraphicCWD();
  void SetGraphicCWD(const char *cwd);


  bool EmitterStateChanged();
  void ClearEmitterStateChanged();
  EmitterState *GetEmitterState();
  void SetEmitterState(EmitterState *source);
  void UpdateEmitterStateDisplay();

  bool InitialPositionStateChanged();
  void ClearInitialPositionStateChanged();
  Emitter3DState *GetInitialPositionState();
  void SetInitialPositionState(Emitter3DState *source);
  void UpdateInitialPositionStateDisplay();

  bool AttractorStateChanged();
  void ClearAttractorStateChanged();
  AttractorState *GetAttractorState();
  void SetAttractorState(AttractorState *source);
  void UpdateAttractorStateDisplay();


  void SetVFS(csRef<iVFS> newvfs);
  csRef<iVFS> GetVFS();

};

#endif // __AWS_SINK_TEST_H__


