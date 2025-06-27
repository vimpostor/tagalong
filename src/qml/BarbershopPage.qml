import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Quartz
import Backend

Item {
	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 16
		Search {
			id: search
			property bool insearch: false
			Layout.fillWidth: true
			placeholderText: "Search Tags"
			onSearched: {
				search.insearch = false;
				const t = parseInt(text)
				if (t) {
					Api.requestTag(t)
					TagCompletionsModel.reset();
				}
			}
			onTextChanged: {
				search.insearch = true;
				TagCompletionsModel.complete(text);
			}
			onFocusChanged: {
				if (focus) {
					search.insearch = true;
				}
			}
		}
		ChipFlow {
			Layout.fillWidth: true
			clip: true
			implicitHeight: contentHeight * search.insearch
			Behavior on implicitHeight {
				NumberAnimation { duration: 300; easing.type: Easing.OutCirc; }
			}
			Chip {
				type: Chip.Type.Filter
				text: collections.currentIndex + 1 ? collections.currentText : "Collection"
				leftIco.name: collections.currentIndex + 1 ? "check" : ""
				rightIco.name: "arrow_drop_down"
				onClicked: collections.popup.open()
				onRightIconClicked: collections.popup.open()
				ComboBox {
					id: collections
					property int lastIndex: -1
					visible: false
					currentIndex: -1
					model: ["classic", "easytags", "100"]
					onActivated: (i) => {
						if (i == lastIndex) {
							currentIndex = -1
							lastIndex = -1
						} else {
							lastIndex = currentIndex
						}
					}
				}
			}
			Chip {
				type: Chip.Type.Filter
				text: "Learning Track"
				checkable: true
				leftIco.name: checked ? "check" : ""
			}
			Chip {
				type: Chip.Type.Filter
				text: "Sheet Music"
				checkable: true
				leftIco.name: checked ? "check" : ""
			}
			Chip {
				type: Chip.Type.Filter
				text: sort.currentIndex + 1 ? sort.currentText : "Sort by"
				leftIco.name: sort.currentIndex + 1 ? "check" : ""
				rightIco.name: "arrow_drop_down"
				onClicked: sort.popup.open()
				onRightIconClicked: sort.popup.open()
				ComboBox {
					id: sort
					property int lastIndex: -1
					visible: false
					currentIndex: -1
					model: ["Title", "Posted", "Rating", "Downloaded"]
					onActivated: (i) => {
						if (i == lastIndex) {
							currentIndex = -1
							lastIndex = -1
						} else {
							lastIndex = currentIndex
						}
					}
				}
			}
		}
		ListView {
			id: listview
			Layout.fillWidth: true
			Layout.fillHeight: true
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
	BusyIndicator {
		anchors.centerIn: parent
		visible: Api.downloadActive
	}
}
