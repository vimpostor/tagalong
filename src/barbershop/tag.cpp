#include "tag.hpp"

#include <qdebug.h>

Tag::Tag(const QDomElement &node) {
	auto i = node.firstChildElement();
	while (true) {
		if (i.isNull()) {
			return;
		}

		const auto t = i.tagName();
		const auto s = i.text();
		if (t == "id") {
			id = s.toInt();
		} else if (t == "title") {
			title = s;
		} else if (t == "SheetMusic") {
			sheetmusic = s;
		} else if (t == "SheetMusicAlt") {
			sheetMusicAlt = s;
		}

		i = i.nextSiblingElement();
	}
}
