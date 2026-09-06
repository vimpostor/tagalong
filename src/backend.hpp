#pragma once

#include "barbershop/api.hpp"

class Backend : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(QUrl documentSource MEMBER documentSource NOTIFY documentSourceChanged)
	Q_PROPERTY(QString documentType MEMBER documentType NOTIFY documentTypeChanged)
	Q_PROPERTY(QUrl audioSource MEMBER audioSource NOTIFY audioSourceChanged)
	Q_PROPERTY(QString snackbar MEMBER snackbar NOTIFY snackbarChanged)
	Q_PROPERTY(bool overlayVisible MEMBER overlayVisible NOTIFY overlayVisibleChanged)
public:
	QML_CPP_SINGLETON(Backend)

	void setDocumentSource(QUrl src);
	void setDocumentType(QString type);
	void setAudioSource(QUrl src);
	void notifySnackbar(QString s);
signals:
	void documentSourceChanged();
	void documentTypeChanged();
	void audioSourceChanged();
	void snackbarChanged();
	void overlayVisibleChanged();
private:
	QUrl documentSource;
	QString documentType;
	QUrl audioSource;
	QString snackbar;
	bool overlayVisible = false;
};
