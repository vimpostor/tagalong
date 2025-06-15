import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

AppWindow {
	id: root
	title: "Tag along"
	width: 800
	height: 600
	Material.theme: Material.System
	Material.primary: Material.Indigo
	Material.accent: Material.Pink
	BarbershopPage {
		anchors.fill: parent
	}
}
