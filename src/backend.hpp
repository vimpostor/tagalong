#pragma once

#include "barbershop/api.hpp"

class Backend : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(QUrl documentSource MEMBER document CONSTANT)
public:
	QML_CPP_SINGLETON(Backend)
	QUrl document;
};
