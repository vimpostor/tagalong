import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

ApplicationWindow {
	id: root
	visible: true
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
