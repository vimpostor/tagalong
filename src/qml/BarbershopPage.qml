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
			Api.requestTag(parseInt(text))
			TagCompletionsModel.reset();
		}
		onTextChanged: TagCompletionsModel.complete(text);
	}
	ListView {
		anchors.top: search.bottom
		anchors.left: search.left
		anchors.right: search.right
		anchors.bottom: parent.bottom
		model: TagCompletionsModel
		clip: true
		delegate: IconButton {
			text: modelData.title + " (" + modelData.id + ")"
			ico.name: "search"
			onClicked: {
				Api.requestTag(modelData.id);
				TagCompletionsModel.reset();
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
