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
		visible: !loader.active
	}
	Loader {
		id: loader
		anchors.fill: parent
		active: Backend.documentSource != ""
		sourceComponent: PdfViewer {
			anchors.fill: parent
			source: "file://" + Backend.documentSource
		}
	}
	Snackbar {
		text: Backend.snackbar
		onTextChanged: open();
	}
}
