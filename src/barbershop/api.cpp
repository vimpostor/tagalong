#include "api.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include "backend.hpp"

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	connect(&manager, &QNetworkAccessManager::finished, this, &Api::parseTags);
	initDb();
	if (db.tables().empty()) {
		syncMetadata();
	}
}

void Api::requestTag(TagId id) {
	auto res = tagFromId(id);
	if (res) {
		emit tagReady(*res);
	} else {
		Backend::get()->notifySnackbar("Tag not found");
	}
}

std::vector<Tag> Api::complete(QString query) {
	if (!db.open() || query.isEmpty()) {
		return {};
	}
	bool ok;
	const auto id = query.toInt(&ok);
	if (id > 0 && ok) {
		// interpret as tag ID search
		auto t = tagFromId(id);
		if (t) {
			return {*t};
		}
	}
	QSqlQuery q;
	q.prepare("SELECT * FROM tags WHERE title LIKE ?");
	q.bindValue(0, "%" + query + "%");

	std::vector<Tag> res;
	q.exec();
	while (q.next()) {
		res.push_back(tagFromQuery(q));
	}
	return res;
}

void Api::syncMetadata() {
	auto req = endpoint;
	// cannot be bothered with pagination, just return all tags in one go
	req.setQuery("n=9999");
	auto res = manager.get(QNetworkRequest(req));
	m_isSyncing = true;
	emit syncingChanged();
}

Tag Api::tagFromQuery(QSqlQuery &q) const {
	Tag res;
	res.id = q.value(0).toInt();
	res.title = q.value(1).toString();
	res.sheetMusicAlt = q.value(2).toUrl();
	return res;
}

std::optional<Tag> Api::tagFromId(TagId id) const {
	QSqlQuery q {"SELECT * FROM tags WHERE id = " + QString::number(id)};
	q.exec();
	if (!q.first()) {
		return std::nullopt;
	}

	return tagFromQuery(q);
}

void Api::parseTags(QNetworkReply *res) {
	if (res->error()) {
		Backend::get()->notifySnackbar("Network request failed: " + res->errorString());
		return;
	}

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
		Backend::get()->notifySnackbar("Failed to insert tags: " + q.lastError().text());
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
		Backend::get()->notifySnackbar("Failed to open db: " + db.lastError().text());
		return;
	}
}
