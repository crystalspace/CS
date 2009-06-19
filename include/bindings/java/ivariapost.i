%template(scfJConsoleExecCallback) scfImplementation1<csJExecCallback,iConsoleExecCallback>;
%inline %{
	struct csJExecCallback : public scfImplementation1<csJExecCallback,iConsoleExecCallback> {
		csJExecCallback (jobject obj) : scfImplementationType (this), myJobject(0), execute_mid(0) {
			JNIEnv * m_env = jni_env_getter->getJNIEnv();
			if ( m_env != 0 ) {
				myJobject = m_env->NewGlobalRef(obj);
				jclass callback_class = m_env->GetObjectClass(myJobject);
				execute_mid = m_env->GetMethodID(callback_class, "Execute", "(Ljava/lang/String;)V");
			}
		}
		virtual ~csJExecCallback () {
			JNIEnv * m_env = jni_env_getter->getJNIEnv();
			if ( m_env != 0 ) {
				m_env->DeleteGlobalRef(myJobject);
			}
		}
		virtual void Execute (const char* cmd) {
			JNIEnv * m_env = jni_env_getter->getJNIEnv();
			if ( m_env != 0 ) {
				try {
					m_env->CallVoidMethod(
						myJobject, 
						execute_mid,
						m_env->NewStringUTF(cmd)
					);
				}
				catch (...) {
					m_env->ExceptionClear();
				}
			}
		}
		private:
		jobject myJobject;
		jmethodID execute_mid;
	};
%}
