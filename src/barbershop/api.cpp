#include "api.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QXmlStreamReader>

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	connect(&manager, &QNetworkAccessManager::finished, this, &Api::parseTags);
	initDb();
	if (db.tables().empty()) {
		syncMetadata();
	}
}

void Api::requestTag(TagId id) {
	QSqlQuery q {"SELECT * FROM tags WHERE id = " + QString::number(id)};
	if (!q.first()) {
		qWarning() << "Tag not found";
		return;
	}

	Tag tag;
	tag.id = q.value(0).toInt();
	tag.title = q.value(1).toString();
	tag.sheetMusicAlt = q.value(2).toUrl();
	emit tagReady(tag);
}

void Api::syncMetadata() {
	auto req = endpoint;
	// cannot be bothered with pagination, just return all tags in one go
	req.setQuery("n=9999");
	manager.get(QNetworkRequest(req));
	syncing = true;
	emit syncingChanged();
}

void Api::parseTags(QNetworkReply *res) {
	syncing = false;
	emit syncingChanged();

	QXmlStreamReader r {res};
	QSqlQuery q;
	q.exec("CREATE TABLE tags(id INT PRIMARY KEY NOT NULL, title TEXT, sheetmusic TEXT)");
	Tag tag;

	while (!r.atEnd()) {
		const auto token = r.readNext();
		if (token == QXmlStreamReader::StartElement) {
			if (r.name() == "tag") {
				tag = {};
			} else if (r.name() == "id") {
				tag.id = r.readElementText().toInt();
			} else if (r.name() == "Title") {
				tag.title = r.readElementText();
			} else if (r.name() == "SheetMusic") {
				tag.sheetmusic = r.readElementText();
			}
		} else if (token == QXmlStreamReader::EndElement && r.name() == "tag") {
			// insert tag
			q.prepare("INSERT INTO tags VALUES (?, ?, ?)");
			q.bindValue(0, tag.id);
			q.bindValue(1, tag.title);
			q.bindValue(2, tag.sheetmusic.toString());
			q.exec();
		}
	}
	res->deleteLater();
}

void Api::initDb() {
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/tagalong.sqlite");
	if (!db.open()) {
		qWarning() << "Failed to open db:" << db.lastError().text();
		return;
	}
}
