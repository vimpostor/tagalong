import QtQuick
import QtQuick.Controls.Material
import QtQuick.Pdf

import Quartz

Rectangle {
	id: root
	property alias source: pdf.source
	color: "#FFFFFF"
	Flickable {
		anchors.fill: parent
		contentWidth: pdf.width * pdf.scale
		contentHeight: pdf.height * pdf.scale
		Image {
			id: pdf
			width: root.width
			height: root.height
			scale: pinch.persistentScale
			transformOrigin: Item.TopLeft
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
			PinchHandler {
				id: pinch
				target: null
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
