This plugin is highly EXPERIMENTAL!

Use at your own risk! (It shouldn't break anything, just cause
frustration if you're trying to use something that doesn't quite work yet).

This plugin is designed to provide Pixel Shader v1.x functionality using
a similar syntax to the pixel shader instructions. Most existing shader
programs could probably be used by encasing them in XML code. However, a
few instructions are not supported (such as PS 1.2 and 1.3 instructions)
and there are a few restrictions.

Cards with nVidia chipsets use the NV_register_combiners and
NV_texture_shader extensions. These are only able to provide functionality
equivalent to PS 1.1. Any attempt to use PS 1.4 on these cards will fail
(and will fall back to the next shader technique) PS 1.2 and 1.3 programs
may work, but only if they do not use texture addressing instructions
specific to these versions. Most GeForce3 and later cards should work
fine. Note that GeForce 1, GeForce 2 and TNT2 cards support
NV_register_combiners, but these will not work yet. (I am planning to add
a specual subset of instructions that would work on these cards, which
would provide access to basic features such as a secondary color and
arithmetic operations)

Cards with the ATI Radeon 8500 or later chipsets use the
ATI_fragment_shader extension. This is fully equivalent to PS 1.4 and
almost all PS 1.4 programs should work normally. However, no PS 1.1,
1.2 or 1.3 programs will run using this extension, as 1.4 is very
different from the way the older versions work. Dual phase programs
are supported and should work normally.
