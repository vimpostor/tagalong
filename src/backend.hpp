#pragma once

#include "barbershop/api.hpp"

class Backend : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(QUrl documentSource MEMBER documentSource NOTIFY documentSourceChanged)
	Q_PROPERTY(QString documentType MEMBER documentType NOTIFY documentTypeChanged)
	Q_PROPERTY(QString snackbar MEMBER snackbar NOTIFY snackbarChanged)
public:
	QML_CPP_SINGLETON(Backend)

	void setDocumentSource(QUrl src);
	void setDocumentType(QString type);
	void notifySnackbar(QString s);
signals:
	void snackbarChanged();
	void documentSourceChanged();
	void documentTypeChanged();
private:
	QUrl documentSource;
	QString documentType;
	QString snackbar;
};
