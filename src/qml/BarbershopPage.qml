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
			ComboChip {
				category: "Collection"
				model: ["classic", "easytags", "100"]
			}
			CheckChip {
				text: "Learning Track"
			}
			ComboChip {
				category: "Sort by"
				model: ["Title", "Posted", "Rating", "Downloaded", "Visited"]
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
				height: 60
				Column {
					anchors.left: parent.left
					anchors.top: parent.top
					anchors.bottom: parent.bottom
					anchors.right: bookmark.right
					anchors.margins: 8
					spacing: 4
					Label {
						text: modelData.title
						font.pointSize: 12
					}
					Row {
						spacing: 8
						height: 24
						Icon {
							name: "info"
						}
						Label {
							text: modelData.id
							verticalAlignment: Text.AlignVCenter
							height: parent.height
						}
						Icon {
							name: "download"
						}
						Label {
							text: modelData.downloaded
							verticalAlignment: Text.AlignVCenter
							height: parent.height
						}
						Icon {
							name: "offline_pin"
							visible: modelData.visited
						}
						Icon {
							name: "calendar_clock"
						}
						Label {
							text: modelData.posted.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
							verticalAlignment: Text.AlignVCenter
							height: parent.height
						}
					}
				}
				IconButton {
					id: bookmark
					anchors.right: parent.right
					anchors.top: parent.top
					anchors.bottom: parent.bottom
					ico.name: "bookmark"
					ico.filled: checked
					checkable: true
					onCheckedChanged: modelData.setBookmarked(checked)
					Component.onCompleted: checked = modelData.bookmarked
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
