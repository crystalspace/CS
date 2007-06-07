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
#endif

