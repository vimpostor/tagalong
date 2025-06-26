#include "api.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "backend.hpp"

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	initDb();
	if (db.tables().empty()) {
		syncMetadata();
	}
}

void Api::requestTag(TagId id) {
	auto res = tagFromId(id);
	if (!res) {
		Backend::get()->notifySnackbar("Tag not found");
		return;
	}

	if (res->cachedsheetmusic.isEmpty()) {
		downloadSheetmusic(*res);
	} else {
		writeSheetmusic(*res);
	}
}

void Api::downloadSheetmusic(Tag &tag) {
	auto res = manager.get(QNetworkRequest(tag.sheetMusicAlt));
	connect(res, &QNetworkReply::finished, this, std::bind(&Api::handleSheetmusic, this, res, tag));
}

void Api::handleSheetmusic(QNetworkReply *reply, Tag tag) {
	if (reply->error()) {
		Backend::get()->notifySnackbar("Download failed: " + reply->errorString());
		return;
	}
	tag.cachedsheetmusic = reply->readAll();
	QSqlQuery q;
	q.prepare("UPDATE tags SET cachedsheetmusic = ? WHERE id = ?");
	q.bindValue(0, tag.cachedsheetmusic, QSql::ParamTypeFlag::In | QSql::ParamTypeFlag::Binary);
	q.bindValue(1, tag.id);
	if (!q.exec()) {
		qWarning() << "Failed to store sheet music";
		return;
	}
	writeSheetmusic(tag);

	reply->deleteLater();
}

void Api::writeSheetmusic(Tag &tag) {
	QFile f {QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/blob"};
	f.open(QFile::OpenModeFlag::WriteOnly);
	f.write(tag.cachedsheetmusic);
	f.close();
	tag.sheetmusiclocation = QUrl::fromLocalFile(f.fileName());
	Backend::get()->setDocumentSource(tag.sheetmusiclocation);
	const auto suffix = QFileInfo(tag.sheetMusicAlt.toString()).suffix();
	Backend::get()->setDocumentType(suffix == "pdf" ? "pdf" : "png");
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
	QSqlQuery q;
	q.exec("CREATE TABLE tags(id INT PRIMARY KEY NOT NULL, title TEXT, sheetmusic TEXT, cachedsheetmusic BLOB DEFAULT NULL)");
	xml.clear();
	pendingtags.clear();
	invideo = false;
	tagsAvailable = 0;
	currentIndex = 0;

	auto req = endpoint;
	// cannot be bothered with pagination, just return all tags in one go
	req.setQuery("n=9999");
	reply = manager.get(QNetworkRequest(req));
	connect(reply, &QNetworkReply::readyRead, this, &Api::parseTags);
	m_isSyncing = true;
	emit syncingChanged();
}

Tag Api::tagFromQuery(QSqlQuery &q) const {
	Tag res;
	res.id = q.value(0).toInt();
	res.title = q.value(1).toString();
	res.sheetMusicAlt = q.value(2).toUrl();
	res.cachedsheetmusic = q.value(3).toByteArray();
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

void Api::parseTags() {
	if (reply->error()) {
		Backend::get()->notifySnackbar("Network request failed: " + reply->errorString());
		return;
	}

	auto data = reply->readAll();
	if (!currentIndex && data.startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\"")) {
		// API incorrectly reports the wrong encoding, which breaks XML parsing
		data.replace(30, 5, "ISO-8859-1");
	}
	xml.addData(data);

	while (!xml.atEnd()) {
		const auto token = xml.readNext();
		if (token == QXmlStreamReader::StartElement && !invideo) {
			currentName = xml.name().toString();
			if (currentName == "tags") {
				tagsAvailable = xml.attributes().value("count").toInt();
				pendingtags.reserve(tagsAvailable);
			} else if (currentName == "tag") {
				currenttag = {};
			} else if (currentName == "videos") {
				invideo = true;
			}
		} else if (token == QXmlStreamReader::EndElement) {
			if (xml.name() == "tag") {
				pendingtags.emplace_back(currenttag);
				currentIndex++;
			} else if (xml.name() == "videos") {
				invideo = false;
			}
		} else if (token == QXmlStreamReader::Characters && !invideo && !xml.isWhitespace()) {
			if (currentName == "id") {
				currenttag.id = xml.text().toInt();
			} else if (currentName == "Title") {
				currenttag.title = xml.text().toString();
			} else if (currentName == "AltTitle") {
				currenttag.altTitle = xml.text().toString();
			} else if (currentName == "WritKey") {
				currenttag.key = xml.text().toString();
			} else if (currentName == "Parts") {
				currenttag.parts = xml.text().toInt();
			} else if (currentName == "Notes") {
				currenttag.notes = xml.text().toString();
			} else if (currentName == "Arranger") {
				currenttag.arranger = xml.text().toString();
			} else if (currentName == "Arranged") {
				currenttag.arranged = xml.text().toString();
			} else if (currentName == "SungBy") {
				currenttag.sungBy = xml.text().toString();
			} else if (currentName == "SungYear") {
				currenttag.sungYear = xml.text().toString();
			} else if (currentName == "Quartet") {
				currenttag.quartet = xml.text().toString();
			} else if (currentName == "Posted") {
				currenttag.posted = QDate::fromString(xml.text().toString(), Qt::DateFormat::RFC2822Date);
			} else if (currentName == "Collection") {
				currenttag.collection = xml.text().toString();
			} else if (currentName == "Rating") {
				currenttag.rating = xml.text().toFloat();
			} else if (currentName == "RatingCount") {
				currenttag.ratingCount = xml.text().toInt();
			} else if (currentName == "Downloaded") {
				currenttag.downloaded = xml.text().toInt();
			} else if (currentName == "SheetMusic") {
				currenttag.sheetmusic = xml.text().toString();
			} else if (currentName == "SheetMusicAlt") {
				currenttag.sheetMusicAlt = xml.text().toString();
			}
		} else if (token == QXmlStreamReader::EndDocument && xml.error() == QXmlStreamReader::Error::NoError) {
			// insert tags
			auto params = QString(" (?, ?, ?, NULL),").repeated(pendingtags.size());
			params.removeLast(); // remove trailing comma
			QSqlQuery q;
			q.prepare("INSERT INTO tags VALUES" + params);
			int bindpos = 0;
			for (size_t i = 0; i < pendingtags.size(); ++i) {
				const auto &t = pendingtags[i];
				q.bindValue(bindpos++, t.id);
				q.bindValue(bindpos++, t.title);
				q.bindValue(bindpos++, t.sheetmusic.toString());
			}
			if (!q.exec()) {
				Backend::get()->notifySnackbar("Failed to insert tags: " + q.lastError().text());
			}

			pendingtags.clear();
			reply->deleteLater();
			m_isSyncing = false;
			emit syncingChanged();
		}
	}

	if (m_isSyncing && tagsAvailable > 0) {
		m_syncProgress = static_cast<float>(currentIndex) / tagsAvailable;
		emit syncingChanged();
	}
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
