#include "tagcompletionsmodel.hpp"

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
	beginResetModel();
	tags = Api::get()->complete(q);
	endResetModel();
}

void TagCompletionsModel::reset() {
	complete("");
}
