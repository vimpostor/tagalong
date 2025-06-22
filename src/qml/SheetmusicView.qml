import QtQuick

import Quartz
import Backend

Item {
	Loader {
		id: loader
		anchors.fill: parent
		active: Backend.documentType == "pdf"
		sourceComponent: PdfViewer {
			anchors.fill: parent
			source: Backend.documentSource
		}
	}
	Image {
		id: img
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
		cache: false
		source: Backend.documentType == "png" ? Backend.documentSource : ""
	}
	IconButton {
		anchors.right: parent.right
		anchors.top: parent.top
		ico.name: "close"
		onClicked: Backend.documentType = ""
	}
}
