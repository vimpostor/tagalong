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
private:
	std::vector<Tag> tags;
};
