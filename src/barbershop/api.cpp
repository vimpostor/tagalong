#include "api.hpp"

#include <QDir>
#include <QGuiApplication>
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
	m_isSyncing = true;
	emit syncingChanged();
}

void Api::parseTags(QNetworkReply *res) {
	auto data = res->readAll();
	if (data.startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\"")) {
		// API incorrectly reports the wrong encoding, which breaks XML parsing
		data.replace(30, 5, "ISO-8859-1");
	}
	QXmlStreamReader r {data};
	QSqlQuery q;
	q.exec("CREATE TABLE tags(id INT PRIMARY KEY NOT NULL, title TEXT, sheetmusic TEXT)");
	std::vector<Tag> tags;
	Tag tag;
	bool invideo = false;
	int tagsAvailable = 0;
	int currentTag = 0;

	while (!r.atEnd()) {
		const auto token = r.readNext();
		if (token == QXmlStreamReader::StartElement && !invideo) {
			if (r.name() == "tags") {
				tagsAvailable = r.attributes().value("count").toInt();
				tags.reserve(tagsAvailable);
			} else if (r.name() == "tag") {
				tag = {};
			} else if (r.name() == "id") {
				tag.id = r.readElementText().toInt();
			} else if (r.name() == "Title") {
				tag.title = r.readElementText();
			} else if (r.name() == "SheetMusic") {
				tag.sheetmusic = r.readElementText();
			} else if (r.name() == "videos") {
				invideo = true;
			}
		} else if (token == QXmlStreamReader::EndElement) {
			if (r.name() == "tag") {
				tags.emplace_back(tag);
				currentTag++;
				if (tagsAvailable > 0) {
					m_syncProgress = static_cast<float>(currentTag) / tagsAvailable;
					emit syncingChanged();
					QGuiApplication::processEvents();
				}
			} else if (r.name() == "videos") {
				invideo = false;
			}
		}
	}

	// insert tags
	auto params = QString(" (?, ?, ?),").repeated(tags.size());
	params.removeLast(); // remove trailing comma
	q.prepare("INSERT INTO tags VALUES" + params);
	int bindpos = 0;
	for (size_t i = 0; i < tags.size(); ++i) {
		const auto &t = tags[i];
		q.bindValue(bindpos++, t.id);
		q.bindValue(bindpos++, t.title);
		q.bindValue(bindpos++, t.sheetmusic.toString());
	}
	if (!q.exec()) {
		qWarning() << "Failed to insert tags:" << q.lastError().text();
	}

	res->deleteLater();
	m_isSyncing = false;
	emit syncingChanged();
}

void Api::initDb() {
	db = QSqlDatabase::addDatabase("QSQLITE");
	const QDir dir {QStandardPaths::writableLocation(QStandardPaths::CacheLocation)};
	if (!dir.exists()) {
		if (!dir.mkpath(dir.path())) {
			qWarning() << "Failed to create cache directory";
			return;
		}
	}
	db.setDatabaseName(dir.path() + "/tagalong.sqlite");
	if (!db.open()) {
		qWarning() << "Failed to open db:" << db.lastError().text();
		return;
	}
}
