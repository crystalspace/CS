#ifndef __CS_DECAL_TEMPLATE_H__
#define __CS_DECAL_TEMPLATE_H__

#include <igeom/decal.h>
#include <csutil/scf_implementation.h>
#include <csgeom/vector2.h>

#define CS_DECAL_DEFAULT_TIME_TO_LIVE       -1.0f
#define CS_DECAL_DEFAULT_RENDER_PRIORITY    0
#define CS_DECAL_DEFAULT_NORMAL_THRESHOLD   0.01f
#define CS_DECAL_DEFAULT_OFFSET             0.05f
#define CS_DECAL_DEFAULT_NEAR_FAR_DIST      1.5f
#define CS_DECAL_DEFAULT_CLIP_NEAR_FAR      false

class csDecalTemplate : public scfImplementation1<csDecalTemplate,
                                                 iDecalTemplate>
{
private:
  float                 timeToLive;
  iMaterialWrapper*     material;
  long                  renderPriority;
  csZBufMode            zBufMode;
  float                 polygonNormalThreshold;
  float                 decalOffset;
  bool                  hasNearFarClipping;
  float                 nearFarClippingDist;
  csVector2             minTexCoord;
  csVector2             maxTexCoord;
  uint			mixMode;

public:

  csDecalTemplate();
  csDecalTemplate(iBase* parent);
  virtual ~csDecalTemplate();

  /**
   * Retrieves the time the decal will have to live in seconds before it is 
   * killed.
   *  \return The time to live in seconds.
   */
  virtual float GetTimeToLive() const;

  /**
   * Retrieves the material wrapper to use for this decal.
   *  \return the material wrapper.
   */
  virtual iMaterialWrapper* GetMaterialWrapper();

  /**
   * Retrieves the rendering priority for this decal
   *  \return the rendering priority.
   */
  virtual long GetRenderPriority() const;

  /** 
   * Retrieves the z-buffer mode for this decal.
   *  \return the z-buffer mode.
   */
  virtual csZBufMode GetZBufMode() const;

  /** 
   * Retrieves the polygon normal threshold for this decal.  
   *
   * Values close to 1 will exclude polygons that don't match the decal's 
   * normal closely, and values closer to 0 will be more accepting and allow
   * polygons with a very different normal from the decal's normal.
   *
   * Values between -1 and 0 are acceptable, but will allow polygons that
   * are facing in the opposite direction from the decal to be included.
   *
   *  \return the polygon threshold.
   */
  virtual float GetPolygonNormalThreshold() const;

  /**
   * A decal will be offset a bit from the geometry it wraps around in order
   * to avoid z-buffer fighting issues.
   *
   * The greater this offset is, the less chance there is of z fighting, but
   * if this is too high then the decal will appear to be floating.
   *
   *  \return the decal offset.
   */
  virtual float GetDecalOffset() const;

  /**
   * Determines whether the decal will be clipped against a near and far
   * plane.
   * \return True if near-far clipping is enabled.
   */
  virtual bool HasNearFarClipping() const;

  /**
   * If near-far clipping is enabled, this determines the distance between
   * the near and far plane.
   * \return The distance between the near and far plane.
   */
  virtual float GetNearFarClippingDist() const;

  /**
   * The min tex coord is the uv coordinate of the top-left corner of the
   * decal.
   *  \return The min tex coordinate.
   */
  virtual const csVector2 & GetMinTexCoord() const;
  
  /**
   * The max tex coord is the uv coordinate of the bottom-right corner of the
   * decal.
   *  \return The max tex coordinate.
   */
  virtual const csVector2 & GetMaxTexCoord() const;

  /**
   * The mixmode of the decal.
   *  \return The mixmode.
   */
  virtual const uint GetMixMode() const;
  
  /**
   * Sets the time the decal will have to live in seconds before it is 
   * killed.
   *  \param timeToLive	The time to live in seconds.
   */
  virtual void SetTimeToLive(float timeToLive);
  
  /**
   * Sets the material wrapper to use for this decal.
   *  \param material	The material wrapper of the decal.
   */
  virtual void SetMaterialWrapper(iMaterialWrapper* material);

  /**
   * Sets the rendering priority for this decal
   *  \param renderPriority	The render priority of the decal.
   */
  virtual void SetRenderPriority(long renderPriority);

  /** 
   * Sets the z-buffer mode for this decal.
   *  \param mode	The z-buffer mode for the decal.
   */
  virtual void SetZBufMode(csZBufMode mode);

  /** 
   * Sets the polygon normal threshold for this decal.  
   *
   * Values close to 1 will exclude polygons that don't match the decal's 
   * normal closely, and values closer to 0 will be more accepting and allow
   * polygons with a very different normal from the decal's normal.
   *
   * Values between -1 and 0 are acceptable, but will allow polygons that
   * are facing in the opposite direction from the decal to be included.
   *
   *  \param polygonNormalThreshold	The polygon normal threshold.
   */
  virtual void SetPolygonNormalThreshold(float polygonNormalThreshold);
  
  /**
   * A decal will be offset a bit from the geometry it wraps around in order
   * to avoid z-buffer fighting issues.
   *
   * The greater this offset is, the less chance there is of z fighting, but
   * if this is too high then the decal will appear to be floating.
   *
   *  \param decalOffset	The distance between decal and the geometry.
   */
  virtual void SetDecalOffset(float decalOffset);

  /**
   * Determines whether the decal will be clipped against a near and far
   * plane.
   *  \param enabled	True if near-far clipping is enabled.
   */
  virtual void SetNearFarClipping(bool enabled);

  /**
   * If near-far clipping is enabled, this determines the distance between
   * the near and far plane.
   *  \param dist	The distance between the near and far plane.
   */
  virtual void SetNearFarClippingDist(float dist);

  /**
   * The tex coords are the uv coordinate of the top-left and bottom-right 
   * corner of the decal.
   *  \param min	The top-left corner of the decal.
   *  \param max	The bottom-right corner of the decal.
   */
  virtual void SetTexCoords(const csVector2 & min, const csVector2 & max);

  /**
   * The mixmode of the decal.
   *  \param mixMode	 The mixmode of the decal.
   */
  virtual void SetMixMode(uint mixMode);
};

#endif // __CS_DECAL_TEMPLATE_H__
