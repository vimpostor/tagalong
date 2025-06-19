import QtQuick
import QtQuick.Controls.Material

import Quartz
import Backend

Item {
	Search {
		id: search
		focus: true
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.margins: 16
		placeholderText: "Search Tags"
		onSearched: Api.requestTag(parseInt(text))
	}
	BusyIndicator {
		anchors.centerIn: parent
		running: Api.isSyncing
	}

	Image {
		id: img
		anchors.fill: parent
		anchors.margins: 8
		fillMode: Image.PreserveAspectFit
	}
	Connections {
		target: Api
		function onTagReady(tag) {
			img.source = tag.sheetmusicUrl
		}
	}
}
