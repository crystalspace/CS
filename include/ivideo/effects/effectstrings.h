/*
    Copyright (C) 2002 by Anders Stenberg

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * Do not add double include protection here! 
 * CS\include\ivideo\effects\efstring.h  is the only file that should
 * include this file and it MUST be able to include it twice or these
 * strings will all be empty!
 */



/////////////////////////////////////////
// General strings
/////////////////////////////////////////
REGISTER_STRING( enabled, "enabled" )
REGISTER_STRING( disabled, "disabled" )
REGISTER_STRING( fog, "fog" )
REGISTER_STRING( mesh, "mesh" )
REGISTER_STRING( lightmap, "lightmap" )

/////////////////////////////////////////
// Misc drawing strings
/////////////////////////////////////////
REGISTER_STRING( shade_mode, "shade mode" )
REGISTER_STRING( flat, "flat" )
REGISTER_STRING( smooth, "smooth" )

/////////////////////////////////////////
// Blend mode strings
/////////////////////////////////////////
REGISTER_STRING( blending, "blending" )
REGISTER_STRING( source_blend_mode, "source blend mode" )
REGISTER_STRING( destination_blend_mode, "destination blend mode" )
REGISTER_STRING( source_color, "source color" )
REGISTER_STRING( inverted_source_color, "inverted source color" )
REGISTER_STRING( destination_color, "destination color" )
REGISTER_STRING( inverted_destination_color, "inverted_destination color" )
REGISTER_STRING( source_alpha, "source alpha" )
REGISTER_STRING( inverted_source_alpha, "inverted source alpha" )
REGISTER_STRING( destination_alpha, "destination alpha" )
REGISTER_STRING( inverted_destination_alpha, "inverted destination alpha" )
REGISTER_STRING( saturated_source_alpha, "saturated source alpha" )
REGISTER_STRING( one, "one" )
REGISTER_STRING( zero, "zero" )

/////////////////////////////////////////
// Texture setup
/////////////////////////////////////////
REGISTER_STRING( vertex_color_source, "vertex color source" )
REGISTER_STRING( constant_color_source, "constant color source" )
REGISTER_STRING( texture_source, "texture source" )
REGISTER_STRING( texture_coordinate_source, "texture coordinate source" )

/////////////////////////////////////////
// Multi texture setup
/////////////////////////////////////////

REGISTER_STRING( color_source_1, "color source 1" )
REGISTER_STRING( color_source_modifier_1, "color source modifier 1" )
REGISTER_STRING( color_source_2, "color source 2" )
REGISTER_STRING( color_source_modifier_2, "color source modifier 2" )
REGISTER_STRING( color_source_3, "color source 3" )
REGISTER_STRING( color_source_modifier_3, "color source modifier 3" )
REGISTER_STRING( alpha_source_1, "alpha source 1" )
REGISTER_STRING( alpha_source_modifier_1, "alpha source modifier 1" )
REGISTER_STRING( alpha_source_2, "alpha source 2" )
REGISTER_STRING( alpha_source_modifier_2, "alpha source modifier 2" )
REGISTER_STRING( alpha_source_3, "alpha source 2" )
REGISTER_STRING( alpha_source_modifier_3, "alpha source modifier 3" )
REGISTER_STRING( vertex_color, "vertex color" )
REGISTER_STRING( texture_color, "texture color" )
REGISTER_STRING( constant_color, "constant color" )
REGISTER_STRING( previous_layer_color, "previous layer color" )
REGISTER_STRING( vertex_alpha, "vertex alpha" )
REGISTER_STRING( texture_alpha, "texture alpha" )
REGISTER_STRING( constant_alpha, "constant alpha" )
REGISTER_STRING( previous_layer_alpha, "previous layer alpha" )

REGISTER_STRING( color_operation, "color operation" )
REGISTER_STRING( alpha_operation, "alpha operation" )
REGISTER_STRING( use_source_1, "use source 1" )
REGISTER_STRING( multiply, "multiply" )
REGISTER_STRING( add, "add" )
REGISTER_STRING( add_signed, "add signed" )
REGISTER_STRING( subtract, "subtract" )
REGISTER_STRING( interpolate, "interpolate" )
REGISTER_STRING( dot_product, "dot product" )
REGISTER_STRING( dot_product_to_alpha, "dot product to alpha" )

REGISTER_STRING( scale_rgb, "scale rgb")
REGISTER_STRING( scale_alpha, "scale alpha")

/////////////////////////////////////////
// NVVertex program support
/////////////////////////////////////////

REGISTER_STRING( nvvertex_program_gl, "nvvertex program gl")


