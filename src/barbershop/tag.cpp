#include "tag.hpp"

Tag::Tag(QSqlQuery &q) {
	id = q.value(0).toInt();
	title = q.value(1).toString();
	altTitle = q.value(2).toString();
	key = q.value(3).toString();
	parts = q.value(4).toInt();
	notes = q.value(5).toString();
	arranger = q.value(6).toString();
	arranged = q.value(7).toString();
	sungBy = q.value(8).toString();
	quartet = q.value(9).toString();
	posted = QDate::fromJulianDay(q.value(10).toInt());
	collection = q.value(11).toString();
	rating = q.value(12).toFloat();
	ratingCount = q.value(13).toInt();
	downloaded = q.value(14).toInt();
	sheetmusic = q.value(15).toUrl();
	sheetMusicAlt = q.value(16).toUrl();
	bookmarked = q.value(17).toBool();
	cachedsheetmusic = q.value(18).toByteArray();
}

void Tag::setBookmarked(bool b) {
	if (bookmarked == b) {
		return;
	}
	bookmarked = b;
	QSqlQuery q;
	q.prepare("UPDATE tags SET bookmarked = ? WHERE id = ?");
	q.bindValue(0, static_cast<int>(b), QSql::ParamTypeFlag::In);
	updateSqliteById(q);
}

void Tag::setCachedSheetMusic(const QByteArray &b) {
	cachedsheetmusic = b;
	QSqlQuery q;
	q.prepare("UPDATE tags SET cachedsheetmusic = ? WHERE id = ?");
	q.bindValue(0, cachedsheetmusic, QSql::ParamTypeFlag::In | QSql::ParamTypeFlag::Binary);
	updateSqliteById(q);
}

bool Tag::isVisited() const {
	return !cachedsheetmusic.isNull();
}

void Tag::updateSqliteById(QSqlQuery &q) {
	q.bindValue(1, id);
	if (!q.exec()) {
		qWarning() << "Failed to update database";
	}
}
