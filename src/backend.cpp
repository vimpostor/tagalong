#include "backend.hpp"

void Backend::notifySnackbar(QString s) {
	qDebug() << s;
	snackbar = s;
	emit snackbarChanged();
}
