import QtQuick
import QtQuick.Controls.Material
import QtQuick.Pdf

import Quartz

Rectangle {
	id: root
	property alias source: pdf.source
	color: "#FFFFFF"
	Flickable {
		id: flickable
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
			sourceSize.width: flickable.contentWidth
			sourceSize.height: flickable.contentHeight
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
			WheelHandler {
				acceptedModifiers: Qt.ControlModifier
				onWheel: e => pinch.persistentScale *= 1.2 ** (e.angleDelta.y / 120)
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
