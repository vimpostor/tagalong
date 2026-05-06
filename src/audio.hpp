#pragma once

#include <QAudioSink>
#include <QAudioSource>

#include <fftune.hpp>
#include <quartz/macros.hpp>

static const constexpr qint64 bufferSize = 256;

class AudioBuffer : public QIODevice {
	Q_OBJECT
public:
	AudioBuffer();

	uint32_t samplerate = 48000;
	float frequency = fftune::FreqA4;
	bool stop = true;
	uint32_t currentFadeSample = 0;
protected:
	qint64 readData(char *data, qint64 maxSize) override;
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 bytesAvailable() const override;
private:
	uint32_t currentSample = 0;
	static const constexpr qint64 fadeSamples = bufferSize / 8;
};

class PitchBuffer : public QIODevice {
	Q_OBJECT
public:
	PitchBuffer();
signals:
	void noteDetected(const fftune::note_estimate &note);
protected:
	qint64 readData(char *data, qint64 maxSize) override;
	qint64 writeData(const char *data, qint64 maxSize) override;
private:
	fftune::sample_buffer buf {bufferSize * 4};
	uint32_t sampleCount = 0;
	static constexpr fftune::config conf {.algorithm = fftune::pitch_detection_method::Yin, .buffer_size = bufferSize * 4};
	fftune::pitch_detector<conf> detector {conf};
};

class Audio : public QObject {
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

	Q_PROPERTY(int detectedNote MEMBER detectedNote NOTIFY detectedNoteChanged)
public:
	QML_CPP_SINGLETON(Audio)

	Q_INVOKABLE void play(int note);
	void stop();
signals:
	void detectedNoteChanged();
private:
	void init();
	void handleNote(const fftune::note_estimate &note);

	std::unique_ptr<QAudioSink> sink;
	std::unique_ptr<QAudioSource> source;
	AudioBuffer buf;
	PitchBuffer recording;
	bool ok = false;

	int detectedNote = -1;
};
