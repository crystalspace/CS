/*
 * Compute an inverse colormap efficiently.
 */

#ifndef inv_cmap_h
#define inv_cmap_h

void
inv_cmap(int colors, unsigned char *colormap[3], 
	 int rbits, int gbits, int bbits, 
	 unsigned long *dist_buf, 
	 unsigned char *rgbmap);

#endif
