import QtQuick
import QtQuick.Controls.Material

import Quartz
import Backend

Item {
	Search {
		id: search
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.margins: 16
		placeholderText: "Search Tags"
		onSearched: {
			const t = parseInt(text)
			if (t) {
				Api.requestTag(t)
				TagCompletionsModel.reset();
			}
		}
		onTextChanged: TagCompletionsModel.complete(text);
	}
	ListView {
		id: listview
		anchors.top: search.bottom
		anchors.left: search.left
		anchors.right: search.right
		anchors.bottom: parent.bottom
		anchors.topMargin: 8
		model: TagCompletionsModel
		clip: true
		spacing: 8
		delegate: Card {
			width: listview.width
			height: 50
			Column {
				anchors.fill: parent
				anchors.margins: 8
				Label {
					text: modelData.title
				}
				Label {
					text: "ID: " + modelData.id
				}
			}
			TapHandler {
				onSingleTapped: {
					Api.requestTag(modelData.id);
					TagCompletionsModel.reset();
				}
			}
		}
	}
	Dialog {
		title: "Syncing"
		visible: Api.isSyncing
		anchors.centerIn: parent
		closePolicy: Popup.NoAutoClose
		Column {
			spacing: 8
			Label {
				text: "Downloading tags metadata"
			}
			ProgressBar {
				id: progressbar
				value: Api.syncProgress
				indeterminate: !value
			}
		}
	}
}
