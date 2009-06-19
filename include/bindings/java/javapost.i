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

#ifndef CS_MINI_SWIG


%template(scfJEventHandler) scfImplementation1<_csJEventHandler, iEventHandler >;
%inline %{
	struct _csJEventHandler : public scfImplementation1<_csJEventHandler, iEventHandler> 
	{
		_csJEventHandler () : scfImplementationType (this), my_jobject(0), event_class(0), event_ctr_mid(0), handle_event_mid(0) 
		{
            		JNIEnv * env = jni_env_getter->getJNIEnv();
			if (env != 0) 
			{
				event_class = (jclass)env->NewGlobalRef(env->FindClass("org/crystalspace3d/iEvent"));
				jclass handler_class = env->FindClass("org/crystalspace3d/csJEventHandler");
				event_ctr_mid = env->GetMethodID(event_class, "<init>", "(JZ)V");
				handle_event_mid = env->GetMethodID(handler_class, "HandleEvent", "(Lorg/crystalspace3d/iEvent;)Z");
			}

		}
		virtual ~_csJEventHandler () 
		{
            		JNIEnv * env = jni_env_getter->getJNIEnv();
			if (env != 0) 
			{
	            		env->DeleteGlobalRef(my_jobject);
	            		env->DeleteGlobalRef(event_class);
			}
			delete jni_env_getter;
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
				JNIEnv * env = jni_env_getter->getJNIEnv();
                		if (env != 0) {
					env->ExceptionClear();
				}
            		}
            		return false;
        	}
        	bool _HandleEvent (iEvent & event) 
		{
            		JNIEnv * env = jni_env_getter->getJNIEnv();
			if (env != 0) 
			{
				jlong cptr = 0;
				*(iEvent **)&cptr = &event; 
				jobject event_object = env->NewObject(event_class, event_ctr_mid, cptr, false);
				if (!event_object)
					return false;
				jboolean result = env->CallBooleanMethod(my_jobject, handle_event_mid, event_object);
				return result;
			}
			return false;
		}
    		CS_EVENTHANDLER_NAMES("crystalspace.java")
    		CS_EVENTHANDLER_NIL_CONSTRAINTS
		private:
		jobject my_jobject;
		jclass event_class;
		jmethodID event_ctr_mid;
		jmethodID handle_event_mid;
	};
%}
#endif // CS_MINI_SWIG

%inline %{
    iObjectRegistry * theObjectRegistry;

%}

#ifndef CS_MINI_SWIG
%{

    jobject _csJEventHandler::_csJEventHandler_jobject;

    extern "C" {
        JNIEXPORT void JNICALL Java_org_crystalspace3d_csJEventHandler__1exportJEventHandler
            (JNIEnv *, jclass, jobject);
    }
                                                                                                              
    JNIEXPORT void JNICALL Java_org_crystalspace3d_csJEventHandler__1exportJEventHandler
        (JNIEnv * env, jclass, jobject obj) 
    {
        _csJEventHandler::_csJEventHandler_jobject = env->NewGlobalRef(obj);
    }

%}
#endif // CS_MINI_SWIG

#endif // SWIGJAVA
