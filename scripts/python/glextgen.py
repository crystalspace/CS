from xml.dom import minidom

xmldoc = minidom.parse ("metaglext.xml")
header = file ("glextheader", "r")
footer = file ("glextfooter", "r")
output = file ("glextmanager.h", "w")

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
            writeFunctionType (func)
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#endif\n")
        output.write ("\n")
    output.write ("\n\n\n")

def writeExtensions (extensions):
    for ext in extensions:
        output.write ("  bool CS_" + ext.getAttribute ("name") + ";\n")
    output.write ("\n")

def writeFunctions (extensions):
    for ext in extensions:
        output.write ("  // " + ext.getAttribute ("name") + "\n")
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#ifdef _WIN32\n")
        for func in ext.getElementsByTagName ("FUNCTION"):
            writeFunction (func)
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
    output.write ("    bool allclear;\n")
    output.write ("    const char* extensions = (const char*)glGetString (GL_EXTENSIONS);\n");
    for ext in extensions:
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#ifdef _WIN32\n")
        output.write ("    // " + ext.getAttribute ("name") + "\n")
        output.write ("    CS_" + ext.getAttribute ("name") + " = (strstr (extensions, \"" + \
                      ext.getAttribute ("name") + "\") != NULL);\n")
        output.write ("    if (CS_" + ext.getAttribute ("name") + ")\n    {\n")
        output.write ("      allclear = true;\n")
        for func in ext.getElementsByTagName ("FUNCTION"):
            writeFunctionInit (func)
        output.write ("      if (CS_" + ext.getAttribute ("name") + " = allclear)\n")
        output.write ("        printf (\"GL Extension '" + \
                      ext.getAttribute ("name") + "' found and used.\\n\");\n    }\n")
        if ext.getAttribute("name").endswith("WGL",0,3):
            output.write ("#endif\n")
        output.write ("\n")

def writeFunctionInit (func):
    output.write ("      if (!(" + func.getAttribute ("name") + " = (" + \
                  "cs" + func.getAttribute ("name").upper () + \
                  ") gl->GetProcAddress (\"" + \
                  func.getAttribute ("name") + "\"))) allclear = false;\n")


output.write (header.read ())
writeDefinitions (xmldoc.getElementsByTagName ("EXTENSION"))
output.write ("class csGLExtensionManager\n\
{\n\
public:\n");
writeExtensions (xmldoc.getElementsByTagName ("EXTENSION"))
writeFunctions (xmldoc.getElementsByTagName ("EXTENSION"))
output.write ("  void InitExtensions (iOpenGLInterface* gl)\n  {\n")
writeInitExtensions (xmldoc.getElementsByTagName ("EXTENSION"))
output.write ("  }\n};\n\n");
output.write (footer.read ())
output.close ()
