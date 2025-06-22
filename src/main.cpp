#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "backend.hpp"

#include <qglobal.h>
#ifdef Q_OS_ANDROID
#include "android.hpp"
#endif

int main(int argc, char *argv[]) {
	QCoreApplication::setOrganizationName("tagalong");
	QCoreApplication::setApplicationName("tagalong");
	QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
	QObject::connect(&app, &QGuiApplication::applicationStateChanged, [&]() {
		const auto *iface = app.nativeInterface<QNativeInterface::QAndroidApplication>();
		if (iface->isActivityContext()) {
			iface->context().callMethod<void>("dispatchIntent");
		}
	});
#endif

	Api::get()->init();
	if (argc > 1) {
		Backend::get()->setDocumentSource(QUrl::fromLocalFile(argv[1]));
		Backend::get()->setDocumentType("pdf");
	}

	QGuiApplication::setWindowIcon(QIcon::fromTheme("tagalong", QIcon(":/tagalong")));

	QQmlApplicationEngine engine;

	engine.loadFromModule("Backend", "Main");
	if (engine.rootObjects().isEmpty()) {
		return EXIT_FAILURE;
	}

	return app.exec();
}
