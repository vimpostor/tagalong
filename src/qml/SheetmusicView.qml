import QtQuick

import Quartz
import Backend

Item {
	Loader {
		id: loader
		anchors.fill: parent
		active: Backend.documentType.length
		sourceComponent: PdfViewer {
			anchors.fill: parent
			source: Backend.documentSource
		}
	}
	IconButton {
		anchors.right: parent.right
		anchors.top: parent.top
		ico.name: "close"
		onClicked: Backend.documentType = ""
	}
}
