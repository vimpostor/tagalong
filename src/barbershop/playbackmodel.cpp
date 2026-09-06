#include "playbackmodel.hpp"

#include "backend.hpp"

int PlaybackModel::rowCount(const QModelIndex &) const {
	return audio.size();
}

QVariant PlaybackModel::data(const QModelIndex &index, int) const {
	return QVariant::fromValue(audio[index.row()]->name);
}

QHash<int, QByteArray> PlaybackModel::roleNames() const {
	return {{Qt::UserRole, "name"}};
}

void PlaybackModel::play(int i) {
	emit playRequested(audio[i]);
}

void PlaybackModel::reset() {
	Backend::get()->setAudioSource({});
	beginResetModel();
	audio.clear();
	endResetModel();
}

void PlaybackModel::setTag(const Tag &tag) {
	beginResetModel();
	for (auto &[_, m] : tag.media) {
		if (m.isAudio()) {
			audio.emplace_back(&m);
		}
	}
	endResetModel();
}
