import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia

import Quartz
import Backend

Item {
	ColumnLayout {
		anchors.fill: parent
		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Wave {
				id: wave
				anchors.fill: parent
				centreY: height
				color: Material.color(Material.Blue, Material.Shade100)
				ListView {
					anchors.fill: parent
					id: listview
					anchors.bottom: parent.bottom
					model: PlaybackModel
					delegate: IconButton {
						width: parent.width
						text: modelData
						flat: true
						ico.name: "audio_file"
						onClicked: {
							button.click();
							PlaybackModel.play(index);
						}
					}
				}
			}
		}
		Item {
			height: button.height
			Layout.fillWidth: true
			IconButton {
				id: button
				anchors { left: parent.left; right: controls.left; }
				visible: (listview.count && Backend.overlayVisible) || loader.active
				flat: true
				text: listview.count + " learning tracks"
				ico.name: "audiotrack"
				onClicked: {
					Backend.audioSource = "";
					wave.size = !wave.size;
				}
			}
			RowLayout {
				id: controls
				visible: loader.active
				width: parent.width / 3 * 2 * visible
				anchors.right: parent.right
				Behavior on width {
					NumberAnimation { duration: 300; easing.type: Easing.OutCirc; }
				}
				Slider {
					Layout.fillWidth: true
					value: loader.active ? loader.item.position : 0
					to: loader.active ? loader.item.duration : 0
					onMoved: loader.item.position = position * to;
				}
				IconButton {
					ico.name: "play_pause"
					onClicked: {
						if (loader.item.playing) {
							loader.item.pause();
						} else {
							loader.item.play();
						}
					}
				}
			}
		}
	}
	Loader {
		id: loader
		active: Backend.audioSource.toString().length
		sourceComponent: MediaPlayer {
			source: Backend.audioSource
			autoPlay: true
			audioOutput: AudioOutput {}
		}
	}
}
