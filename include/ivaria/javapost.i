/*
    Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef SWIGJAVA

/*
%typemap(in) jclass %{ $1 = $input; %}
%typemap(jni) jclass "jclass";
%typemap(jtype) jclass "Class";
%typemap(jstype) jclass "Class";
%typemap(javain) jclass "$javainput";
%typemap(javaout) jclass { return $jnicall; }

%typemap(out) jobject %{ $result = $1; %}
%typemap(jni) jobject "jobject";
%typemap(jtype) jobject "Object";
%typemap(jstype) jobject "Object";
%typemap(javain) jobject "$javainput";
%typemap(javaout) jobject { return $jnicall; }
*/

//#define SHOW(fmt, arg) printf("%d: %s=" fmt "\n", __LINE__, #arg, arg)

%typemap(in) (const char * iface, int iface_ver) (char className[1024])
{
    const char * s = jenv->GetStringUTFChars($input, 0);
    const char * dot = strrchr(s, '.');
    strcpy(className, "org/crystalspace3d/");
    strcat(className, dot?dot+1:s);
    $1 = className + sizeof("org/crystalspace3d/") - 1;
    jenv->ReleaseStringUTFChars($input, s);
    jclass cls = jenv->FindClass(className);
    jmethodID mid = jenv->GetStaticMethodID(cls, "scfGetVersion", "()I");
    $2 = jenv->CallStaticIntMethod(cls, mid);
}
%typemap(jni) (const char * iface, int iface_ver) "jstring"
%typemap(jtype) (const char * iface, int iface_ver) "String"
%typemap(jstype) (const char * iface, int iface_ver) "Class"
%typemap(javain) (const char * iface, int iface_ver) "$javainput.getName()"

#undef SCF_QUERY_INTERFACE
#undef SCF_QUERY_INTERFACE_SAFE
#undef CS_QUERY_REGISTRY
#undef CS_QUERY_REGISTRY_TAG_INTERFACE
#undef CS_QUERY_PLUGIN_CLASS
#undef CS_LOAD_PLUGIN
#undef CS_GET_CHILD_OBJECT
#undef CS_GET_NAMED_CHILD_OBJECT
#undef CS_GET_FIRST_NAMED_CHILD_OBJECT

csWrapPtr CS_QUERY_REGISTRY (iObjectRegistry *reg, const char *iface,
  int iface_ver)
{
  return _CS_QUERY_REGISTRY (reg, iface, iface_ver);
}
csWrapPtr CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
  const char *tag, const char *iface, int iface_ver)
{
  return _CS_QUERY_REGISTRY_TAG_INTERFACE (reg, tag, iface, iface_ver);
}
csWrapPtr SCF_QUERY_INTERFACE (iBase *obj, const char *iface, int iface_ver)
{
  return _SCF_QUERY_INTERFACE (obj, iface, iface_ver);
}
csWrapPtr SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface,
  int iface_ver)
{
  return _SCF_QUERY_INTERFACE (obj, iface, iface_ver);
}
csWrapPtr CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return _CS_QUERY_PLUGIN_CLASS (obj, id, iface, iface_ver);
}
csWrapPtr CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return _CS_LOAD_PLUGIN (obj, id, iface, iface_ver);
}
csWrapPtr CS_GET_CHILD_OBJECT (iObject *obj, const char *iface, int iface_ver)
{
  return _CS_GET_CHILD_OBJECT (obj, iface, iface_ver);
}
csWrapPtr CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return _CS_GET_NAMED_CHILD_OBJECT (obj, iface, iface_ver, name);
}
csWrapPtr CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return _CS_GET_FIRST_NAMED_CHILD_OBJECT (obj, iface, iface_ver, name);
}

#ifndef CS_MINI_SWIG
%{
    static JavaVM * _the_jvm = 0;
%}

%inline %{

	struct _csJEventHandler : public iEventHandler
	{
		SCF_DECLARE_IBASE;
		_csJEventHandler () : my_jobject(0)
		{
			SCF_CONSTRUCT_IBASE(0);
			IncRef();
		}
		virtual ~_csJEventHandler ()
        {
	    SCF_DESTRUCT_IBASE();
            DecRef();
            JNIEnv * env = 0;
            _the_jvm->AttachCurrentThread((void **)&env, NULL);
            env->DeleteGlobalRef(my_jobject);
        }
        static jobject _csJEventHandler_jobject;
        void _importJEventHandler ()
        {
            my_jobject = _csJEventHandler_jobject;
        }
		virtual bool HandleEvent (iEvent & event)
		{
            try
            {
                return _HandleEvent(event);
            }
            catch (...)
            {
                JNIEnv * env = 0;
                _the_jvm->AttachCurrentThread((void **)&env, NULL);
                env->ExceptionClear();
            }
            return false;
        }
        bool _HandleEvent (iEvent & event)
        {
            JNIEnv * env = 0;
            _the_jvm->AttachCurrentThread((void **)&env, NULL);
            jclass event_class = env->FindClass("org/crystalspace3d/iEvent");
            jclass handler_class = env->FindClass("org/crystalspace3d/csJEventHandler");
            jmethodID event_ctr_mid = env->GetMethodID(event_class, "<init>", "(JZ)V");
            jmethodID handle_event_mid = env->GetMethodID(handler_class, "HandleEvent", "(Lorg/crystalspace3d/iEvent;)Z");
            jlong cptr = 0;
            *(iEvent **)&cptr = &event; 
            jobject event_object = env->NewObject(event_class, event_ctr_mid, cptr, false);
            if (!event_object)
                return false;
            jboolean result = env->CallBooleanMethod(my_jobject, handle_event_mid, event_object);
            return result;
		}
	private:
		jobject my_jobject;
	};
%}
#endif // CS_MINI_SWIG

%inline %{
    iObjectRegistry * theObjectRegistry;

%}

#ifndef CS_MINI_SWIG
%{

	SCF_IMPLEMENT_IBASE(_csJEventHandler)
	SCF_IMPLEMENT_IBASE_END

    jobject _csJEventHandler::_csJEventHandler_jobject;

    extern "C" {
        JNIEXPORT void JNICALL Java_org_crystalspace3d_csJEventHandler__1exportJEventHandler
            (JNIEnv *, jclass, jobject);
    }
                                                                                                              
    JNIEXPORT void JNICALL Java_org_crystalspace3d_csJEventHandler__1exportJEventHandler
        (JNIEnv * env, jclass, jobject obj)
    {
        if (!_the_jvm)
            env->GetJavaVM(&_the_jvm);
        _csJEventHandler::_csJEventHandler_jobject = env->NewGlobalRef(obj);
    }

%}
#endif // CS_MINI_SWIG

/* Following doesn't work for unknown reason.
// Unresoled symbols when loaded by JVM.
%define MAKE_CONSTANT(type, name, value)
    type _csjConstant_ ## name() { return value; }
    %{ #define _csjConstant_ ## name() value %}
    %constant type mask = _csjConstant_ ## name();
%enddef

#undef CSMASK_Nothing
MAKE_CONSTANT(int, CSMASK_Nothing, (1 << csevNothing))
#undef CSMASK_FrameProcess
MAKE_CONSTANT(int, CSMASK_FrameProcess, (CSMASK_Nothing))
#undef CSMASK_Keyboard
MAKE_CONSTANT(int, CSMASK_Keyboard, (1 << csevKeyboard))
#undef CSMASK_MouseMove
MAKE_CONSTANT(int, CSMASK_MouseMove, (1 << csevMouseMove))
#undef CSMASK_MouseDown
MAKE_CONSTANT(int, CSMASK_MouseDown, (1 << csevMouseDown))
#undef CSMASK_MouseUp
MAKE_CONSTANT(int, CSMASK_MouseUp, (1 << csevMouseUp))
#undef CSMASK_MouseClick
MAKE_CONSTANT(int, CSMASK_MouseClick, (1 << csevMouseClick))
#undef CSMASK_MouseDoubleClick
MAKE_CONSTANT(int, CSMASK_MouseDoubleClick, (1 << csevMouseDoubleClick))
#undef CSMASK_JoystickMove
MAKE_CONSTANT(int, CSMASK_JoystickMove, (1 << csevJoystickMove))
#undef CSMASK_JoystickDown
MAKE_CONSTANT(int, CSMASK_JoystickDown, (1 << csevJoystickDown))
#undef CSMASK_JoystickUp
MAKE_CONSTANT(int, CSMASK_JoystickUp, (1 << csevJoystickUp))
#undef CSMASK_Command
MAKE_CONSTANT(int, CSMASK_Command, (1 << csevCommand))
#undef CSMASK_Broadcast
MAKE_CONSTANT(int, CSMASK_Broadcast, (1 << csevBroadcast))
#undef CSMASK_Mouse
MAKE_CONSTANT(int, CSMASK_Mouse, (CSMASK_MouseMove | CSMASK_MouseDown | CSMASK_MouseUp | CSMASK_MouseClick | CSMASK_MouseDoubleClick))
#undef CSMASK_Joystick
MAKE_CONSTANT(int, CSMASK_Joystick, (CSMASK_JoystickMove | CSMASK_JoystickDown | CSMASK_JoystickUp))
#undef CSMASK_Input
MAKE_CONSTANT(int, CSMASK_Input, (CSMASK_Keyboard | CSMASK_Mouse | CSMASK_Joystick))
*/

#undef CSMASK_Nothing
%constant int CSMASK_Nothing = 0x1;
#undef CSMASK_FrameProcess
%constant int CSMASK_FrameProcess = 0x1;
#undef CSMASK_Keyboard
%constant int CSMASK_Keyboard = 0x2;
#undef CSMASK_MouseMove
%constant int CSMASK_MouseMove = 0x4;
#undef CSMASK_MouseDown
%constant int CSMASK_MouseDown = 0x8;
#undef CSMASK_MouseUp
%constant int CSMASK_MouseUp = 0x10;
#undef CSMASK_MouseClick
%constant int CSMASK_MouseClick = 0x20;
#undef CSMASK_MouseDoubleClick
%constant int CSMASK_MouseDoubleClick = 0x40;
#undef CSMASK_JoystickMove
%constant int CSMASK_JoystickMove = 0x80;
#undef CSMASK_JoystickDown
%constant int CSMASK_JoystickDown = 0x100;
#undef CSMASK_JoystickUp
%constant int CSMASK_JoystickUp = 0x200;
#undef CSMASK_Command
%constant int CSMASK_Command = 0x400;
#undef CSMASK_Broadcast
%constant int CSMASK_Broadcast = 0x800;
#undef CSMASK_Mouse
%constant int CSMASK_Mouse = (CSMASK_MouseMove | CSMASK_MouseDown | CSMASK_MouseUp | CSMASK_MouseClick | CSMASK_MouseDoubleClick);
#undef CSMASK_Joystick
%constant int CSMASK_Joystick = (CSMASK_JoystickMove | CSMASK_JoystickDown | CSMASK_JoystickUp);
#undef CSMASK_Input
%constant int CSMASK_Input = (CSMASK_Keyboard | CSMASK_Mouse | CSMASK_Joystick);

#endif // SWIGJAVA

