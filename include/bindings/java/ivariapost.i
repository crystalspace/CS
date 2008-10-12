#ifdef SWIGJAVA

#ifndef CS_MINI_SWIG

%template(scfJConsoleExecCallback) scfImplementation1<csJExecCallback,iConsoleExecCallback>;
%inline %{
	struct csJExecCallback : public scfImplementation1<csJExecCallback,iConsoleExecCallback> {
		csJExecCallback (jobject obj) : scfImplementationType (this), myJobject(0), execute_mid(0) {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
#ifdef __cplusplus
				myJobject = jenv->NewGlobalRef(obj);
				jclass callback_class = jenv->GetObjectClass(myJobject);
				execute_mid = jenv->GetMethodID(callback_class, "Execute", "(Ljava/lang/String;)V");
#else
				myJobject = (*jenv)->NewGlobalRef(jenv,obj);
				jclass callback_class = (*jenv)->GetObjectClass(jenv,myJobject);
				execute_mid = (*jenv)->GetMethodID(jenv,callback_class, "Execute", "(Ljava/lang/String;)V");
#endif
			}
		}
		virtual ~csJExecCallback () {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
#ifdef __cplusplus
				jenv->DeleteGlobalRef(myJobject);
#else
				(*jenv)->DeleteGlobalRef(jenv,myJobject);
#endif
			}
		}
		virtual void Execute (const char* cmd) {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
				try {
#ifdef __cplusplus
					jenv->CallVoidMethod(
						myJobject, 
						execute_mid,
						jenv->NewStringUTF(cmd)
					);
#else
					(*jenv)->CallVoidMethod(jenv,
						myJobject, 
						execute_mid,
						(*jenv)->NewStringUTF(jenv,cmd)
					);
#endif
				}
				catch (...) {
#ifdef __cplusplus
					jenv->ExceptionClear();
#else
					(*jenv)->ExceptionClear(jenv);
#endif
				}
			}
		}
		private:
		jobject myJobject;
		jmethodID execute_mid;
	};
%}

#endif // CS_MINI_SWIG

#endif // SWIGJAVA
