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

	/**
	 * Avoid integer overflow.
	 * Unfortunately using modulo is only sufficient for integer frequencies.
	 * For floating point frequencies in general we have to derive a slightly more complicated formula.
	 * However this should still be faster than arcsin.
	 *
	 * Using the sin equality sin(x) = sin(x + i * 2 * pi) for any integer i, we can adjust the new sample index.
	 * Let f be the tone frequency with integer part fi and decimal part fd, s the samplerate, i the current sample index,
	 * j = i % samplerate the downshifted sample index, tau = 2 * pi, then we can prove the following:
	 *
	 * sin(tau * f * i/s)
	 * = sin(tau * fi * i/s + tau * fd * i/s)
	 * = sin(tau * fi * j/s + tau * fd * i/s)
	 * = sin(tau * f * fi/f * j/s + tau * f * fd/f * i/s)
	 * = sin(tau * f * ((fi/f * j + fd/f * i) / s))
	 *
	 * Thus we can compute the new downshifted index for arbitrary frequencies as the numerator of the above fraction.
	*/
	float iptr;
	float dec = std::modf(frequency, &iptr);
	currentSample = iptr / frequency * (currentSample % samplerate) + dec / frequency * currentSample;

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
