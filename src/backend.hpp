#pragma once

#include "barbershop/api.hpp"

class Backend : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(QUrl documentSource MEMBER document CONSTANT)
	Q_PROPERTY(QString snackbar MEMBER snackbar NOTIFY snackbarChanged)
public:
	QML_CPP_SINGLETON(Backend)
	QUrl document;
	void notifySnackbar(QString s);
signals:
	void snackbarChanged();
private:
	QString snackbar;
};
