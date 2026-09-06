#pragma once

#include <QAbstractListModel>

#include "tag.hpp"
#include <quartz/macros.hpp>

class PlaybackModel : public QAbstractListModel {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	QML_CPP_SINGLETON(PlaybackModel)

	virtual int rowCount(const QModelIndex &) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE void play(int i);
	Q_INVOKABLE void reset();
	void setTag(const Tag &tag);
signals:
	void playRequested(const Media *media);
private:
	std::vector<const Media *> audio;
};
