from xml.dom import minidom

xmldoc = minidom.parse ("metaglext.xml")
header = file ("glextheader", "r")
footer = file ("glextfooter", "r")
output = file ("glextmanager.h", "w")

writtenFuncs = list()
writtenFuncTypes = list()

def writeConstant (const):
    output.write ("#ifndef " + const.getAttribute ("name") + "\n")
    output.write (("#define " + const.getAttribute ("name") + \
                   "\t" + const.getAttribute ("value") + "\n").expandtabs (70))
    output.write ("#endif\n")

def writeFunctionType (func):
    output.write ("typedef " + func.getAttribute ("return"))
    if (func.getAttribute ("pointer") == "TRUE"):
        output.write ("*")
    output.write (" (csAPIENTRY* cs" + \
                  func.getAttribute ("name").upper () + ") (")
    prevarg = 0;
    for arg in func.getElementsByTagName ("ARG"):
        writeArgument (arg, prevarg)
        prevarg = arg;
    output.write (");\n")

def writeArgument (arg, prevarg):
    if prevarg != 0:
        output.write (", ")
    if arg.getAttribute("type").endswith("GL",0,2):
        output.write ("GL" + arg.getAttribute("type").lstrip ("GL").lower ());
    else:
        output.write (arg.getAttribute("type"));
    if (arg.getAttribute ("pointer") == "TRUE"):
        output.write ("*")
    output.write (" " + arg.getAttribute("name"));


def writeDefinitions (extensions):
    for ext in extensions:
        output.write ("// " + ext.getAttribute ("name") + "\n")
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#ifdef _WIN32\n")
        for const in ext.getElementsByTagName ("TOKEN"):
            writeConstant (const)
        for func in ext.getElementsByTagName ("FUNCTION"):
            if func.getAttribute("name") not in writtenFuncTypes:
            	writeFunctionType (func)
            	writtenFuncTypes.append (func.getAttribute("name"))
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#endif\n")
        output.write ("\n")
    output.write ("\n\n\n")

def writeExtensions (extensions):
    for ext in extensions:
        output.write ("  bool CS_" + ext.getAttribute ("name") + ";\n")
    output.write ("\n")
    output.write ("\
private:\n");
    for ext in extensions:
        output.write ("  bool tested_CS_" + ext.getAttribute ("name") + ";\n")
    output.write ("\
public:\n\
  csGLExtensionManager (): object_reg(NULL)\n\
  {\n");
    for ext in extensions:
        output.write ("    CS_" + ext.getAttribute ("name") + " = false;\n")
        output.write ("    tested_CS_" + ext.getAttribute ("name") + " = false;\n")
    output.write ("\
  }\n");

def writeFunctions (extensions):
    for ext in extensions:
        output.write ("  // " + ext.getAttribute ("name") + "\n")
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#ifdef _WIN32\n")
        for func in ext.getElementsByTagName ("FUNCTION"):
            if func.getAttribute("name") not in writtenFuncs:
            	writeFunction (func)
            	writtenFuncs.append (func.getAttribute("name"))
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#endif\n")
        output.write ("\n")

def writeFunction (func):
    output.write ("  #ifndef " + func.getAttribute ("name").upper () + "_DECL\n"\
                  "  #define " + func.getAttribute ("name").upper () + "_DECL\n") 
    output.write ("  cs" + func.getAttribute ("name").upper () + \
                  " " + func.getAttribute ("name") + ";\n")
    output.write ("  #endif\n\n")

def writeInitExtensions (extensions):
    cfgprefix = "Video.OpenGL.UseExtension.";
    for ext in extensions:
        name = ext.getAttribute ("name")
        if name.endswith("WGL",0,3):
	  output.write ("#ifdef _WIN32\n")
	  output.write ("  void Init" + name + " (HDC hDC)\n  {\n")
	else:
	  output.write ("  void Init" + name + " ()\n  {\n")
        output.write ("    if (tested_CS_" + name + ") return;\n");
        output.write ("    tested_CS_" + name + " = true;\n")
        output.write ("    const char* ext = \"" + name +"\";\n");
        output.write ("    char cfgkey[" + str (len (cfgprefix) + len (name) + 1) + "];\n");
        output.write ("    sprintf (cfgkey, \"" + cfgprefix + "%s\", ext);\n\n");
	# sligthly different ext checking for Win32
        if ((name.startswith("GL_version_")) or 
	  (name == "WGL_ARB_extensions_string") or 
	  (name == "WGL_EXT_extensions_string")):
          output.write ("    CS_" + ext.getAttribute ("name") + " = true;\n")
	else:
	  if name.endswith("WGL",0,3):
	    output.write ("    if (!tested_CS_WGL_ARB_extensions_string) " +
	      "InitWGL_ARB_extensions_string (hDC);\n");
	    output.write ("    const char* extensions;\n");
	    output.write ("    if (CS_WGL_ARB_extensions_string)\n");
	    output.write ("    {\n");
	    output.write ("      extensions = wglGetExtensionsStringARB (hDC);\n");
	    output.write ("    }\n");
	    output.write ("    else\n");
	    output.write ("    {\n");
	    output.write ("      extensions = " +
	      " (const char*)glGetString (GL_EXTENSIONS);\n");
	    output.write ("    }\n");
	  else:
	    output.write ("    const char* extensions = (const char*)glGetString (GL_EXTENSIONS);\n");
	  output.write ("    CS_" + ext.getAttribute ("name") + " = (strstr (extensions, ext) != NULL);\n")
	output.write ("\n");
        output.write ("    bool allclear, funcTest;\n")
        if name.endswith("WGL",0,3):
	  output.write ("    if (CS_" + name + ")\n");
	else:
	  output.write ("    if (gl && CS_" + name + ")\n");
        output.write ("\
    {\n");
        output.write ("      allclear = true;\n")
	for func in ext.getElementsByTagName ("FUNCTION"):
	    writeFunctionInit (func)
        output.write ("      if (CS_" + name + " = allclear)\n")
        output.write ("      {\n");
        output.write ("\
        CS_" + name + " &= config->GetBool (cfgkey, true);\n\
        if (CS_" + name + ")\n\
        {\n\
          Report (\"GL Extension '%s' found and used.\", ext);\n\
        }\n\
        else\n\
        {\n\
          Report (\"GL Extension '%s' found, but not used.\", ext);\n\
        }\n\
      }\n\
      else\n\
      {\n\
        Report (\"GL Extension '%s' failed to initialize.\", ext);\n\
      }\n\
    }\n\
    else\n\
    {\n\
      Report (\"GL Extension '%s' not found.\", ext);\n\
    }\n");
        output.write ("  }\n\n");
        if name.endswith("WGL",0,3):
            output.write ("#endif\n")

def writeFunctionInit (func):
    name = func.getAttribute ("name")
    output.write ("      funcTest = ((" + name + " = (" + \
                  "cs" + name.upper () + \
                  ") gl->GetProcAddress (\"" + \
                  name + "\")) != NULL);\n")
    output.write ("      if (!funcTest && config->GetBool \
(\"Video.OpenGL.ReportMissingEntries\", REPORT_MISSING_ENTRIES))\n");                
    output.write ("        Report (\"Failed to retrieve %s\", \"" + name + "\");\n");
    output.write ("      allclear &= funcTest;\n");

output.write (header.read ())
writeDefinitions (xmldoc.getElementsByTagName ("EXTENSION"))
output.write ("\
#ifdef CS_DEBUG\n\
#  define REPORT_MISSING_ENTRIES true\n\
#else\n\
#  define REPORT_MISSING_ENTRIES false\n\
#endif\n\
\n\
struct csGLExtensionManager\n\
{\n\
private:\n\
  iObjectRegistry* object_reg;\n\
  csConfigAccess config;\n\
  iOpenGLInterface* gl;\n\
\n\
  void Report (const char* msg, ...)\n\
  {\n\
    va_list arg;\n\
    va_start (arg, msg);\n\
    csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));\n\
    if (rep)\n\
      rep->ReportV (CS_REPORTER_SEVERITY_NOTIFY,\n\
         \"crystalspace.canvas.opengl.extmgr\", msg, arg);\n\
    else\n\
    {\n\
      csPrintfV (msg, arg);\n\
      csPrintf (\"\\n\");\n\
    }\n\
    va_end (arg);\n\
  }\n\
\n\
public:\n\
  void Initialize (iObjectRegistry* object_reg, iGraphics2D* g2d)\n\
  {\n\
    csGLExtensionManager::object_reg = object_reg;\n\
    gl = csRef<iOpenGLInterface>\n\
      (SCF_QUERY_INTERFACE (g2d, iOpenGLInterface));\n\
    // Low priority so canvas/renderer cfgs may override the settings\n\
    config.AddConfig (object_reg, \"/config/glext.cfg\", true,\n\
      iConfigManager::ConfigPriorityPlugin - 1);\n\
  }\n\
  \n\
  void Open ()\n\
  {\n\
  }\n\
  \n\
  void Close ()\n\
  {\n\
  }\n\
  \n\
");
writeExtensions (xmldoc.getElementsByTagName ("EXTENSION"))
writeFunctions (xmldoc.getElementsByTagName ("EXTENSION"))
writeInitExtensions (xmldoc.getElementsByTagName ("EXTENSION"))
output.write ("};\n\
#undef REPORT_MISSING_ENTRIES\n\n");
output.write (footer.read ())
output.close ()
