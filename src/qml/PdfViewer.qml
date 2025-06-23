import QtQuick
import QtQuick.Pdf

Rectangle {
	property alias source: pdf.source
	color: "#FFFFFF"
	Image {
		id: pdf
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
		TapHandler {
			onTapped: (p, b) => pdf.currentFrame += (p.position.x > pdf.width / 3 * 2) - (p.position.x < pdf.width / 3)
		}
	}
}
