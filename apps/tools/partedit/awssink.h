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



  void SetVFS(csRef<iVFS> newvfs);
  csRef<iVFS> GetVFS();

};

#endif // __AWS_SINK_TEST_H__


