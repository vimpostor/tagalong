#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlDatabase>
#include <QtQml/qqmlregistration.h>
#include <quartz/macros.hpp>

#include "tag.hpp"

class Api : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(bool isSyncing MEMBER m_isSyncing NOTIFY syncingChanged)
	Q_PROPERTY(float syncProgress MEMBER m_syncProgress NOTIFY syncingChanged)
public:
	QML_CPP_SINGLETON(Api)

	void init();
	Q_INVOKABLE void requestTag(TagId id);
	void syncMetadata();
signals:
	void tagReady(Tag tag);
	void syncingChanged();
private:
	void parseTags(QNetworkReply *res);
	void initDb();

	bool m_isSyncing = false;
	float m_syncProgress = 0;
	QNetworkAccessManager manager;
	QSqlDatabase db;
};
