import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

Item {
	GroupBox {
		title: "Settings"
		anchors.centerIn: parent
		Column {
			Button {
				text: "Clear entire cache"
				onClicked: Api.reset(true);
			}
			Button {
				text: "Clear only media cache"
				onClicked: Api.reset(false);
			}
		}
	}
}
