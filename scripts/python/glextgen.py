# Utility to create Crystal Space's glextmanager.h compatibility header.
# The header is constructed from the database metaglext.xml, and the templates
# in the glext subdirectory.

from xml.dom import minidom
from string import *

xmldoc = minidom.parse ("metaglext.xml")

writtenFuncs = list()
writtenFuncTypes = list()

templates = {}

def getTemplate (template):
  if template in templates:
    return templates[template];
  else:
    templateFile = file ("glext/" + template, "r");
    lines = templateFile.readlines();
    templates[template] = lines;
    return lines;   

def concat(list):
  result = "";
  for word in list:
    result += word;
  return result;

def getTemplateK (template, kind):
  return getTemplate (template + "_" + kind);

def extensionUrl (ext):
  urlPrefix = "http://oss.sgi.com/projects/ogl-sample/registry/";
  if ext.startswith("GL_"):
    ext = ext[3:];
    extsplit = ext.split ("_", 1);
    return urlPrefix + extsplit[0] + "/" + extsplit[1] + ".txt";
  elif ext.startswith("WGL_"):
    extsplit = ext.split ("_", 2);
    return urlPrefix + extsplit[1] + "/" + extsplit[0].lower() + "_" + extsplit[2] + ".txt";

def interpolate (strings, values):
  resStrings = list();
  for string in strings:
    words = string.split ("%");
    result = "";
    i = 0;
    if len (words) >= 2:
      while i < (len (words) - 1):
	result = result + words[i];
	key = words[i + 1];
	if key == "":
	  result = result + "%"
	else:
	  ksplit = key.split (":");
	  value = values [ksplit[0].lower()];
	  if key.islower():
	    value = value.lower();
	  if key.isupper():
	    value = value.upper();
	  if len (ksplit) > 1:
	    if ksplit[1] == "url":
	      value = extensionUrl (value);
	    else:
	      width = int (ksplit[1]);
	      if width > 0:
		value = value.ljust (width);
	      if width < 0:
		value = value.rjust (-width);
	  result = result + value;  
	i = i + 2;
    while i < len (words):
      result = result + words[i];
      i = i + 1;
    resStrings.append (result);
  return resStrings;

def getConstant (const):
    values = { "name" : const.getAttribute ("name"), 
      "value" : const.getAttribute ("value") };
    return interpolate (getTemplate ("constant"), values);

def getFunctionType (func):
    values = { "name" : func.getAttribute ("name"), 
      "return" : func.getAttribute ("return") };
    if (func.getAttribute ("pointer") == "true"):
      values["pointer"] = "*";
    else:
      values["pointer"] = "";
    prevarg = 0;
    arguments = "";
    for arg in func.getElementsByTagName ("arg"):
      arguments = arguments + join (getArgument (arg, prevarg), "");
      prevarg = arg;
    values["arglist"] = arguments;
    return interpolate (getTemplate ("func_type"), values);

def getArgument (arg, prevarg):
    values = { "name" : arg.getAttribute ("name") };
    if prevarg != 0:
      values["comma"] = ", ";
    else:
      values["comma"] = "";
    if (arg.getAttribute ("const") == "true"):
      values["const"] = "const ";
    else:
      values["const"] = "";
    values["type"] = arg.getAttribute("type");
    if (arg.getAttribute ("pointer") == "true"):
      values["pointer"] = "*";
    else:
      values["pointer"] = "";
    return interpolate (getTemplate ("func_arg"), values);


def getDefinitions (ext):
    name = ext.getAttribute ("name");
    #type = ((name.split ("_"))[0]).lower();
    type = ext.getAttribute ("type")
    values = { "name" : name, "tokens" : "", "functiontypes" : "" };
    for const in ext.getElementsByTagName ("token"):
      values["tokens"] = values["tokens"] + join (getConstant (const), "");
    for func in ext.getElementsByTagName ("function"):
      if func.getAttribute("name") not in writtenFuncTypes:
	values["functiontypes"] = values["functiontypes"] + \
	  join (getFunctionType (func), "");
	writtenFuncTypes.append (func.getAttribute("name"))
    return interpolate (getTemplateK ("defs", type), values);

def getExtensions (extensions):
    tflags = list();
    dflags = list();
    initflags = list();
    defs = list();
    funcs = list();
    initext = list();
    for ext in extensions:
      name = ext.getAttribute ("name");
      #type = ((name.split ("_"))[0]).lower();
      type = ext.getAttribute ("type")
      print name + "...";
      values = { "name" : name };
      print " flags...";
      tflags += interpolate (getTemplate ("ext_tested"), values);
      dflags += interpolate (getTemplateK ("ext_flag", type), values);
      initflags += interpolate (getTemplate ("ext_init"), values);
      print " tokens...";
      defs += getDefinitions (ext);
      print " funcs...";
      funcs += getFunctions (ext);
      initext += getInitExtensions (ext);
    values = {
      "definitions" : concat (defs),
      "initflags" : concat (initflags),
      "functions" : concat (funcs),
      "extflagsdetected" : concat (dflags),
      "extflagstested" : concat (tflags),
      "initextensions" : concat (initext)
    };
    print "assembling...";
    return interpolate (getTemplate ("headerfiletemplate"), values);

def getFunctions (ext):
  funcs = list();
  name = ext.getAttribute ("name");
  #type = ((name.split ("_"))[0]).lower();
  type = ext.getAttribute ("type")
  for func in ext.getElementsByTagName ("function"):
    values = { "name" : func.getAttribute("name") };
    if func.getAttribute("name") not in writtenFuncs:
      funcs = funcs + interpolate (getTemplate ("func"), values);
      writtenFuncs.append (func.getAttribute("name"))
  values = {"name" : name, "functions" : join (funcs, "") };
  return interpolate (getTemplateK ("funcs", type), values);

def getInitExtensions (ext):
    cfgprefix = "Video.OpenGL.UseExtension.";
    name = ext.getAttribute ("name")
    values = { 
      "name" : name, 
      "namelen" : str (len (name)),
      "cfgprefix" : cfgprefix, 
      "cfglen" : str (len (cfgprefix)),
      "functionsinit" : "",
      "depcheck" : "" };
    #type = ((name.split ("_"))[0]).lower();
    type = ext.getAttribute ("type")
    ettype = type;
    specials = ( 
      "WGL_ARB_extensions_string"
      );
    if name in specials:
      ettype = "special";
    values["extcheck"] = join (interpolate (getTemplateK ("extcheck", 
      ettype), values), "");
    for func in ext.getElementsByTagName ("function"):
      values["functionsinit"] += join (getFunctionInit (func), "");
    for dep in ext.getElementsByTagName ("depends"):
      values["depcheck"] += join (getDependency (dep), "");
    return interpolate (getTemplateK ("initext", 
      type), values);

def getFunctionInit (func):
    name = func.getAttribute ("name")
    values = { 
      "name" : name};
    return interpolate (getTemplate ("funcinit"), values);

def getDependency (dep):
    ext = dep.getAttribute ("ext")
    values = { 
      "ext" : ext};
    return interpolate (getTemplate ("depends"), values);

stuff = join (getExtensions (xmldoc.getElementsByTagName ("extension")),
  "");
output = file ("glextmanager.h", "w")
output.write ("/**\n")
output.write (" * WARNING - This file is automatically generated\n")
output.write (" */\n\n")
output.write (stuff)
output.close ()
