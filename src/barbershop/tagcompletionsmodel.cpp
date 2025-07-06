#include "tagcompletionsmodel.hpp"

#include <ranges>

int TagCompletionsModel::rowCount(const QModelIndex &) const {
	return tags.size();
}

QVariant TagCompletionsModel::data(const QModelIndex &index, int) const {
	return QVariant::fromValue(tags[index.row()]);
}

QHash<int, QByteArray> TagCompletionsModel::roleNames() const {
	return {{Qt::UserRole, "tag"}};
}

void TagCompletionsModel::complete(QString q) {
	query = q;
	beginResetModel();
	tags = Api::get()->complete(q);
	filter(tags);
	sort(tags);
	endResetModel();
}

void TagCompletionsModel::reset() {
	complete("");
}

void TagCompletionsModel::setCollection(QString c) {
	collection = c;
	complete(query);
}

void TagCompletionsModel::setLearningTrack(bool l) {
	needsLearningTrack = l;
	complete(query);
}

void TagCompletionsModel::setBookmarked(bool b) {
	needsBookmarked = b;
	complete(query);
}

void TagCompletionsModel::setSorting(QString s) {
	sorting = s;
	complete(query);
}

void TagCompletionsModel::setDescending(bool s) {
	descending = s;
	complete(query);
}

void TagCompletionsModel::filter(std::vector<Tag> &candidates) {
	std::vector<std::function<bool(const Tag &)>> predicates;
	if (!collection.isEmpty()) {
		predicates.push_back([&](auto &t) { return t.collection == collection; });
	}
	if (needsLearningTrack) {
		// TODO: Add support for learning tracks
		predicates.push_back([&](auto &t) { return false; });
	}
	if (needsBookmarked) {
		predicates.push_back([&](auto &t) { return t.bookmarked; });
	}

	tags = tags | std::views::filter([&](const auto &t) { return std::ranges::all_of(predicates, [&](auto &p) { return p(t); }); }) | std::ranges::to<std::vector>();
}

void TagCompletionsModel::sort(std::vector<Tag> &t) {
	if (sorting.isEmpty()) {
		return;
	}
	std::unordered_map<QString, std::function<std::partial_ordering(const Tag &, const Tag &)>> predicates {
		{"Title", [](const auto &a, const auto &b) { return a.title <=> b.title; }},
		{"Posted", [](const auto &a, const auto &b) { return a.posted <=> b.posted; }},
		{"Rating", [](const auto &a, const auto &b) { return a.rating <=> b.rating; }},
		{"Downloaded", [](const auto &a, const auto &b) { return a.downloaded <=> b.downloaded; }},
		{"Visited", [](const auto &a, const auto &b) { return a.visited <=> b.visited; }},
	};
	std::ranges::sort(t, [&](const auto &a, const auto &b) { return predicates[sorting](a, b) == (descending ? std::partial_ordering::greater : std::partial_ordering::less); });
}
