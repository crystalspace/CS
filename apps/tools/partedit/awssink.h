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

typedef struct st_FieldState {
  Emitter3DState e3d_state;
  bool active;
} FieldState;

typedef struct st_AgingMomentsState {

  int moment_count;

} AgingMomentsState;


typedef struct st_LoadSaveFileState {
  bool commandqueued;
  bool loading;
} LoadSaveFileState;

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

  struct st_FreeScrollData {
    iAwsComponent *iawscomponent_FSWindow;
    iAwsComponent *iawscomponent_FSLabel;
    iAwsComponent *iawscomponent_FSTextBox;
    iAwsComponent *iawscomponent_FSScrollBar;

    iAwsComponent *iawscomponent_AssociatedTextBox;
    bool floatval; // false if integer
    bool *invalidate_pointer;
    void *value_pointer;
  } FreeScrollData;

  
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
    iAwsComponent *iawscomponent_InitialSpeed;
    iAwsComponent *iawscomponent_ISFPX;
    iAwsComponent *iawscomponent_ISFPY;
    iAwsComponent *iawscomponent_ISFPZ;
    iAwsComponent *iawscomponent_ISFWeight;
    iAwsComponent *iawscomponent_ISLSX;
    iAwsComponent *iawscomponent_ISLSY;
    iAwsComponent *iawscomponent_ISLSZ;
    iAwsComponent *iawscomponent_ISLEX;
    iAwsComponent *iawscomponent_ISLEY;
    iAwsComponent *iawscomponent_ISLEZ;
    iAwsComponent *iawscomponent_ISLWeight;
    iAwsComponent *iawscomponent_ISBMX;
    iAwsComponent *iawscomponent_ISBMY;
    iAwsComponent *iawscomponent_ISBMZ;
    iAwsComponent *iawscomponent_ISBXX;
    iAwsComponent *iawscomponent_ISBXY;
    iAwsComponent *iawscomponent_ISBXZ;
    iAwsComponent *iawscomponent_ISBWeight;
    iAwsComponent *iawscomponent_ISSCX;
    iAwsComponent *iawscomponent_ISSCY;
    iAwsComponent *iawscomponent_ISSCZ;
    iAwsComponent *iawscomponent_ISSMin;
    iAwsComponent *iawscomponent_ISSMax;
    iAwsComponent *iawscomponent_ISSWeight;
    iAwsComponent *iawscomponent_ISCNOX;
    iAwsComponent *iawscomponent_ISCNOY;
    iAwsComponent *iawscomponent_ISCNOZ;
    iAwsComponent *iawscomponent_ISCNElevation;
    iAwsComponent *iawscomponent_ISCNAzimuth;
    iAwsComponent *iawscomponent_ISCNAperture;
    iAwsComponent *iawscomponent_ISCNMin;
    iAwsComponent *iawscomponent_ISCNMax;
    iAwsComponent *iawscomponent_ISCNWeight;
    iAwsComponent *iawscomponent_ISCYSX;
    iAwsComponent *iawscomponent_ISCYSY;
    iAwsComponent *iawscomponent_ISCYSZ;
    iAwsComponent *iawscomponent_ISCYEX;
    iAwsComponent *iawscomponent_ISCYEY;
    iAwsComponent *iawscomponent_ISCYEZ;
    iAwsComponent *iawscomponent_ISCYMin;
    iAwsComponent *iawscomponent_ISCYMax;
    iAwsComponent *iawscomponent_ISCYWeight;
    iAwsComponent *iawscomponent_ISSTCX;
    iAwsComponent *iawscomponent_ISSTCY;
    iAwsComponent *iawscomponent_ISSTCZ;
    iAwsComponent *iawscomponent_ISSTMin;
    iAwsComponent *iawscomponent_ISSTMax;
    iAwsComponent *iawscomponent_ISSTWeight;
    iAwsComponent *iawscomponent_ISCYTSX;
    iAwsComponent *iawscomponent_ISCYTSY;
    iAwsComponent *iawscomponent_ISCYTSZ;
    iAwsComponent *iawscomponent_ISCYTEX;
    iAwsComponent *iawscomponent_ISCYTEY;
    iAwsComponent *iawscomponent_ISCYTEZ;
    iAwsComponent *iawscomponent_ISCYTMin;
    iAwsComponent *iawscomponent_ISCYTMax;
    iAwsComponent *iawscomponent_ISCYTWeight;
    Emitter3DState state;
    bool settings_changed;

  } InitialSpeedData;

  struct st_InitialAccelerationData {
    iAwsComponent *iawscomponent_InitialAcceleration;
    iAwsComponent *iawscomponent_IAFPX;
    iAwsComponent *iawscomponent_IAFPY;
    iAwsComponent *iawscomponent_IAFPZ;
    iAwsComponent *iawscomponent_IAFWeight;
    iAwsComponent *iawscomponent_IALSX;
    iAwsComponent *iawscomponent_IALSY;
    iAwsComponent *iawscomponent_IALSZ;
    iAwsComponent *iawscomponent_IALEX;
    iAwsComponent *iawscomponent_IALEY;
    iAwsComponent *iawscomponent_IALEZ;
    iAwsComponent *iawscomponent_IALWeight;
    iAwsComponent *iawscomponent_IABMX;
    iAwsComponent *iawscomponent_IABMY;
    iAwsComponent *iawscomponent_IABMZ;
    iAwsComponent *iawscomponent_IABXX;
    iAwsComponent *iawscomponent_IABXY;
    iAwsComponent *iawscomponent_IABXZ;
    iAwsComponent *iawscomponent_IABWeight;
    iAwsComponent *iawscomponent_IASCX;
    iAwsComponent *iawscomponent_IASCY;
    iAwsComponent *iawscomponent_IASCZ;
    iAwsComponent *iawscomponent_IASMin;
    iAwsComponent *iawscomponent_IASMax;
    iAwsComponent *iawscomponent_IASWeight;
    iAwsComponent *iawscomponent_IACNOX;
    iAwsComponent *iawscomponent_IACNOY;
    iAwsComponent *iawscomponent_IACNOZ;
    iAwsComponent *iawscomponent_IACNElevation;
    iAwsComponent *iawscomponent_IACNAzimuth;
    iAwsComponent *iawscomponent_IACNAperture;
    iAwsComponent *iawscomponent_IACNMin;
    iAwsComponent *iawscomponent_IACNMax;
    iAwsComponent *iawscomponent_IACNWeight;
    iAwsComponent *iawscomponent_IACYSX;
    iAwsComponent *iawscomponent_IACYSY;
    iAwsComponent *iawscomponent_IACYSZ;
    iAwsComponent *iawscomponent_IACYEX;
    iAwsComponent *iawscomponent_IACYEY;
    iAwsComponent *iawscomponent_IACYEZ;
    iAwsComponent *iawscomponent_IACYMin;
    iAwsComponent *iawscomponent_IACYMax;
    iAwsComponent *iawscomponent_IACYWeight;
    iAwsComponent *iawscomponent_IASTCX;
    iAwsComponent *iawscomponent_IASTCY;
    iAwsComponent *iawscomponent_IASTCZ;
    iAwsComponent *iawscomponent_IASTMin;
    iAwsComponent *iawscomponent_IASTMax;
    iAwsComponent *iawscomponent_IASTWeight;
    iAwsComponent *iawscomponent_IACYTSX;
    iAwsComponent *iawscomponent_IACYTSY;
    iAwsComponent *iawscomponent_IACYTSZ;
    iAwsComponent *iawscomponent_IACYTEX;
    iAwsComponent *iawscomponent_IACYTEY;
    iAwsComponent *iawscomponent_IACYTEZ;
    iAwsComponent *iawscomponent_IACYTMin;
    iAwsComponent *iawscomponent_IACYTMax;
    iAwsComponent *iawscomponent_IACYTWeight;
    Emitter3DState state;
    bool settings_changed;
  } InitialAccelerationData;

  struct st_FieldSpeedData {
    iAwsComponent *iawscomponent_FieldSpeed;
    iAwsComponent *iawscomponent_FSActive;
    iAwsComponent *iawscomponent_FSFPX;
    iAwsComponent *iawscomponent_FSFPY;
    iAwsComponent *iawscomponent_FSFPZ;
    iAwsComponent *iawscomponent_FSFWeight;
    iAwsComponent *iawscomponent_FSLSX;
    iAwsComponent *iawscomponent_FSLSY;
    iAwsComponent *iawscomponent_FSLSZ;
    iAwsComponent *iawscomponent_FSLEX;
    iAwsComponent *iawscomponent_FSLEY;
    iAwsComponent *iawscomponent_FSLEZ;
    iAwsComponent *iawscomponent_FSLWeight;
    iAwsComponent *iawscomponent_FSBMX;
    iAwsComponent *iawscomponent_FSBMY;
    iAwsComponent *iawscomponent_FSBMZ;
    iAwsComponent *iawscomponent_FSBXX;
    iAwsComponent *iawscomponent_FSBXY;
    iAwsComponent *iawscomponent_FSBXZ;
    iAwsComponent *iawscomponent_FSBWeight;
    iAwsComponent *iawscomponent_FSSCX;
    iAwsComponent *iawscomponent_FSSCY;
    iAwsComponent *iawscomponent_FSSCZ;
    iAwsComponent *iawscomponent_FSSMin;
    iAwsComponent *iawscomponent_FSSMax;
    iAwsComponent *iawscomponent_FSSWeight;
    iAwsComponent *iawscomponent_FSCNOX;
    iAwsComponent *iawscomponent_FSCNOY;
    iAwsComponent *iawscomponent_FSCNOZ;
    iAwsComponent *iawscomponent_FSCNElevation;
    iAwsComponent *iawscomponent_FSCNAzimuth;
    iAwsComponent *iawscomponent_FSCNAperture;
    iAwsComponent *iawscomponent_FSCNMin;
    iAwsComponent *iawscomponent_FSCNMax;
    iAwsComponent *iawscomponent_FSCNWeight;
    iAwsComponent *iawscomponent_FSCYSX;
    iAwsComponent *iawscomponent_FSCYSY;
    iAwsComponent *iawscomponent_FSCYSZ;
    iAwsComponent *iawscomponent_FSCYEX;
    iAwsComponent *iawscomponent_FSCYEY;
    iAwsComponent *iawscomponent_FSCYEZ;
    iAwsComponent *iawscomponent_FSCYMin;
    iAwsComponent *iawscomponent_FSCYMax;
    iAwsComponent *iawscomponent_FSCYWeight;
    iAwsComponent *iawscomponent_FSSTCX;
    iAwsComponent *iawscomponent_FSSTCY;
    iAwsComponent *iawscomponent_FSSTCZ;
    iAwsComponent *iawscomponent_FSSTMin;
    iAwsComponent *iawscomponent_FSSTMax;
    iAwsComponent *iawscomponent_FSSTWeight;
    iAwsComponent *iawscomponent_FSCYTSX;
    iAwsComponent *iawscomponent_FSCYTSY;
    iAwsComponent *iawscomponent_FSCYTSZ;
    iAwsComponent *iawscomponent_FSCYTEX;
    iAwsComponent *iawscomponent_FSCYTEY;
    iAwsComponent *iawscomponent_FSCYTEZ;
    iAwsComponent *iawscomponent_FSCYTMin;
    iAwsComponent *iawscomponent_FSCYTMax;
    iAwsComponent *iawscomponent_FSCYTWeight;
    FieldState state;
    bool settings_changed;
  } FieldSpeedData;

  struct st_FieldAccelerationData {
    iAwsComponent *iawscomponent_FieldAccel;
    iAwsComponent *iawscomponent_FAActive;
    iAwsComponent *iawscomponent_FAFPX;
    iAwsComponent *iawscomponent_FAFPY;
    iAwsComponent *iawscomponent_FAFPZ;
    iAwsComponent *iawscomponent_FAFWeight;
    iAwsComponent *iawscomponent_FALSX;
    iAwsComponent *iawscomponent_FALSY;
    iAwsComponent *iawscomponent_FALSZ;
    iAwsComponent *iawscomponent_FALEX;
    iAwsComponent *iawscomponent_FALEY;
    iAwsComponent *iawscomponent_FALEZ;
    iAwsComponent *iawscomponent_FALWeight;
    iAwsComponent *iawscomponent_FABMX;
    iAwsComponent *iawscomponent_FABMY;
    iAwsComponent *iawscomponent_FABMZ;
    iAwsComponent *iawscomponent_FABXX;
    iAwsComponent *iawscomponent_FABXY;
    iAwsComponent *iawscomponent_FABXZ;
    iAwsComponent *iawscomponent_FABWeight;
    iAwsComponent *iawscomponent_FASCX;
    iAwsComponent *iawscomponent_FASCY;
    iAwsComponent *iawscomponent_FASCZ;
    iAwsComponent *iawscomponent_FASMin;
    iAwsComponent *iawscomponent_FASMax;
    iAwsComponent *iawscomponent_FASWeight;
    iAwsComponent *iawscomponent_FACNOX;
    iAwsComponent *iawscomponent_FACNOY;
    iAwsComponent *iawscomponent_FACNOZ;
    iAwsComponent *iawscomponent_FACNElevation;
    iAwsComponent *iawscomponent_FACNAzimuth;
    iAwsComponent *iawscomponent_FACNAperture;
    iAwsComponent *iawscomponent_FACNMin;
    iAwsComponent *iawscomponent_FACNMax;
    iAwsComponent *iawscomponent_FACNWeight;
    iAwsComponent *iawscomponent_FACYSX;
    iAwsComponent *iawscomponent_FACYSY;
    iAwsComponent *iawscomponent_FACYSZ;
    iAwsComponent *iawscomponent_FACYEX;
    iAwsComponent *iawscomponent_FACYEY;
    iAwsComponent *iawscomponent_FACYEZ;
    iAwsComponent *iawscomponent_FACYMin;
    iAwsComponent *iawscomponent_FACYMax;
    iAwsComponent *iawscomponent_FACYWeight;
    iAwsComponent *iawscomponent_FASTCX;
    iAwsComponent *iawscomponent_FASTCY;
    iAwsComponent *iawscomponent_FASTCZ;
    iAwsComponent *iawscomponent_FASTMin;
    iAwsComponent *iawscomponent_FASTMax;
    iAwsComponent *iawscomponent_FASTWeight;
    iAwsComponent *iawscomponent_FACYTSX;
    iAwsComponent *iawscomponent_FACYTSY;
    iAwsComponent *iawscomponent_FACYTSZ;
    iAwsComponent *iawscomponent_FACYTEX;
    iAwsComponent *iawscomponent_FACYTEY;
    iAwsComponent *iawscomponent_FACYTEZ;
    iAwsComponent *iawscomponent_FACYTMin;
    iAwsComponent *iawscomponent_FACYTMax;
    iAwsComponent *iawscomponent_FACYTWeight;
    FieldState state;
    bool settings_changed;
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
    iAwsComponent *iawscomponent_AgingMoments;
  } AgingMomentsData;

  struct st_LoadSaveData {
    iAwsComponent *iawscomponent_LoadSave;
    LoadSaveFileState state;
    bool settings_changed;
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


  /// Static callback to handle initial speed option window registration during .def file parsing.
  static void RegisterInitialSpeed(void *sk, iAwsSource *source);
  static void RegisterISFPX(void *sk, iAwsSource *source);
  static void RegisterISFPY(void *sk, iAwsSource *source);
  static void RegisterISFPZ(void *sk, iAwsSource *source);
  static void RegisterISFWeight(void *sk, iAwsSource *source);
  static void RegisterISLSX(void *sk, iAwsSource *source);
  static void RegisterISLSY(void *sk, iAwsSource *source);
  static void RegisterISLSZ(void *sk, iAwsSource *source);
  static void RegisterISLEX(void *sk, iAwsSource *source);
  static void RegisterISLEY(void *sk, iAwsSource *source);
  static void RegisterISLEZ(void *sk, iAwsSource *source);
  static void RegisterISLWeight(void *sk, iAwsSource *source);
  static void RegisterISBMX(void *sk, iAwsSource *source);
  static void RegisterISBMY(void *sk, iAwsSource *source);
  static void RegisterISBMZ(void *sk, iAwsSource *source);
  static void RegisterISBXX(void *sk, iAwsSource *source);
  static void RegisterISBXY(void *sk, iAwsSource *source);
  static void RegisterISBXZ(void *sk, iAwsSource *source);
  static void RegisterISBWeight(void *sk, iAwsSource *source);
  static void RegisterISSCX(void *sk, iAwsSource *source);
  static void RegisterISSCY(void *sk, iAwsSource *source);
  static void RegisterISSCZ(void *sk, iAwsSource *source);
  static void RegisterISSMin(void *sk, iAwsSource *source);
  static void RegisterISSMax(void *sk, iAwsSource *source);
  static void RegisterISSWeight(void *sk, iAwsSource *source);
  static void RegisterISCNOX(void *sk, iAwsSource *source);
  static void RegisterISCNOY(void *sk, iAwsSource *source);
  static void RegisterISCNOZ(void *sk, iAwsSource *source);
  static void RegisterISCNElev(void *sk, iAwsSource *source);
  static void RegisterISCNAzim(void *sk, iAwsSource *source);
  static void RegisterISCNAper(void *sk, iAwsSource *source);
  static void RegisterISCNMin(void *sk, iAwsSource *source);
  static void RegisterISCNMax(void *sk, iAwsSource *source);
  static void RegisterISCNWeight(void *sk, iAwsSource *source);
  static void RegisterISCYSX(void *sk, iAwsSource *source);
  static void RegisterISCYSY(void *sk, iAwsSource *source);
  static void RegisterISCYSZ(void *sk, iAwsSource *source);
  static void RegisterISCYEX(void *sk, iAwsSource *source);
  static void RegisterISCYEY(void *sk, iAwsSource *source);
  static void RegisterISCYEZ(void *sk, iAwsSource *source);
  static void RegisterISCYMin(void *sk, iAwsSource *source);
  static void RegisterISCYMax(void *sk, iAwsSource *source);
  static void RegisterISCYWeight(void *sk, iAwsSource *source);
  static void RegisterISSTCX(void *sk, iAwsSource *source);
  static void RegisterISSTCY(void *sk, iAwsSource *source);
  static void RegisterISSTCZ(void *sk, iAwsSource *source);
  static void RegisterISSTMin(void *sk, iAwsSource *source);
  static void RegisterISSTMax(void *sk, iAwsSource *source);
  static void RegisterISSTWeight(void *sk, iAwsSource *source);
  static void RegisterISCYTSX(void *sk, iAwsSource *source);
  static void RegisterISCYTSY(void *sk, iAwsSource *source);
  static void RegisterISCYTSZ(void *sk, iAwsSource *source);
  static void RegisterISCYTEX(void *sk, iAwsSource *source);
  static void RegisterISCYTEY(void *sk, iAwsSource *source);
  static void RegisterISCYTEZ(void *sk, iAwsSource *source);
  static void RegisterISCYTMin(void *sk, iAwsSource *source);
  static void RegisterISCYTMax(void *sk, iAwsSource *source);
  static void RegisterISCYTWeight(void *sk, iAwsSource *source);

  static void AwsSetISFPositionX(void *sk, iAwsSource *source);
  static void AwsSetISFPositionY(void *sk, iAwsSource *source);
  static void AwsSetISFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetISFWeight(void *sk, iAwsSource *source);
  static void AwsSetISLStartX(void *sk, iAwsSource *source);
  static void AwsSetISLStartY(void *sk, iAwsSource *source);
  static void AwsSetISLStartZ(void *sk, iAwsSource *source);
  static void AwsSetISLEndX(void *sk, iAwsSource *source);
  static void AwsSetISLEndY(void *sk, iAwsSource *source);
  static void AwsSetISLEndZ(void *sk, iAwsSource *source);
  static void AwsSetISLWeight(void *sk, iAwsSource *source);
  static void AwsSetISBMinX(void *sk, iAwsSource *source);
  static void AwsSetISBMinY(void *sk, iAwsSource *source);
  static void AwsSetISBMinZ(void *sk, iAwsSource *source);
  static void AwsSetISBMaxX(void *sk, iAwsSource *source);
  static void AwsSetISBMaxY(void *sk, iAwsSource *source);
  static void AwsSetISBMaxZ(void *sk, iAwsSource *source);
  static void AwsSetISBWeight(void *sk, iAwsSource *source);
  static void AwsSetISSCenterX(void *sk, iAwsSource *source);
  static void AwsSetISSCenterY(void *sk, iAwsSource *source);
  static void AwsSetISSCenterZ(void *sk, iAwsSource *source);
  static void AwsSetISSMin(void *sk, iAwsSource *source);
  static void AwsSetISSMax(void *sk, iAwsSource *source);
  static void AwsSetISSWeight(void *sk, iAwsSource *source);
  static void AwsSetISCNOriginX(void *sk, iAwsSource *source);
  static void AwsSetISCNOriginY(void *sk, iAwsSource *source);
  static void AwsSetISCNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetISCNElev(void *sk, iAwsSource *source);
  static void AwsSetISCNAzim(void *sk, iAwsSource *source);
  static void AwsSetISCNAper(void *sk, iAwsSource *source);
  static void AwsSetISCNMin(void *sk, iAwsSource *source);
  static void AwsSetISCNMax(void *sk, iAwsSource *source);
  static void AwsSetISCNWeight(void *sk, iAwsSource *source);
  static void AwsSetISCYStartX(void *sk, iAwsSource *source);
  static void AwsSetISCYStartY(void *sk, iAwsSource *source);
  static void AwsSetISCYStartZ(void *sk, iAwsSource *source);
  static void AwsSetISCYEndX(void *sk, iAwsSource *source);
  static void AwsSetISCYEndY(void *sk, iAwsSource *source);
  static void AwsSetISCYEndZ(void *sk, iAwsSource *source);
  static void AwsSetISCYMin(void *sk, iAwsSource *source);
  static void AwsSetISCYMax(void *sk, iAwsSource *source);
  static void AwsSetISCYWeight(void *sk, iAwsSource *source);
  static void AwsSetISSTCenterX(void *sk, iAwsSource *source);
  static void AwsSetISSTCenterY(void *sk, iAwsSource *source);
  static void AwsSetISSTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetISSTMin(void *sk, iAwsSource *source);
  static void AwsSetISSTMax(void *sk, iAwsSource *source);
  static void AwsSetISSTWeight(void *sk, iAwsSource *source);
  static void AwsSetISCYTStartX(void *sk, iAwsSource *source);
  static void AwsSetISCYTStartY(void *sk, iAwsSource *source);
  static void AwsSetISCYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetISCYTEndX(void *sk, iAwsSource *source);
  static void AwsSetISCYTEndY(void *sk, iAwsSource *source);
  static void AwsSetISCYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetISCYTMin(void *sk, iAwsSource *source);
  static void AwsSetISCYTMax(void *sk, iAwsSource *source);
  static void AwsSetISCYTWeight(void *sk, iAwsSource *source);




  /// Static callback to handle initial acceleration option window registration during .def file parsing.
  static void RegisterInitialAcceleration(void *sk, iAwsSource *source);
  static void RegisterIAFPX(void *sk, iAwsSource *source);
  static void RegisterIAFPY(void *sk, iAwsSource *source);
  static void RegisterIAFPZ(void *sk, iAwsSource *source);
  static void RegisterIAFWeight(void *sk, iAwsSource *source);
  static void RegisterIALSX(void *sk, iAwsSource *source);
  static void RegisterIALSY(void *sk, iAwsSource *source);
  static void RegisterIALSZ(void *sk, iAwsSource *source);
  static void RegisterIALEX(void *sk, iAwsSource *source);
  static void RegisterIALEY(void *sk, iAwsSource *source);
  static void RegisterIALEZ(void *sk, iAwsSource *source);
  static void RegisterIALWeight(void *sk, iAwsSource *source);
  static void RegisterIABMX(void *sk, iAwsSource *source);
  static void RegisterIABMY(void *sk, iAwsSource *source);
  static void RegisterIABMZ(void *sk, iAwsSource *source);
  static void RegisterIABXX(void *sk, iAwsSource *source);
  static void RegisterIABXY(void *sk, iAwsSource *source);
  static void RegisterIABXZ(void *sk, iAwsSource *source);
  static void RegisterIABWeight(void *sk, iAwsSource *source);
  static void RegisterIASCX(void *sk, iAwsSource *source);
  static void RegisterIASCY(void *sk, iAwsSource *source);
  static void RegisterIASCZ(void *sk, iAwsSource *source);
  static void RegisterIASMin(void *sk, iAwsSource *source);
  static void RegisterIASMax(void *sk, iAwsSource *source);
  static void RegisterIASWeight(void *sk, iAwsSource *source);
  static void RegisterIACNOX(void *sk, iAwsSource *source);
  static void RegisterIACNOY(void *sk, iAwsSource *source);
  static void RegisterIACNOZ(void *sk, iAwsSource *source);
  static void RegisterIACNElev(void *sk, iAwsSource *source);
  static void RegisterIACNAzim(void *sk, iAwsSource *source);
  static void RegisterIACNAper(void *sk, iAwsSource *source);
  static void RegisterIACNMin(void *sk, iAwsSource *source);
  static void RegisterIACNMax(void *sk, iAwsSource *source);
  static void RegisterIACNWeight(void *sk, iAwsSource *source);
  static void RegisterIACYSX(void *sk, iAwsSource *source);
  static void RegisterIACYSY(void *sk, iAwsSource *source);
  static void RegisterIACYSZ(void *sk, iAwsSource *source);
  static void RegisterIACYEX(void *sk, iAwsSource *source);
  static void RegisterIACYEY(void *sk, iAwsSource *source);
  static void RegisterIACYEZ(void *sk, iAwsSource *source);
  static void RegisterIACYMin(void *sk, iAwsSource *source);
  static void RegisterIACYMax(void *sk, iAwsSource *source);
  static void RegisterIACYWeight(void *sk, iAwsSource *source);
  static void RegisterIASTCX(void *sk, iAwsSource *source);
  static void RegisterIASTCY(void *sk, iAwsSource *source);
  static void RegisterIASTCZ(void *sk, iAwsSource *source);
  static void RegisterIASTMin(void *sk, iAwsSource *source);
  static void RegisterIASTMax(void *sk, iAwsSource *source);
  static void RegisterIASTWeight(void *sk, iAwsSource *source);
  static void RegisterIACYTSX(void *sk, iAwsSource *source);
  static void RegisterIACYTSY(void *sk, iAwsSource *source);
  static void RegisterIACYTSZ(void *sk, iAwsSource *source);
  static void RegisterIACYTEX(void *sk, iAwsSource *source);
  static void RegisterIACYTEY(void *sk, iAwsSource *source);
  static void RegisterIACYTEZ(void *sk, iAwsSource *source);
  static void RegisterIACYTMin(void *sk, iAwsSource *source);
  static void RegisterIACYTMax(void *sk, iAwsSource *source);
  static void RegisterIACYTWeight(void *sk, iAwsSource *source);

  static void AwsSetIAFPositionX(void *sk, iAwsSource *source);
  static void AwsSetIAFPositionY(void *sk, iAwsSource *source);
  static void AwsSetIAFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetIAFWeight(void *sk, iAwsSource *source);
  static void AwsSetIALStartX(void *sk, iAwsSource *source);
  static void AwsSetIALStartY(void *sk, iAwsSource *source);
  static void AwsSetIALStartZ(void *sk, iAwsSource *source);
  static void AwsSetIALEndX(void *sk, iAwsSource *source);
  static void AwsSetIALEndY(void *sk, iAwsSource *source);
  static void AwsSetIALEndZ(void *sk, iAwsSource *source);
  static void AwsSetIALWeight(void *sk, iAwsSource *source);
  static void AwsSetIABMinX(void *sk, iAwsSource *source);
  static void AwsSetIABMinY(void *sk, iAwsSource *source);
  static void AwsSetIABMinZ(void *sk, iAwsSource *source);
  static void AwsSetIABMaxX(void *sk, iAwsSource *source);
  static void AwsSetIABMaxY(void *sk, iAwsSource *source);
  static void AwsSetIABMaxZ(void *sk, iAwsSource *source);
  static void AwsSetIABWeight(void *sk, iAwsSource *source);
  static void AwsSetIASCenterX(void *sk, iAwsSource *source);
  static void AwsSetIASCenterY(void *sk, iAwsSource *source);
  static void AwsSetIASCenterZ(void *sk, iAwsSource *source);
  static void AwsSetIASMin(void *sk, iAwsSource *source);
  static void AwsSetIASMax(void *sk, iAwsSource *source);
  static void AwsSetIASWeight(void *sk, iAwsSource *source);
  static void AwsSetIACNOriginX(void *sk, iAwsSource *source);
  static void AwsSetIACNOriginY(void *sk, iAwsSource *source);
  static void AwsSetIACNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetIACNElev(void *sk, iAwsSource *source);
  static void AwsSetIACNAzim(void *sk, iAwsSource *source);
  static void AwsSetIACNAper(void *sk, iAwsSource *source);
  static void AwsSetIACNMin(void *sk, iAwsSource *source);
  static void AwsSetIACNMax(void *sk, iAwsSource *source);
  static void AwsSetIACNWeight(void *sk, iAwsSource *source);
  static void AwsSetIACYStartX(void *sk, iAwsSource *source);
  static void AwsSetIACYStartY(void *sk, iAwsSource *source);
  static void AwsSetIACYStartZ(void *sk, iAwsSource *source);
  static void AwsSetIACYEndX(void *sk, iAwsSource *source);
  static void AwsSetIACYEndY(void *sk, iAwsSource *source);
  static void AwsSetIACYEndZ(void *sk, iAwsSource *source);
  static void AwsSetIACYMin(void *sk, iAwsSource *source);
  static void AwsSetIACYMax(void *sk, iAwsSource *source);
  static void AwsSetIACYWeight(void *sk, iAwsSource *source);
  static void AwsSetIASTCenterX(void *sk, iAwsSource *source);
  static void AwsSetIASTCenterY(void *sk, iAwsSource *source);
  static void AwsSetIASTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetIASTMin(void *sk, iAwsSource *source);
  static void AwsSetIASTMax(void *sk, iAwsSource *source);
  static void AwsSetIASTWeight(void *sk, iAwsSource *source);
  static void AwsSetIACYTStartX(void *sk, iAwsSource *source);
  static void AwsSetIACYTStartY(void *sk, iAwsSource *source);
  static void AwsSetIACYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetIACYTEndX(void *sk, iAwsSource *source);
  static void AwsSetIACYTEndY(void *sk, iAwsSource *source);
  static void AwsSetIACYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetIACYTMin(void *sk, iAwsSource *source);
  static void AwsSetIACYTMax(void *sk, iAwsSource *source);
  static void AwsSetIACYTWeight(void *sk, iAwsSource *source);



  /// Static callback to handle field speed option window registration during .def file parsing.
  static void RegisterFieldSpeed(void *sk, iAwsSource *source);
  static void RegisterFSActive(void *sk, iAwsSource *source);
  static void RegisterFSFPX(void *sk, iAwsSource *source);
  static void RegisterFSFPY(void *sk, iAwsSource *source);
  static void RegisterFSFPZ(void *sk, iAwsSource *source);
  static void RegisterFSFWeight(void *sk, iAwsSource *source);
  static void RegisterFSLSX(void *sk, iAwsSource *source);
  static void RegisterFSLSY(void *sk, iAwsSource *source);
  static void RegisterFSLSZ(void *sk, iAwsSource *source);
  static void RegisterFSLEX(void *sk, iAwsSource *source);
  static void RegisterFSLEY(void *sk, iAwsSource *source);
  static void RegisterFSLEZ(void *sk, iAwsSource *source);
  static void RegisterFSLWeight(void *sk, iAwsSource *source);
  static void RegisterFSBMX(void *sk, iAwsSource *source);
  static void RegisterFSBMY(void *sk, iAwsSource *source);
  static void RegisterFSBMZ(void *sk, iAwsSource *source);
  static void RegisterFSBXX(void *sk, iAwsSource *source);
  static void RegisterFSBXY(void *sk, iAwsSource *source);
  static void RegisterFSBXZ(void *sk, iAwsSource *source);
  static void RegisterFSBWeight(void *sk, iAwsSource *source);
  static void RegisterFSSCX(void *sk, iAwsSource *source);
  static void RegisterFSSCY(void *sk, iAwsSource *source);
  static void RegisterFSSCZ(void *sk, iAwsSource *source);
  static void RegisterFSSMin(void *sk, iAwsSource *source);
  static void RegisterFSSMax(void *sk, iAwsSource *source);
  static void RegisterFSSWeight(void *sk, iAwsSource *source);
  static void RegisterFSCNOX(void *sk, iAwsSource *source);
  static void RegisterFSCNOY(void *sk, iAwsSource *source);
  static void RegisterFSCNOZ(void *sk, iAwsSource *source);
  static void RegisterFSCNElev(void *sk, iAwsSource *source);
  static void RegisterFSCNAzim(void *sk, iAwsSource *source);
  static void RegisterFSCNAper(void *sk, iAwsSource *source);
  static void RegisterFSCNMin(void *sk, iAwsSource *source);
  static void RegisterFSCNMax(void *sk, iAwsSource *source);
  static void RegisterFSCNWeight(void *sk, iAwsSource *source);
  static void RegisterFSCYSX(void *sk, iAwsSource *source);
  static void RegisterFSCYSY(void *sk, iAwsSource *source);
  static void RegisterFSCYSZ(void *sk, iAwsSource *source);
  static void RegisterFSCYEX(void *sk, iAwsSource *source);
  static void RegisterFSCYEY(void *sk, iAwsSource *source);
  static void RegisterFSCYEZ(void *sk, iAwsSource *source);
  static void RegisterFSCYMin(void *sk, iAwsSource *source);
  static void RegisterFSCYMax(void *sk, iAwsSource *source);
  static void RegisterFSCYWeight(void *sk, iAwsSource *source);
  static void RegisterFSSTCX(void *sk, iAwsSource *source);
  static void RegisterFSSTCY(void *sk, iAwsSource *source);
  static void RegisterFSSTCZ(void *sk, iAwsSource *source);
  static void RegisterFSSTMin(void *sk, iAwsSource *source);
  static void RegisterFSSTMax(void *sk, iAwsSource *source);
  static void RegisterFSSTWeight(void *sk, iAwsSource *source);
  static void RegisterFSCYTSX(void *sk, iAwsSource *source);
  static void RegisterFSCYTSY(void *sk, iAwsSource *source);
  static void RegisterFSCYTSZ(void *sk, iAwsSource *source);
  static void RegisterFSCYTEX(void *sk, iAwsSource *source);
  static void RegisterFSCYTEY(void *sk, iAwsSource *source);
  static void RegisterFSCYTEZ(void *sk, iAwsSource *source);
  static void RegisterFSCYTMin(void *sk, iAwsSource *source);
  static void RegisterFSCYTMax(void *sk, iAwsSource *source);
  static void RegisterFSCYTWeight(void *sk, iAwsSource *source);

  static void AwsSetFSActive(void *sk, iAwsSource *source);
  static void AwsSetFSFPositionX(void *sk, iAwsSource *source);
  static void AwsSetFSFPositionY(void *sk, iAwsSource *source);
  static void AwsSetFSFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetFSFWeight(void *sk, iAwsSource *source);
  static void AwsSetFSLStartX(void *sk, iAwsSource *source);
  static void AwsSetFSLStartY(void *sk, iAwsSource *source);
  static void AwsSetFSLStartZ(void *sk, iAwsSource *source);
  static void AwsSetFSLEndX(void *sk, iAwsSource *source);
  static void AwsSetFSLEndY(void *sk, iAwsSource *source);
  static void AwsSetFSLEndZ(void *sk, iAwsSource *source);
  static void AwsSetFSLWeight(void *sk, iAwsSource *source);
  static void AwsSetFSBMinX(void *sk, iAwsSource *source);
  static void AwsSetFSBMinY(void *sk, iAwsSource *source);
  static void AwsSetFSBMinZ(void *sk, iAwsSource *source);
  static void AwsSetFSBMaxX(void *sk, iAwsSource *source);
  static void AwsSetFSBMaxY(void *sk, iAwsSource *source);
  static void AwsSetFSBMaxZ(void *sk, iAwsSource *source);
  static void AwsSetFSBWeight(void *sk, iAwsSource *source);
  static void AwsSetFSSCenterX(void *sk, iAwsSource *source);
  static void AwsSetFSSCenterY(void *sk, iAwsSource *source);
  static void AwsSetFSSCenterZ(void *sk, iAwsSource *source);
  static void AwsSetFSSMin(void *sk, iAwsSource *source);
  static void AwsSetFSSMax(void *sk, iAwsSource *source);
  static void AwsSetFSSWeight(void *sk, iAwsSource *source);
  static void AwsSetFSCNOriginX(void *sk, iAwsSource *source);
  static void AwsSetFSCNOriginY(void *sk, iAwsSource *source);
  static void AwsSetFSCNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetFSCNElev(void *sk, iAwsSource *source);
  static void AwsSetFSCNAzim(void *sk, iAwsSource *source);
  static void AwsSetFSCNAper(void *sk, iAwsSource *source);
  static void AwsSetFSCNMin(void *sk, iAwsSource *source);
  static void AwsSetFSCNMax(void *sk, iAwsSource *source);
  static void AwsSetFSCNWeight(void *sk, iAwsSource *source);
  static void AwsSetFSCYStartX(void *sk, iAwsSource *source);
  static void AwsSetFSCYStartY(void *sk, iAwsSource *source);
  static void AwsSetFSCYStartZ(void *sk, iAwsSource *source);
  static void AwsSetFSCYEndX(void *sk, iAwsSource *source);
  static void AwsSetFSCYEndY(void *sk, iAwsSource *source);
  static void AwsSetFSCYEndZ(void *sk, iAwsSource *source);
  static void AwsSetFSCYMin(void *sk, iAwsSource *source);
  static void AwsSetFSCYMax(void *sk, iAwsSource *source);
  static void AwsSetFSCYWeight(void *sk, iAwsSource *source);
  static void AwsSetFSSTCenterX(void *sk, iAwsSource *source);
  static void AwsSetFSSTCenterY(void *sk, iAwsSource *source);
  static void AwsSetFSSTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetFSSTMin(void *sk, iAwsSource *source);
  static void AwsSetFSSTMax(void *sk, iAwsSource *source);
  static void AwsSetFSSTWeight(void *sk, iAwsSource *source);
  static void AwsSetFSCYTStartX(void *sk, iAwsSource *source);
  static void AwsSetFSCYTStartY(void *sk, iAwsSource *source);
  static void AwsSetFSCYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetFSCYTEndX(void *sk, iAwsSource *source);
  static void AwsSetFSCYTEndY(void *sk, iAwsSource *source);
  static void AwsSetFSCYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetFSCYTMin(void *sk, iAwsSource *source);
  static void AwsSetFSCYTMax(void *sk, iAwsSource *source);
  static void AwsSetFSCYTWeight(void *sk, iAwsSource *source);


  /// Static callback to handle field acceleration option window registration during .def file parsing.
  static void RegisterFieldAccel(void *sk, iAwsSource *source);
  static void RegisterFAActive(void *sk, iAwsSource *source);
  static void RegisterFAFPX(void *sk, iAwsSource *source);
  static void RegisterFAFPY(void *sk, iAwsSource *source);
  static void RegisterFAFPZ(void *sk, iAwsSource *source);
  static void RegisterFAFWeight(void *sk, iAwsSource *source);
  static void RegisterFALSX(void *sk, iAwsSource *source);
  static void RegisterFALSY(void *sk, iAwsSource *source);
  static void RegisterFALSZ(void *sk, iAwsSource *source);
  static void RegisterFALEX(void *sk, iAwsSource *source);
  static void RegisterFALEY(void *sk, iAwsSource *source);
  static void RegisterFALEZ(void *sk, iAwsSource *source);
  static void RegisterFALWeight(void *sk, iAwsSource *source);
  static void RegisterFABMX(void *sk, iAwsSource *source);
  static void RegisterFABMY(void *sk, iAwsSource *source);
  static void RegisterFABMZ(void *sk, iAwsSource *source);
  static void RegisterFABXX(void *sk, iAwsSource *source);
  static void RegisterFABXY(void *sk, iAwsSource *source);
  static void RegisterFABXZ(void *sk, iAwsSource *source);
  static void RegisterFABWeight(void *sk, iAwsSource *source);
  static void RegisterFASCX(void *sk, iAwsSource *source);
  static void RegisterFASCY(void *sk, iAwsSource *source);
  static void RegisterFASCZ(void *sk, iAwsSource *source);
  static void RegisterFASMin(void *sk, iAwsSource *source);
  static void RegisterFASMax(void *sk, iAwsSource *source);
  static void RegisterFASWeight(void *sk, iAwsSource *source);
  static void RegisterFACNOX(void *sk, iAwsSource *source);
  static void RegisterFACNOY(void *sk, iAwsSource *source);
  static void RegisterFACNOZ(void *sk, iAwsSource *source);
  static void RegisterFACNElev(void *sk, iAwsSource *source);
  static void RegisterFACNAzim(void *sk, iAwsSource *source);
  static void RegisterFACNAper(void *sk, iAwsSource *source);
  static void RegisterFACNMin(void *sk, iAwsSource *source);
  static void RegisterFACNMax(void *sk, iAwsSource *source);
  static void RegisterFACNWeight(void *sk, iAwsSource *source);
  static void RegisterFACYSX(void *sk, iAwsSource *source);
  static void RegisterFACYSY(void *sk, iAwsSource *source);
  static void RegisterFACYSZ(void *sk, iAwsSource *source);
  static void RegisterFACYEX(void *sk, iAwsSource *source);
  static void RegisterFACYEY(void *sk, iAwsSource *source);
  static void RegisterFACYEZ(void *sk, iAwsSource *source);
  static void RegisterFACYMin(void *sk, iAwsSource *source);
  static void RegisterFACYMax(void *sk, iAwsSource *source);
  static void RegisterFACYWeight(void *sk, iAwsSource *source);
  static void RegisterFASTCX(void *sk, iAwsSource *source);
  static void RegisterFASTCY(void *sk, iAwsSource *source);
  static void RegisterFASTCZ(void *sk, iAwsSource *source);
  static void RegisterFASTMin(void *sk, iAwsSource *source);
  static void RegisterFASTMax(void *sk, iAwsSource *source);
  static void RegisterFASTWeight(void *sk, iAwsSource *source);
  static void RegisterFACYTSX(void *sk, iAwsSource *source);
  static void RegisterFACYTSY(void *sk, iAwsSource *source);
  static void RegisterFACYTSZ(void *sk, iAwsSource *source);
  static void RegisterFACYTEX(void *sk, iAwsSource *source);
  static void RegisterFACYTEY(void *sk, iAwsSource *source);
  static void RegisterFACYTEZ(void *sk, iAwsSource *source);
  static void RegisterFACYTMin(void *sk, iAwsSource *source);
  static void RegisterFACYTMax(void *sk, iAwsSource *source);
  static void RegisterFACYTWeight(void *sk, iAwsSource *source);

  static void AwsSetFAActive(void *sk, iAwsSource *source);
  static void AwsSetFAFPositionX(void *sk, iAwsSource *source);
  static void AwsSetFAFPositionY(void *sk, iAwsSource *source);
  static void AwsSetFAFPositionZ(void *sk, iAwsSource *source);
  static void AwsSetFAFWeight(void *sk, iAwsSource *source);
  static void AwsSetFALStartX(void *sk, iAwsSource *source);
  static void AwsSetFALStartY(void *sk, iAwsSource *source);
  static void AwsSetFALStartZ(void *sk, iAwsSource *source);
  static void AwsSetFALEndX(void *sk, iAwsSource *source);
  static void AwsSetFALEndY(void *sk, iAwsSource *source);
  static void AwsSetFALEndZ(void *sk, iAwsSource *source);
  static void AwsSetFALWeight(void *sk, iAwsSource *source);
  static void AwsSetFABMinX(void *sk, iAwsSource *source);
  static void AwsSetFABMinY(void *sk, iAwsSource *source);
  static void AwsSetFABMinZ(void *sk, iAwsSource *source);
  static void AwsSetFABMaxX(void *sk, iAwsSource *source);
  static void AwsSetFABMaxY(void *sk, iAwsSource *source);
  static void AwsSetFABMaxZ(void *sk, iAwsSource *source);
  static void AwsSetFABWeight(void *sk, iAwsSource *source);
  static void AwsSetFASCenterX(void *sk, iAwsSource *source);
  static void AwsSetFASCenterY(void *sk, iAwsSource *source);
  static void AwsSetFASCenterZ(void *sk, iAwsSource *source);
  static void AwsSetFASMin(void *sk, iAwsSource *source);
  static void AwsSetFASMax(void *sk, iAwsSource *source);
  static void AwsSetFASWeight(void *sk, iAwsSource *source);
  static void AwsSetFACNOriginX(void *sk, iAwsSource *source);
  static void AwsSetFACNOriginY(void *sk, iAwsSource *source);
  static void AwsSetFACNOriginZ(void *sk, iAwsSource *source);
  static void AwsSetFACNElev(void *sk, iAwsSource *source);
  static void AwsSetFACNAzim(void *sk, iAwsSource *source);
  static void AwsSetFACNAper(void *sk, iAwsSource *source);
  static void AwsSetFACNMin(void *sk, iAwsSource *source);
  static void AwsSetFACNMax(void *sk, iAwsSource *source);
  static void AwsSetFACNWeight(void *sk, iAwsSource *source);
  static void AwsSetFACYStartX(void *sk, iAwsSource *source);
  static void AwsSetFACYStartY(void *sk, iAwsSource *source);
  static void AwsSetFACYStartZ(void *sk, iAwsSource *source);
  static void AwsSetFACYEndX(void *sk, iAwsSource *source);
  static void AwsSetFACYEndY(void *sk, iAwsSource *source);
  static void AwsSetFACYEndZ(void *sk, iAwsSource *source);
  static void AwsSetFACYMin(void *sk, iAwsSource *source);
  static void AwsSetFACYMax(void *sk, iAwsSource *source);
  static void AwsSetFACYWeight(void *sk, iAwsSource *source);
  static void AwsSetFASTCenterX(void *sk, iAwsSource *source);
  static void AwsSetFASTCenterY(void *sk, iAwsSource *source);
  static void AwsSetFASTCenterZ(void *sk, iAwsSource *source);
  static void AwsSetFASTMin(void *sk, iAwsSource *source);
  static void AwsSetFASTMax(void *sk, iAwsSource *source);
  static void AwsSetFASTWeight(void *sk, iAwsSource *source);
  static void AwsSetFACYTStartX(void *sk, iAwsSource *source);
  static void AwsSetFACYTStartY(void *sk, iAwsSource *source);
  static void AwsSetFACYTStartZ(void *sk, iAwsSource *source);
  static void AwsSetFACYTEndX(void *sk, iAwsSource *source);
  static void AwsSetFACYTEndY(void *sk, iAwsSource *source);
  static void AwsSetFACYTEndZ(void *sk, iAwsSource *source);
  static void AwsSetFACYTMin(void *sk, iAwsSource *source);
  static void AwsSetFACYTMax(void *sk, iAwsSource *source);
  static void AwsSetFACYTWeight(void *sk, iAwsSource *source);





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



  static void RegisterFSWindow(void *sk, iAwsSource *source);
  static void RegisterFSLabel(void *sk, iAwsSource *source);
  static void RegisterFSTextBox(void *sk, iAwsSource *source);
  static void RegisterFSScrollBar(void *sk, iAwsSource *source);
  static void AwsSetFSTextBox(void *sk, iAwsSource *source);
  static void AwsSetFSScrollBar(void *sk, iAwsSource *source);

  static void FreeScrollSetComponent(bool floatval,void *value_pointer,iAwsComponent *associated,bool *invalidate_pointer);

  /// Static callback to handle Load/Save option window registration during .def file parsing.
  static void RegisterLoadSave(void *sk, iAwsSource *source);
  /// Static callback to handle Aging Moments option window registration during .def file parsing.
  static void RegisterAgingMoments(void *sk, iAwsSource *source);


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

  bool InitialSpeedStateChanged();
  void ClearInitialSpeedStateChanged();
  Emitter3DState *GetInitialSpeedState();
  void SetInitialSpeedState(Emitter3DState *source);
  void UpdateInitialSpeedStateDisplay();

  bool InitialAccelerationStateChanged();
  void ClearInitialAccelerationStateChanged();
  Emitter3DState *GetInitialAccelerationState();
  void SetInitialAccelerationState(Emitter3DState *source);
  void UpdateInitialAccelerationStateDisplay();


  bool FieldSpeedStateChanged();
  void ClearFieldSpeedStateChanged();
  FieldState *GetFieldSpeedState();
  void SetFieldSpeedState(FieldState *source);
  void UpdateFieldSpeedStateDisplay();

  bool FieldAccelStateChanged();
  void ClearFieldAccelStateChanged();
  FieldState *GetFieldAccelState();
  void SetFieldAccelState(FieldState *source);
  void UpdateFieldAccelStateDisplay();

  bool AttractorStateChanged();
  void ClearAttractorStateChanged();
  AttractorState *GetAttractorState();
  void SetAttractorState(AttractorState *source);
  void UpdateAttractorStateDisplay();


  bool LoadSaveStateChanged() { return LoadSaveData.settings_changed; };
  void ClearLoadSaveStateChanged() { LoadSaveData.settings_changed=false; };
  LoadSaveFileState *GetLoadSaveState() { return &(LoadSaveData.state); } ;
//  void SetLoadSaveState(LoadSaveFileState *source);
//  void UpdateLoadSaveStateDisplay();


  void SetVFS(csRef<iVFS> newvfs);
  csRef<iVFS> GetVFS();

};

#endif // __AWS_SINK_TEST_H__


