#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlDatabase>
#include <QXmlStreamReader>
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
	void downloadSheetmusic(Tag &tag);
	void handleSheetmusic(QNetworkReply *reply, Tag tag);
	void writeSheetmusic(Tag &tag);
	std::vector<Tag> complete(QString query);
	void syncMetadata();
signals:
	void syncingChanged();
private:
	Tag tagFromQuery(QSqlQuery &q) const;
	std::optional<Tag> tagFromId(TagId id) const;
	void parseTags();
	void initDb();

	bool m_isSyncing = false;
	float m_syncProgress = 0;
	QNetworkAccessManager manager;
	QSqlDatabase db;

	// parsing
	QNetworkReply *reply = nullptr;
	QXmlStreamReader xml;
	std::vector<Tag> pendingtags;
	Tag currenttag;
	bool invideo = false;
	int tagsAvailable = 0;
	int currentIndex = 0;
	QString currentName = "";
};
