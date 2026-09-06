#include "backend.hpp"


void Backend::setDocumentSource(QUrl src) {
	documentSource = src;
	emit documentSourceChanged();
}

void Backend::setDocumentType(QString type) {
	documentType = type;
	emit documentTypeChanged();
}

void Backend::setAudioSource(QUrl src) {
	audioSource = src;
	emit audioSourceChanged();
}

void Backend::notifySnackbar(QString s) {
	qDebug() << s;
	snackbar = s;
	emit snackbarChanged();
}
