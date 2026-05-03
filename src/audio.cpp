#include "audio.hpp"

AudioBuffer::AudioBuffer() {
	open(QIODeviceBase::ReadOnly);
}

qint64 AudioBuffer::readData(char *data, qint64 maxSize) {
	const auto n = currentSample + (maxSize ? maxSize / sizeof(float) : defaultSize);
	auto d = reinterpret_cast<float *>(data);
	for (; currentSample < n; ++currentSample) {
		*d = std::sin(2 * std::numbers::pi * frequency * currentSample / samplerate);
		++d;
	}
	return maxSize ? maxSize : defaultSize * sizeof(float);
}

qint64 AudioBuffer::writeData(const char *data, qint64 maxSize) {
	return -1;
}

qint64 AudioBuffer::bytesAvailable() const {
	return defaultSize * sizeof(float) + QIODevice::bytesAvailable();
}

void Audio::play(int note) {
	init();
	if (note < 0) {
		sink->stop();
		return;
	}
	buf.frequency = 440 * std::pow(2, (note - 9) / 12.0);
	sink->start(&buf);
}

void Audio::init() {
	if (ok) {
		return;
	}

	auto fmt = QAudioFormat();
	fmt.setChannelCount(1);
	fmt.setSampleRate(buf.samplerate);
	fmt.setSampleFormat(QAudioFormat::SampleFormat::Float);
	sink = std::make_unique<QAudioSink>(fmt);
	ok = true;
}
