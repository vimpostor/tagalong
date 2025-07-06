#pragma once

#include <QDate>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

using TagId = int;

class Tag {
	Q_GADGET
	Q_PROPERTY(int id MEMBER id CONSTANT)
	Q_PROPERTY(QString title MEMBER title CONSTANT)
	Q_PROPERTY(QString altTitle MEMBER altTitle CONSTANT)
	Q_PROPERTY(QString key MEMBER key CONSTANT)
	Q_PROPERTY(int parts MEMBER parts CONSTANT)
	Q_PROPERTY(QString notes MEMBER notes CONSTANT)
	Q_PROPERTY(QString arranger MEMBER arranger CONSTANT)
	Q_PROPERTY(QString arranged MEMBER arranged CONSTANT)
	Q_PROPERTY(QString sungBy MEMBER sungBy CONSTANT)
	Q_PROPERTY(QString quartet MEMBER quartet CONSTANT)
	Q_PROPERTY(QDate posted MEMBER posted CONSTANT)
	Q_PROPERTY(QString collection MEMBER collection CONSTANT)
	Q_PROPERTY(float rating MEMBER rating CONSTANT)
	Q_PROPERTY(int ratingCount MEMBER ratingCount CONSTANT)
	Q_PROPERTY(int downloaded MEMBER downloaded CONSTANT)
	Q_PROPERTY(QUrl sheetmusic MEMBER sheetmusic CONSTANT)
	Q_PROPERTY(QUrl sheetMusicAlt MEMBER sheetMusicAlt CONSTANT)

	Q_PROPERTY(bool bookmarked MEMBER bookmarked CONSTANT)
	Q_PROPERTY(QDateTime visited MEMBER visited CONSTANT)

	Q_PROPERTY(QUrl sheetmusiclocation MEMBER sheetmusiclocation CONSTANT)
public:
	Tag() {};
	explicit Tag(QSqlQuery &q);

	TagId id = 0;
	QString title;
	QString altTitle;
	QString key;
	int parts = 0;
	QString notes;
	QString arranger;
	QString arranged;
	QString sungBy;
	QString quartet;
	QDate posted;
	QString collection;
	float rating = 0;
	int ratingCount = 0;
	int downloaded = 0;
	QUrl sheetmusic;
	QUrl sheetMusicAlt;

	bool bookmarked = false;
	QDateTime visited;

	QUrl sheetmusiclocation;

	QByteArray cachedsheetmusic;

	Q_INVOKABLE void setBookmarked(bool b);
	void setVisited();
	void setCachedSheetMusic(const QByteArray &b);
private:
	void updateSqliteById(QSqlQuery &q);
};
