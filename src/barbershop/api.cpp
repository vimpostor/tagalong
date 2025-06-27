#include "api.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "backend.hpp"
#include "settings.hpp"

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	initDb();
	if (!Settings::get()->getSynced() || db.tables().empty()) {
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
	m_downloadActive = true;
	emit downloadActiveChanged();
}

void Api::handleSheetmusic(QNetworkReply *reply, Tag tag) {
	reply->deleteLater();
	m_downloadActive = false;
	emit downloadActiveChanged();
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
	q.exec("CREATE TABLE tags(id INT PRIMARY KEY NOT NULL, title TEXT, alttitle TEXT, key TEXT, parts INT, notes TEXT, arranger TEXT, arranged TEXT, sungby TEXT, quartet TEXT, posted INT, collection TEXT, rating REAL, ratingcount INT, downloaded INT, sheetmusic TEXT, sheetmusicalt TEXT, bookmarked INT, cachedsheetmusic BLOB DEFAULT NULL)");
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
	connect(reply, &QNetworkReply::finished, this, &Api::handleTagsFinished);
	m_isSyncing = true;
	emit syncingChanged();
}

Tag Api::tagFromQuery(QSqlQuery &q) const {
	Tag res;
	res.id = q.value(0).toInt();
	res.title = q.value(1).toString();
	res.altTitle = q.value(2).toString();
	res.key = q.value(3).toString();
	res.parts = q.value(4).toInt();
	res.notes = q.value(5).toString();
	res.arranger = q.value(6).toString();
	res.arranged = q.value(7).toString();
	res.sungBy = q.value(8).toString();
	res.quartet = q.value(9).toString();
	res.posted = QDate::fromJulianDay(q.value(10).toInt());
	res.collection = q.value(11).toString();
	res.rating = q.value(12).toFloat();
	res.ratingCount = q.value(13).toInt();
	res.downloaded = q.value(14).toInt();
	res.sheetmusic = q.value(15).toUrl();
	res.sheetMusicAlt = q.value(16).toUrl();
	res.bookmarked = q.value(17).toBool();
	res.cachedsheetmusic = q.value(18).toByteArray();
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

	QXmlStreamReader::TokenType token = QXmlStreamReader::TokenType::NoToken;
	while (!xml.atEnd()) {
		token = xml.readNext();
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
		}
	}

	if (tagsAvailable > 0) {
		m_syncProgress = static_cast<float>(currentIndex) / tagsAvailable;
		emit syncingChanged();
	}

	if (pendingtags.size()) {
		// insert tags
		auto params = QString(" (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, NULL),").repeated(pendingtags.size());
		params.removeLast(); // remove trailing comma
		QSqlQuery q;
		q.prepare("INSERT INTO tags VALUES" + params);
		int bindpos = 0;
		for (size_t i = 0; i < pendingtags.size(); ++i) {
			const auto &t = pendingtags[i];
			q.bindValue(bindpos++, t.id);
			q.bindValue(bindpos++, t.title);
			q.bindValue(bindpos++, t.altTitle);
			q.bindValue(bindpos++, t.key);
			q.bindValue(bindpos++, t.parts);
			q.bindValue(bindpos++, t.notes);
			q.bindValue(bindpos++, t.arranger);
			q.bindValue(bindpos++, t.arranged);
			q.bindValue(bindpos++, t.sungBy);
			q.bindValue(bindpos++, t.quartet);
			q.bindValue(bindpos++, t.posted.toJulianDay());
			q.bindValue(bindpos++, t.collection);
			q.bindValue(bindpos++, t.rating);
			q.bindValue(bindpos++, t.ratingCount);
			q.bindValue(bindpos++, t.downloaded);
			q.bindValue(bindpos++, t.sheetmusic.toString());
			q.bindValue(bindpos++, t.sheetMusicAlt.toString());
		}
		if (!q.exec()) {
			Backend::get()->notifySnackbar("Failed to insert tags: " + q.lastError().text());
		}

		pendingtags.clear();
	}

	if (token == QXmlStreamReader::EndDocument && xml.error() == QXmlStreamReader::Error::NoError && currentIndex == tagsAvailable) {
		Settings::get()->setSynced(true);
	}
}

void Api::handleTagsFinished() {
	reply->deleteLater();
	if (reply->error()) {
		Backend::get()->notifySnackbar("Error downloading: " + reply->errorString());
	}
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
