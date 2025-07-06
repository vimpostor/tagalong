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

	tags = tags | std::views::filter([&](const auto &t) { return std::ranges::all_of(predicates, [&](auto &p) { return p(t); }); }) | std::ranges::to<std::vector>();
}

void TagCompletionsModel::sort(std::vector<Tag> &t) {
	if (sorting.isEmpty()) {
		return;
	}
	std::unordered_map<QString, std::function<bool(const Tag &, const Tag &)>> predicates {
		{"Title", [](const auto &a, const auto &b) { return a.title < b.title; }},
		{"Posted", [](const auto &a, const auto &b) { return a.posted < b.posted; }},
		{"Rating", [](const auto &a, const auto &b) { return a.rating < b.rating; }},
		{"Downloaded", [](const auto &a, const auto &b) { return a.downloaded < b.downloaded; }},
		{"Visited", [](const auto &a, const auto &b) { return a.isVisited() && !b.isVisited(); }},
	};
	std::ranges::sort(t, [&](const auto &a, const auto &b) { return descending ^ predicates[sorting](a, b) && !(descending && predicates[sorting](b, a)); });
}
