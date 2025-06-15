#include "api.hpp"

#include <QDomDocument>

const QUrl endpoint {"http://www.barbershoptags.com/api.php"};

void Api::init() {
	connect(&manager, &QNetworkAccessManager::finished, this, &Api::parseTags);
}

void Api::requestTag(TagId id) {
	auto req = endpoint;
	req.setQuery(QString("id=%1").arg(id));
	manager.get(QNetworkRequest(req));
}

void Api::parseTags(QNetworkReply *res) {
	const auto data = res->readAll();
	res->deleteLater();
	QDomDocument doc;
	const auto success = doc.setContent(data);
	if (!success) {
		qWarning() << "Failed to parse XML" << success.errorMessage;
		return;
	}
	auto i = doc.firstChildElement("tags").firstChildElement("tag");
	while (true) {
		if (i.isNull()) {
			return;
		}

		Tag tag {i};
		emit tagReady(tag);

		i = i.nextSiblingElement("tag");
	}
}
