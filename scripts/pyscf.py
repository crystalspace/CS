import scf

scf.scfInitialize()

class csIniFile:
  def __init__(self, file=""):
    self.ptr=scf.new_csIniFile()
    if(file):
      self.Load(file)
  def Load(self, file):
    scf.csIniFile_Load(self.ptr, file)