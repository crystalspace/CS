Crystal Space specific conventions for 3DS artists
==================================================

* Limitations

  Object names can't exceed 10 charactgers. For example "GazeboBars" 
  is not correct, shortening to to "GBars" will work.

  A 3DSMax object can have only 1 texture for the converter to work. 
  If you need more textures, split the objects in different objects.

* Handling Light
  
  The converter also supports lights using these rules:
  - Only Omni light are converted. Spotlights are not supported.
  - To have the same effect in CS and in 3DSMax, define your omni
    light with these parameters:
    Under "General Parameters" check "Diffuse" and "Specular".
    "Ambient Only" UNchecked.
    Under "Attenuation Parameters":
    In section "Near Attenuation" check "Use", "Start" = 0,
    "End" = the outer range of your light.
    In section "Far Attenuation" check "Use", 
      "Start" = "End" = the outer range of your light.
    For example: Near Start = 0 Near End = 5; Far Start = 5 Far End = 5;
	       all 3 last numbers must be equal.

* Objects
  
  Objects are read and have render priority set to 'object' and zbuffer
  settings set to ZUSE by default.

* Textures

  Object textures are read from the material's diffuse bitmap as set in
  the 3DS file.  The 3DS file will only specify the local file name.  The
  actual file itself must be named in all lower case, for example 
  "Gazebo_Bars.png" is not correct, "gazebo_bars.png" is correct.

  Objects' transparency information is read from the material applied to
  the object in the 3DS file. (Note: alpha & ztest must still be set,
  refer to below).

* CS Settings

  To provide specific CS settings, the only way found so far is to 
  encode them in the object name, which is already short.  These 
  settings are delimited by pipe chars and must appear at the start of the 
  object name.
  eg
  "|wu|stone"
  "|sfl|sunset"

  (If anyone has better way, please suggest)

  Object Priorities
  w - wall
  o - object
  s - sky
  a - alpha

  Z Buffer
  f - zfill
  u - zuse 
  t - ztest 
  n - znone

  Others
  l - no light.  This object will be lit using texture at full brightness

  Common combinations
  Prefix  Output attributes
  |wf|     Use for static obects. ( wall + zfill).
  |ou|     Use for all other objects. (object + zuse).
  |sf|     A sky object, priority 'sky' (zfill).
  |sfl|    A sky object, with "no light" set.

Refer to 5.9.9 Render Priorities and Objects in Sectors for details.

