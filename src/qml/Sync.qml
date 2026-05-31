import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

Item {
	Dialog {
		title: "Syncing"
		visible: Api.isSyncing
		anchors.centerIn: parent
		closePolicy: Popup.NoAutoClose
		Column {
			spacing: 8
			Label {
				text: "Downloading tags metadata" + (Api.bytesReceived ? ": " + (Api.bytesReceived / 1000000).toFixed(1) + " MB" : "")
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
