#include "settings.hpp"

bool Settings::getSynced() const {
	return settings.value("synced", false).toBool();
}

void Settings::setSynced(bool synced) {
	settings.setValue("synced", synced);
}

int Settings::getCurrentTab() const {
	return settings.value("currentTab", 0).toInt();
}

void Settings::setCurrentTab(int i) {
	settings.setValue("currentTab", i);
}
