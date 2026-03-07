import QtQuick

import Backend

Item {
	Loader {
		id: loader
		anchors.fill: parent
		active: Backend.documentType.length
		onLoaded: forceActiveFocus();
		sourceComponent: PdfViewer {
			anchors.fill: parent
			source: Backend.documentSource
		}
	}
}
