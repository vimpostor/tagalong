#include "android.hpp"

#include <qglobal.h>
#ifdef Q_OS_ANDROID

#include <QFile>
#include <QJniEnvironment>
#include <QStandardPaths>
#include <jni.h>

static void setDocument(JNIEnv *env, jobject, jstring doc) {
	QString url {env->GetStringUTFChars(doc, 0)};

	// for some reason the QML PDF viewer does not support content URIs
	QFile f {url};
	f.open(QFile::ReadOnly);
	const auto data = f.readAll();
	f.close();
	f.setFileName(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/blob");
	f.open(QFile::WriteOnly);
	f.write(data);
	f.close();

	Backend::get()->setDocumentSource(QUrl::fromLocalFile(f.fileName()));
	Backend::get()->setDocumentType("pdf");
}


static const JNINativeMethod methods[] = {
	{"setDocument", "(Ljava/lang/String;)V", (void *) setDocument},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *, void *) {
	QJniEnvironment env;
	env.registerNativeMethods("com/mggross/tagalong/QtIntentActivity", methods, std::size(methods));
	return JNI_VERSION_1_6;
}

#endif
