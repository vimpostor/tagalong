import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

Item {
	Button {
		anchors.centerIn: parent
		text: "Clear cache"
		onClicked: Api.reset();
	}
}
