#include "tag.hpp"

Media Media::fromQuery(QSqlQuery &q) {
	Media r;
	r.name = q.value(2).toString();
	r.url = q.value(3).toUrl();
	r.cache = q.value(4).toByteArray();
	return r;
}

Tag Tag::fromQuery(QSqlQuery &q) {
	Tag r;
	r.id = q.value(0).toInt();
	r.title = q.value(1).toString();
	r.altTitle = q.value(2).toString();
	r.key = q.value(3).toString();
	r.parts = q.value(4).toInt();
	r.notes = q.value(5).toString();
	r.arranger = q.value(6).toString();
	r.arranged = q.value(7).toString();
	r.sungBy = q.value(8).toString();
	r.quartet = q.value(9).toString();
	r.posted = QDate::fromJulianDay(q.value(10).toInt());
	r.collection = q.value(11).toString();
	r.rating = q.value(12).toFloat();
	r.ratingCount = q.value(13).toInt();
	r.downloaded = q.value(14).toInt();
	r.sheetmusic = q.value(15).toUrl();
	r.sheetMusicAlt = q.value(16).toUrl();
	r.bookmarked = q.value(17).toBool();

	const auto val = q.value(18).toInt();
	if (val) {
		r.visited = QDateTime::fromSecsSinceEpoch(val);
	}
	return r;
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

	setVisited();
}

void Tag::setVisited() {
	visited = QDateTime::currentDateTime();
	QSqlQuery q;
	q.prepare("UPDATE tags SET visited = ? WHERE id = ?");
	q.bindValue(0, static_cast<int>(visited.toSecsSinceEpoch()), QSql::ParamTypeFlag::In);
	updateSqliteById(q);
}

void Tag::setMedia(const QString &name, const QUrl &url, const QByteArray &data) {
	QSqlQuery q;
	q.prepare("INSERT INTO media VALUES (?, ?, ?, ?, ?)");
	int bindpos = 0;
	q.bindValue(bindpos++, QString("%1_%2").arg(this->id).arg(name));
	q.bindValue(bindpos++, this->id);
	q.bindValue(bindpos++, name);
	q.bindValue(bindpos++, url);
	q.bindValue(bindpos++, data, QSql::ParamTypeFlag::In | QSql::ParamTypeFlag::Binary);
	if (!q.exec()) {
		qWarning() << "Failed to attach media: " << q.lastError().text();
	}
	this->media.insert(name, {name, url, data});
}

void Tag::fetchMedia() {
	QSqlQuery q;
	q.prepare("SELECT * FROM media WHERE tag = " + QString::number(id));
	q.exec();
	while (q.next()) {
		auto m = Media::fromQuery(q);
		media.insert(m.name, m);
	}
}

void Tag::updateSqliteById(QSqlQuery &q) {
	q.bindValue(1, id);
	if (!q.exec()) {
		qWarning() << "Failed to update database: " << q.lastError().text();
	}
}
