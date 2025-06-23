import QtQuick
import QtQuick.Controls.Material
import QtQuick.Pdf

import Quartz

Rectangle {
	property alias source: pdf.source
	color: "#FFFFFF"
	Image {
		id: pdf
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
		cache: false
		TapHandler {
			onTapped: (p, b) => {
				if (p.position.x > pdf.width / 3 * 2) {
					pdf.currentFrame++;
				} else if (p.position.x < pdf.width / 3) {
					pdf.currentFrame--;
				} else {
					ic.visible = !ic.visible
				}
			}
		}
	}
	IconButton {
		id: ic
		anchors.right: parent.right
		anchors.top: parent.top
		visible: false
		ico.name: "close"
		ico.color: Material.primary
		onClicked: Backend.documentType = ""
	}
}
