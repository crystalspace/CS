#ifndef GL_STATES_H_INCLUDED
#define GL_STATES_H_INCLUDED


struct gl_states_ext
{
  gl_states_ext()
  {
    glenabled_blend = false;
    glenabled_cull_face = true;
    glenabled_alpha_test = false;
    glenabled_depth_test = true;
    glenabled_stencil_test = false;
    glenabled_texture1d = false;
    glenabled_texture2d = false;
    glenabled_texture3d = false;

    glcsenabled_vertex_array = false;;
    glcsenabled_color_array = false;
    glcsenabled_texture_coord_array = false;
  };

  bool glenabled_blend;
  bool glenabled_cull_face;
  bool glenabled_alpha_test;
  bool glenabled_depth_test;
  bool glenabled_stencil_test;
  bool glenabled_texture1d;
  bool glenabled_texture2d;
  bool glenabled_texture3d;

  // These are the states of various
  // glEnableClientState/glDisableClientState elements.
  bool glcsenabled_vertex_array;
  bool glcsenabled_color_array;
  bool glcsenabled_texture_coord_array;
};

#endif
