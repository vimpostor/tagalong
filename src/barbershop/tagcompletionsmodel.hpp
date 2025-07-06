#pragma once

#include <QAbstractListModel>
#include <vector>

#include "api.hpp"

class TagCompletionsModel : public QAbstractListModel {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	virtual int rowCount(const QModelIndex &) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE void complete(QString q);
	Q_INVOKABLE void reset();
	Q_INVOKABLE void setCollection(QString c);
	Q_INVOKABLE void setLearningTrack(bool l);
	Q_INVOKABLE void setSorting(QString s);
	Q_INVOKABLE void setDescending(bool s);
private:
	std::vector<Tag> tags;
	QString query;
	QString collection;
	bool needsLearningTrack = false;
	QString sorting;
	bool descending = false;

	void filter(std::vector<Tag> &candidates);
	void sort(std::vector<Tag> &t);
};
