Export script from Blender to Crystal Space
 -> written by Norman Krämer
 -> modify for the new Blender API by Yvon TANGUY (yvontang@caramail.com)
 -> work on Blender 2.23.

How it work?

To use correctly this script, you have to name your objects
to determine the type of your object.

you have to put this before the object name:
for a sector: "se_" + sector_name
for a mesh factory: "mf_" + sector_name + mesh_factory_name
for the light: "li_" + light_name
for the start position: "sa_" + what you want

When you have finish your work, go to the text editor
within Blender (Alt+F11), load the plugin by clicking on
the "-" button, then choose "OPEN NEW", and select this plugin.
Now, you just have to press the keys Alt+P, and it's done!

You can change the file export name by modifying the "fileExport"
value at the begining of the script.

To see if all has been correctly done, just look at the Blender
console messages. Tou have to see which entity the script found,
and error message if any.

Yvon TANGUY
05-16-2002