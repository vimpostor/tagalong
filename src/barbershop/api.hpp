#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQml/qqmlregistration.h>
#include <quartz/macros.hpp>

#include "tag.hpp"

class Api : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	QML_CPP_SINGLETON(Api)

	void init();
	Q_INVOKABLE void requestTag(TagId id);
signals:
	void tagReady(Tag tag);
private:
	void parseTags(QNetworkReply *res);

	QNetworkAccessManager manager;
};
