import QtQuick
import QtQuick.Controls.Material
import Quartz

import Backend

AppWindow {
	id: root
	title: "Tag along"
	width: 800
	height: 600
	Material.theme: Material.System
	Material.primary: Material.Indigo
	Material.accent: Material.Pink
	NavigationDrawerPage {
		navIndex: Settings.getCurrentTab()
		model: ["Barbershop", "Piano", "Settings"]
		onCurrentIndexChanged: Settings.setCurrentTab(currentIndex);
		Item {
			BarbershopPage {
				anchors.fill: parent
				visible: !Backend.documentType.length
			}
			SheetmusicView {
				anchors.fill: parent
				visible: Backend.documentType.length
			}
			Snackbar {
				text: Backend.snackbar
				onTextChanged: open();
			}
		}
		Item {
			Piano {
				anchors.fill: parent
			}
		}
		Item {
			Label {
				anchors.centerIn: parent
				text: "Nothing here yet"
			}
		}
	}
}
