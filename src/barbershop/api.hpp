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
	Q_PROPERTY(int bytesReceived MEMBER m_bytesReceived NOTIFY syncingChanged)
	Q_PROPERTY(bool downloadActive MEMBER m_downloadActive NOTIFY downloadActiveChanged)
public:
	QML_CPP_SINGLETON(Api)

	void init();
	Q_INVOKABLE void reset(bool hard = false);
	Q_INVOKABLE void requestTag(TagId id);
	void downloadMedia(const Media &media, Tag &tag);
	void handleMediaDownload(QNetworkReply *reply, const Media &media, Tag &tag);
	void writeMedia(const Media &tag);
	void handlePlayRequest(const Media *media);
	std::vector<Tag> complete(QString query);
	void syncMetadata();
signals:
	void syncingChanged();
	void downloadActiveChanged();
private:
	std::optional<Tag> tagFromId(TagId id) const;
	void parseTags();
	void handleTagsFinished();
	void initDb();
	void downloadAndView(const Media &media);

	bool m_isSyncing = false;
	float m_syncProgress = 0;
	int m_bytesReceived = 0;
	bool m_downloadActive = false;
	QNetworkAccessManager manager;
	QSqlDatabase db;

	// parsing
	QNetworkReply *reply = nullptr;
	QXmlStreamReader xml;
	std::vector<Tag> pendingtags;
	std::vector<std::pair<TagId, Media>> pendingmedia;
	Tag currenttag;
	bool invideo = false;
	int tagsAvailable = 0;
	int currentIndex = 0;
	QString currentName = "";
};
