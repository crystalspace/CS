// To define a static draw_scanline function in the 'Scan' class just
// define the following variables:
//	SCANFUNC	: name of the draw_scanline function (required)
//	SCANEND		: optional additional code after the function (mostly for Z fill)
//	SCANLOOP	: code to draw one segment of 16 (or less) pixels
//	SCANMAP		: define if the function uses the lighted textures
//
// The SCANLOOP code can expect the following variables to be set:
//	uu, vv, izz	: u, v, and 1/z components for start of segment (izz = 8:24)
//	duu, dvv, dzz	: deltas per pixel for above values (dzz = 8:24)
//	_dest		: pointer to screen for start of segment
//	_destend	: pointer to screen for end of segment
//	z_buffer	: pointer to z_buffer for start of segment
//	lastZbuf	: pointer to z_buffer for end of complete line
//	srcTex		: pointer to texture map
//	shifter		: shift value for vv (only if SCANMAP is defined)
//	shifter_h	: shift value for vv (only if SCANMAP is NOT defined)
//	ander_w		: and value for uu (only if SCANMAP is NOT defined)
//	ander_h		: and value for vv (only if SCANMAP is NOT defined)
// The SCANLOOP code needs to update these variables as follows:
//	uu, vv, izz	: u, v, and 1/z components just after this segment (izz = 8:24)
//	_dest		: _destend+1
//	z_buffer	: pointer to z_buffer after end of segment
// izz and z_buffer need only be updated if the loop code is interested
// in using the Z buffer. If the code does only a Z fill, 'izz' need only
// be updated in the Z fill loop.

void Scan::SCANFUNC (int xx, unsigned char* d,
		     unsigned long* z_buf,
		     float inv_z, float u_div_z, float v_div_z)
{
  (void)z_buf;
  if (xx <= 0) return;

  float u1, v1, z, z1, u, v;
  int uu1, vv1, duu, dvv, uu, vv;
  long izz, dzz;

  z = 1 / inv_z;
  u = u_div_z * z;
  v = v_div_z * z;
  uu = QInt16 (u);
  vv = QInt16 (v);
  izz = QInt24 (inv_z);

  // Some optimizations for processors with many registers (PowerPC, 680x0).
  // Get global variables into registers.
# ifdef SCANMAP
  unsigned long shifter = shf_u;
  unsigned char *srcTex = tmap2;
# else
  unsigned long shifter_h = shf_h;
  unsigned long ander_w = and_w;
  unsigned long ander_h = and_h;
  unsigned char *srcTex = tmap;
# endif /*SCANMAP*/
  unsigned long *z_buffer = z_buf; (void)z_buffer;
  unsigned long *lastZbuf = z_buffer + xx-1; (void)lastZbuf;
  unsigned char *_destend;
  unsigned char *_dest = d;

  if (xx >= INTERPOL_STEP)
    inv_z += dM;
  else
    inv_z += M * xx;
  z1 = 1 / inv_z;

  dzz = QInt24 (M);

# if STUPID_TEST && SCANMAP
  //if (uu < BOUNDARIES_OF_STUPID_TEST_U_MIN) uu = BOUNDARIES_OF_STUPID_TEST_U_MIN; else if (uu > Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) uu = Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
  //if (vv < BOUNDARIES_OF_STUPID_TEST_V_MIN) vv = BOUNDARIES_OF_STUPID_TEST_V_MIN; else if (vv > Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) vv = Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
  if (uu < 0) uu = 0; else if (uu > tw2fp) uu = tw2fp;
  if (vv < 0) vv = 0; else if (vv > th2fp) vv = th2fp;
# endif

  do
  {
    if (xx < INTERPOL_STEP)
    {
      u_div_z += J1 * xx;
      v_div_z += K1 * xx;

      _destend = _dest + xx - 1;

      u1 = u_div_z * z1;
      v1 = v_div_z * z1;
      uu1 = QInt16 (u1);
      vv1 = QInt16 (v1);
#     if STUPID_TEST && SCANMAP
      //if (uu1 < BOUNDARIES_OF_STUPID_TEST_U_MIN) uu1 = BOUNDARIES_OF_STUPID_TEST_U_MIN; else if (uu1 > Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) uu1 = Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
      //if (vv1 < BOUNDARIES_OF_STUPID_TEST_V_MIN) vv1 = BOUNDARIES_OF_STUPID_TEST_V_MIN; else if (vv1 > Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) vv1 = Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
      if (uu1 < 0) uu1 = 0; else if (uu1 > tw2fp) uu1 = tw2fp;
      if (vv1 < 0) vv1 = 0; else if (vv1 > th2fp) vv1 = th2fp;
#     endif

      duu = (uu1 - uu) / xx;
      dvv = (vv1 - vv) / xx;

      xx = 0;
    }
    else
    {
      u_div_z += dJ1;
      v_div_z += dK1;

      _destend = _dest + INTERPOL_STEP - 1;
      xx -= INTERPOL_STEP;

      u1 = u_div_z * z1;
      v1 = v_div_z * z1;
      uu1 = QInt16 (u1);
      vv1 = QInt16 (v1);
#     if STUPID_TEST && SCANMAP
      //if (uu1 < BOUNDARIES_OF_STUPID_TEST_U_MIN) uu1 = BOUNDARIES_OF_STUPID_TEST_U_MIN; else if (uu1 > Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) uu1 = Scan::tw2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
      //if (vv1 < BOUNDARIES_OF_STUPID_TEST_V_MIN) vv1 = BOUNDARIES_OF_STUPID_TEST_V_MIN; else if (vv1 > Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX) vv1 = Scan::th2fp+BOUNDARIES_OF_STUPID_TEST_U_MAX;
      if (uu1 < 0) uu1 = 0; else if (uu1 > tw2fp) uu1 = tw2fp;
      if (vv1 < 0) vv1 = 0; else if (vv1 > th2fp) vv1 = th2fp;
#     endif

      if (xx >= INTERPOL_STEP)
        inv_z += dM;
      else
        inv_z += M * xx;

      duu = (uu1 - uu) >> INTERPOL_SHFT;
      dvv = (vv1 - vv) >> INTERPOL_SHFT;
    }

    // The following divide should overlap with integer instructions
    // below and GCC shouldn't pickup the result until the
    // end of the loop. Thus, the divide is free and the
    // routine is almost as fast as an affine mapper on 
    // Pentium class processors.
    // Note that this divide should normally be inside one of the above
    // branches (it is not needed when doing the last part of the scanline)
    // but GCC is not able to correctly optimize this if we put it inside
    // the 'if' statement. That's the reason that it is put here.
    // In case of the last part of the scanline it will just calculate
    // a result that will not be needed afterwards (but it doesn't
    // hurt either).
    z1 = 1 / inv_z;

    SCANLOOP;

#   if STUPID_TEST && SCANMAP
    // This almost definitively removes texture overflows
    uu = uu1;
    vv = vv1;
#   endif
  }
  while (xx);

# ifdef SCANEND
  SCANEND;
# endif /*SCANEND*/
}
