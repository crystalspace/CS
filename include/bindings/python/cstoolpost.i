#if defined(SWIGPYTHON)
%extend iPen {
        void _Rotate(float a)
        { self->Rotate(a); }
    %pythoncode %{
    def Rotate(self,a):
         return _cspace.iPen__Rotate(a)
    %}
}
#endif

