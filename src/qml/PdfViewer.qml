import QtQuick
import QtQuick.Pdf

PdfPageView {
	id: pdf
	property alias source: doc.source
	document: PdfDocument { id: doc }
	TapHandler {
		onTapped: (p, b) => pdf.goToPage(pdf.currentPage + Math.sign(p.position.x - width / 2))
	}
}
