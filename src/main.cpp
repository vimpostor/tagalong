#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "backend.hpp"

int main(int argc, char *argv[]) {
	QCoreApplication::setOrganizationName("tag-along");
	QCoreApplication::setApplicationName("tag-along");
	QGuiApplication app(argc, argv);

	QGuiApplication::setWindowIcon(QIcon::fromTheme("tagalong", QIcon(":/tagalong")));

	QQmlApplicationEngine engine;

	engine.loadFromModule("Backend", "Main");
	if (engine.rootObjects().isEmpty()) {
		return EXIT_FAILURE;
	}

	Api::get()->init();
	return app.exec();
}
