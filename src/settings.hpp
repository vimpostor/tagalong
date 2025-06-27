#pragma once

#include <QSettings>
#include <quartz/macros.hpp>

class Settings : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	QML_CPP_SINGLETON(Settings)

	bool getSynced() const;
	void setSynced(bool synced);
private:
	QSettings settings;
};
