#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "backend.hpp"

int main(int argc, char *argv[]) {
	QCoreApplication::setOrganizationName("tagalong");
	QCoreApplication::setApplicationName("tagalong");
	QGuiApplication app(argc, argv);

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
