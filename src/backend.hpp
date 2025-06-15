#pragma once

#include "barbershop/api.hpp"

class Backend : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	QML_CPP_SINGLETON(Backend)
};
