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
            		JNIEnv * jenv = getJNIEnv();
			if (jenv != 0) 
			{
				event_class = (jclass)jenv->NewGlobalRef(jenv->FindClass("org/crystalspace3d/iEvent"));
				jclass handler_class = jenv->FindClass("org/crystalspace3d/csJEventHandler");
				event_ctr_mid = jenv->GetMethodID(event_class, "<init>", "(JZ)V");
				handle_event_mid = jenv->GetMethodID(handler_class, "HandleEvent", "(Lorg/crystalspace3d/iEvent;)Z");
			}

		}
		virtual ~_csJEventHandler () 
		{
            		JNIEnv * jenv = getJNIEnv();
			if (jenv != 0) 
			{
	            		jenv->DeleteGlobalRef(my_jobject);
	            		jenv->DeleteGlobalRef(event_class);
			}
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
				JNIEnv * jenv = getJNIEnv();
                		if (jenv != 0) {
					jenv->ExceptionClear();
				}
            		}
            		return false;
        	}
        	bool _HandleEvent (iEvent & event) 
		{
            		JNIEnv * jenv = getJNIEnv();
			if (jenv != 0) 
			{
				jlong cptr = 0;
				*(iEvent **)&cptr = &event; 
				jobject event_object = jenv->NewObject(event_class, event_ctr_mid, cptr, false);
				if (!event_object)
					return false;
				jboolean result = jenv->CallBooleanMethod(my_jobject, handle_event_mid, event_object);
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
        (JNIEnv * jenv, jclass, jobject obj) 
    {
        _csJEventHandler::_csJEventHandler_jobject = jenv->NewGlobalRef(obj);
    }

%}
#endif // CS_MINI_SWIG

#endif // SWIGJAVA
