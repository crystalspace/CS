#ifdef SWIGJAVA

#ifndef CS_MINI_SWIG

%template(scfJConsoleExecCallback) scfImplementation1<csJExecCallback,iConsoleExecCallback>;
%inline %{
	struct csJExecCallback : public scfImplementation1<csJExecCallback,iConsoleExecCallback> {
		csJExecCallback (jobject obj) : scfImplementationType (this), myJobject(0), execute_mid(0) {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
				myJobject = jenv->NewGlobalRef(obj);
				jclass callback_class = jenv->GetObjectClass(myJobject);
				execute_mid = jenv->GetMethodID(callback_class, "Execute", "(Ljava/lang/String;)V");
			}
		}
		virtual ~csJExecCallback () {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
				jenv->DeleteGlobalRef(myJobject);
			}
		}
		virtual void Execute (const char* cmd) {
			JNIEnv * jenv = getJNIEnv();
			if ( jenv != 0 ) {
				try {
					jenv->CallVoidMethod(
						myJobject, 
						execute_mid,
						jenv->NewStringUTF(cmd)
					);
				}
				catch (...) {
					jenv->ExceptionClear();
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
