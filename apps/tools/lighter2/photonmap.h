#ifndef __JENSENPHOTONMAP_H__
#define __JENSENPHOTONMAP_H__

namespace lighter
{
  struct Photon;
  struct NearestPhotons;

  class PhotonMap
  {
  public:
    PhotonMap( size_t max_phot );
    ~PhotonMap();

    void store(
      const float power[3],         // photon power
      const float pos[3],           // photon position
      const float dir[3] );         // photon direction

    void scale_photon_power(
      const float scale );          // 1/(number of emitted photons)

    void balance(void);             // balance the kd-tree (before use!)

    void irradiance_estimate(
      float irrad[3],               // returned irradiance
      const float pos[3],           // surface position
      const float normal[3],        // surface normal at pos
      const float max_dist,         // max distance to look for photons
      const int nphotons ) const;   // number of photons to use

    void locate_photons(
      NearestPhotons *const np,     // np is used to locate the photons
      const int index ) const;      // call with index = 1

    void photon_dir(
      float *dir,                    // direction of photon (returned)
      const Photon *p ) const;       // the photon

  private:

    bool expand();

    void balance_segment(
      Photon **pbal,
      Photon **porg,
      const int index,

      const int start,
      const int end );

    void median_split(
      Photon **p,
      const int start,
      const int end,
      const int median,
      const int axis );

    Photon *photons;

    int stored_photons;
    int half_stored_photons;
    int max_photons;
    int prev_scale;

    float bbox_min[3];     // use bbox_min;
    float bbox_max[3];     // use bbox_max;

    // We save some memory by making these static
    static bool directionTablesReady;
    static float costheta[256];
    static float sintheta[256];
    static float cosphi[256];
    static float sinphi[256];
  };
};

#endif // __JENSENPHOTONMAP_H__