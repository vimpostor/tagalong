#include "settings.hpp"

bool Settings::getSynced() const {
	return settings.value("synced", false).toBool();
}

void Settings::setSynced(bool synced) {
	settings.setValue("synced", synced);
}
