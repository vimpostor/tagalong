#pragma once

#include <QAudioSink>

#include <quartz/macros.hpp>

class AudioBuffer : public QIODevice {
	Q_OBJECT
public:
	AudioBuffer();

	uint32_t samplerate = 48000;
	float frequency = 440;
	bool stop = true;
	uint32_t currentFadeSample = 0;
protected:
	qint64 readData(char *data, qint64 maxSize) override;
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 bytesAvailable() const override;
private:
	uint32_t currentSample = 0;
	static const constexpr qint64 defaultSize = 256;
	static const constexpr qint64 fadeSamples = defaultSize;
};

class Audio : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON
public:
	QML_CPP_SINGLETON(Audio)

	Q_INVOKABLE void play(int note);
private:
	void init();

	QAudioSink *sink = nullptr;
	AudioBuffer buf;
	bool ok = false;
};
