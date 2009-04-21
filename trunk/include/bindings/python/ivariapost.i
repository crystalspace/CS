#if defined(SWIGPYTHON)
%extend iCollideSystem
{
  %rename(_GetCollisionPairs) GetCollisionPairs;

  %pythoncode %{
    def GetCollisionPairs (self):
      num = self.GetCollisionPairCount()
      pairs = []
      for i in range(num):
        pairs.append(self.GetCollisionPairByIndex(i))
      return pairs
  %}
}

%pythoncode %{
  def CS_REQUEST_REPORTERLISTENER ():
    return core.CS_REQUEST_PLUGIN("crystalspace.utilities.stdrep",
      iStandardReporterListener)
  def CS_REQUEST_CONSOLEOUT ():
    return core.CS_REQUEST_PLUGIN("crystalspace.console.output.standard",
      iConsoleOutput)
%}

#endif

