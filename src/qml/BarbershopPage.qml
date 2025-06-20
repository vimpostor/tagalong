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
	Dialog {
		title: "Syncing"
		visible: Api.isSyncing
		anchors.centerIn: parent
		closePolicy: Popup.NoAutoClose
		Column {
			spacing: 8
			Label {
				text: progressbar.indeterminate ? "Downloading metadata" : "Parsing metadata"
			}
			ProgressBar {
				id: progressbar
				value: Api.syncProgress
				onValueChanged: console.log(value)
				indeterminate: !value
			}
		}
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
