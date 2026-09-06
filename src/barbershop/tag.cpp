#include "tag.hpp"

Media Media::fromQuery(QSqlQuery &q) {
	Media r;
	int bindpos = 2;
	r.name = q.value(bindpos++).toString();
	r.url = q.value(bindpos++).toUrl();
	r.cache = q.value(bindpos++).toByteArray();
	return r;
}

Tag Tag::fromQuery(QSqlQuery &q) {
	Tag r;
	int bindpos = 0;
	r.id = q.value(bindpos++).toInt();
	r.title = q.value(bindpos++).toString();
	r.altTitle = q.value(bindpos++).toString();
	r.key = q.value(bindpos++).toString();
	r.parts = q.value(bindpos++).toInt();
	r.notes = q.value(bindpos++).toString();
	r.arranger = q.value(bindpos++).toString();
	r.arranged = q.value(bindpos++).toString();
	r.sungBy = q.value(bindpos++).toString();
	r.quartet = q.value(bindpos++).toString();
	r.posted = QDate::fromJulianDay(bindpos++);
	r.collection = q.value(bindpos++).toString();
	r.rating = q.value(bindpos++).toFloat();
	r.ratingCount = q.value(bindpos++).toInt();
	r.downloaded = q.value(bindpos++).toInt();
	r.bookmarked = q.value(bindpos++).toBool();

	const auto val = q.value(bindpos++).toInt();
	if (val) {
		r.visited = QDateTime::fromSecsSinceEpoch(val);
	}
	return r;
}

QString Tag::mediaId(TagId tagId, QString mediaName) {
	return QString("%1_%2").arg(tagId).arg(mediaName);
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
	q.prepare("UPDATE media SET cache = ? WHERE id = ?");
	int bindpos = 0;
	q.bindValue(bindpos++, data, QSql::ParamTypeFlag::In | QSql::ParamTypeFlag::Binary);
	q.bindValue(bindpos++, mediaId(this->id, name));
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
