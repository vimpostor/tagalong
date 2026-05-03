import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

import Backend

Item {
	Repeater {
		model: 12
		Rectangle {
			property bool isBlack: (index % 2) ^ (index > 4)
			x: Math.floor((index + (index > 4)) / 2) * parent.width / 7 + isBlack * parent.width / 14 * 1.5
			width: parent.width / 7 / (1 + isBlack)
			height: parent.height / (1 + isBlack)
			color: taphandler.pressed ? Material.color(Material.Blue, Material.Shade500) : isBlack ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade200)
			border.width: 1
			z: isBlack
			TapHandler {
				id: taphandler
				gesturePolicy: TapHandler.WithinBounds
				onPressedChanged: Audio.play(pressed ? index : -1);
			}
		}
	}
}
