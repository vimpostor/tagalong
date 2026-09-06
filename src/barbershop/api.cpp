#include "api.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QStandardPaths>

#include "backend.hpp"
#include "playbackmodel.hpp"
#include "settings.hpp"

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	initDb();
	if (!Settings::get()->getSynced() || db.tables().empty()) {
		syncMetadata();
	}
	connect(PlaybackModel::get(), &PlaybackModel::playRequested, this, &Api::handlePlayRequest);
}

void Api::reset() {
	auto file = db.databaseName();
	db.close();
	QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnectionName());
	Settings::get()->setSynced(false);
	if (!file.isEmpty()) {
		QFile f {file};
		f.remove();
	}
	init();
}

void Api::requestTag(TagId id) {
	PlaybackModel::get()->reset();
	auto res = tagFromId(id);
	if (!res) {
		Backend::get()->notifySnackbar("Tag not found");
		return;
	}
	currenttag = *res;

	currenttag.setVisited();
	currenttag.fetchMedia();
	PlaybackModel::get()->setTag(currenttag);
	if (!currenttag.media.contains("SheetMusicAlt")) {
		Backend::get()->notifySnackbar("No sheet music provided.");
		return;
	}
	auto &m = currenttag.media["SheetMusicAlt"];
	downloadAndView(m);
}

void Api::downloadMedia(const Media &media, Tag &tag) {
	if (media.url.isEmpty()) {
		Backend::get()->notifySnackbar("Empty download link.");
		return;
	}
	auto reply = manager.get(QNetworkRequest(media.url));
	connect(reply, &QNetworkReply::finished, this, std::bind(&Api::handleMediaDownload, this, reply, media, std::ref(tag)));
	m_downloadActive = true;
	emit downloadActiveChanged();
}

void Api::handleMediaDownload(QNetworkReply *reply, const Media &media, Tag &tag) {
	reply->deleteLater();
	m_downloadActive = false;
	emit downloadActiveChanged();
	if (reply->error()) {
		Backend::get()->notifySnackbar("Download failed: " + reply->errorString());
		return;
	}
	tag.setMedia(media.name, media.url, reply->readAll());
	writeMedia(tag.media[media.name]);
}

void Api::writeMedia(const Media &media) {
	const bool isAudio = media.isAudio();
	QFile f {QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + (isAudio ? "audio" : "blob")};
	if (!f.open(QFile::OpenModeFlag::WriteOnly)) {
		return;
	}

	f.write(media.cache);
	f.close();

	auto src = QUrl::fromLocalFile(f.fileName());
	if (isAudio) {
		Backend::get()->setAudioSource(src);
	} else {
		Backend::get()->setDocumentSource(src);
		const auto suffix = QFileInfo(media.url.toString()).suffix();
		Backend::get()->setDocumentType(suffix == "pdf" ? "pdf" : "png");
	}
}

void Api::handlePlayRequest(const Media *media) {
	downloadAndView(*media);
}

std::vector<Tag> Api::complete(QString query) {
	if (!db.open()) {
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
		res.push_back(Tag::fromQuery(q));
	}
	return res;
}

void Api::syncMetadata() {
	QSqlQuery q;
	q.exec("CREATE TABLE tags(id INT PRIMARY KEY NOT NULL, title TEXT, alttitle TEXT, key TEXT, parts INT, notes TEXT, arranger TEXT, arranged TEXT, sungby TEXT, quartet TEXT, posted INT, collection TEXT, rating REAL, ratingcount INT, downloaded INT, bookmarked INT, visited INT)");
	// generic table for media associated to a tag
	q.exec("CREATE TABLE media(id TEXT PRIMARY KEY NOT NULL, tag INT, name TEXT, url TEXT, cache BLOB DEFAULT NULL)");
	xml.clear();
	pendingtags.clear();
	pendingmedia.clear();
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

std::optional<Tag> Api::tagFromId(TagId id) const {
	QSqlQuery q {"SELECT * FROM tags WHERE id = " + QString::number(id)};
	q.exec();
	if (!q.first()) {
		return std::nullopt;
	}

	return Tag::fromQuery(q);
}

void Api::parseTags() {
	constexpr const auto mediaNames = std::to_array<QStringView>({u"SheetMusic", u"SheetMusicAlt", u"AllParts", u"Bass", u"Bari", u"Lead", u"Tenor"});

	if (reply->error()) {
		Backend::get()->notifySnackbar("Network request failed: " + reply->errorString());
		return;
	}

	auto data = reply->readAll();
	m_bytesReceived += data.size();
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
				pendingmedia.reserve(tagsAvailable * mediaNames.size());
			} else if (currentName == "tag") {
				currenttag = {};
			} else if (currentName == "videos") {
				invideo = true;
			}
		} else if (token == QXmlStreamReader::EndElement) {
			if (xml.name() == "tag") {
				for (auto &[_, m] : currenttag.media) {
					pendingmedia.emplace_back(std::make_pair(currenttag.id, m));
				}
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
			} else if (std::ranges::contains(mediaNames, currentName)) {
				currenttag.media[currentName] = {currentName, xml.text().toString()};
			}
		}
	}

	if (tagsAvailable > 0) {
		m_syncProgress = static_cast<float>(currentIndex) / tagsAvailable;
		emit syncingChanged();
	}

	if (pendingtags.size()) {
		QSqlQuery q;
		int bindpos = 0;

		auto prepare = [&](const QString &construct, const QString &table, unsigned long size) {
			bindpos = 0;
			auto params = QString(" %1,").arg(construct).repeated(size);
			params.removeLast(); // remove trailing comma
			q.prepare(QString("INSERT INTO %1 VALUES%2").arg(table).arg(params));
		};

		// insert tags
		prepare("(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, 0)", "tags", pendingtags.size());
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
		}
		if (!q.exec()) {
			Backend::get()->notifySnackbar("Failed to insert tags: " + q.lastError().text());
		}
		pendingtags.clear();

		if (pendingmedia.size()) {
			// insert media
			prepare("(?, ?, ?, ?, NULL)", "media", pendingmedia.size());
			for (size_t i = 0; i < pendingmedia.size(); ++i) {
				const auto &[id, m] = pendingmedia[i];
				q.bindValue(bindpos++, Tag::mediaId(id, m.name));
				q.bindValue(bindpos++, id);
				q.bindValue(bindpos++, m.name);
				q.bindValue(bindpos++, m.url);
			}
			if (!q.exec()) {
				Backend::get()->notifySnackbar("Failed to insert media: " + q.lastError().text());
			}
			pendingmedia.clear();
		}
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
			qWarning() << "Failed to create database directory";
			return;
		}
	}
	db.setDatabaseName(dir.path() + "/tagalong.sqlite");
	if (!db.open()) {
		Backend::get()->notifySnackbar("Failed to open db: " + db.lastError().text());
		return;
	}
}

void Api::downloadAndView(const Media &media) {
	if (media.cache.isEmpty()) {
		downloadMedia(media, currenttag);
	} else {
		writeMedia(media);
	}
}
