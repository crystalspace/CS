import scf

class csIniFile:
  def __init__(self, file=""):
    self.ptr=scf.new_csIniFile()
    if(file):
      self.Load(file)
  def Load(self, file):
    scf.csIniFile_Load(self.ptr, file)

def scfInit(IniFile=None):
  if(not IniFile):
    ptr="NULL"
  else:
    ptr=IniFile.ptr;  
  scf.scfInitialize()

def scfCreate(name, interface, version):
  return scf.scfCreateInstance(name, interface, version)

class csPython:
  def __init__(self):
    self.ptr=scfCreate('crystalspace.script.python', 'iScript', 1)
  def Init():
    

print 'Starting SCF...',
scfInit()
print 'Done'

a=csPython()

print a