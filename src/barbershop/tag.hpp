#pragma once

#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

using TagId = int;

class Tag {
	Q_GADGET
	Q_PROPERTY(int id MEMBER id CONSTANT)
	Q_PROPERTY(QString title MEMBER title CONSTANT)
	Q_PROPERTY(QUrl sheetmusicUrl MEMBER sheetMusicAlt CONSTANT)
	Q_PROPERTY(QUrl sheetmusiclocation MEMBER sheetmusiclocation CONSTANT)
public:
	TagId id = 0;
	QString title;
	QString altTitle;
	QString key;
	int parts = 0;
	QString notes;
	QString arranger;
	QString arranged;
	QString sungBy;
	QString sungYear;
	QString quartet;
	QString posted;
	QString collection;
	float rating = 0;
	int ratingCount = 0;
	int downloaded = 0;
	QUrl sheetmusic;
	QUrl sheetMusicAlt;

	QByteArray cachedsheetmusic;
	QUrl sheetmusiclocation;
};
