#include "audio.hpp"

AudioBuffer::AudioBuffer() {
	open(QIODeviceBase::ReadOnly);
}

qint64 AudioBuffer::readData(char *data, qint64 maxSize) {
	const auto n = currentSample + (maxSize ? maxSize / sizeof(float) : bufferSize);
	auto d = reinterpret_cast<float *>(data);
	for (; currentSample < n; ++currentSample) {
		float volume = static_cast<float>(currentFadeSample) / (fadeSamples - 1);
		if (stop) {
			volume = 1 - volume;
		}
		if (currentFadeSample < fadeSamples - 1) {
			currentFadeSample++;
		}
		float tone = 0;
		const constexpr int overtones = 8;
		for (int overtone = 1; overtone <= 8; ++overtone) {
			const constexpr float dampeningConstant = 0.2;
			const float dampening = 1.f - (static_cast<float>(overtone - 1) / static_cast<float>(overtones - 1) * (1.f - dampeningConstant));
			tone += dampening * std::sin(2 * M_PI * overtone * frequency * currentSample / samplerate);
		}
		tone /= overtones - 1;
		*d = volume * tone;
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

	return maxSize ? maxSize : bufferSize * sizeof(float);
}

qint64 AudioBuffer::writeData(const char *data, qint64 maxSize) {
	return -1;
}

qint64 AudioBuffer::bytesAvailable() const {
	return bufferSize * sizeof(float) + QIODevice::bytesAvailable();
}

PitchBuffer::PitchBuffer() {
	open(QIODeviceBase::WriteOnly);
}

qint64 PitchBuffer::readData(char *data, qint64 maxSize) {
	return -1;
}

qint64 PitchBuffer::writeData(const char *data, qint64 maxSize) {
	auto d = reinterpret_cast<const float *>(data);
	const auto n = std::min(bufferSize, maxSize / static_cast<qint64>(sizeof(float)));
	buf.read(d, n);
	sampleCount += n;
	if (sampleCount > conf.buffer_size) {
		auto notes = detector.detect(buf);
		if (notes.size()) {
			emit noteDetected(notes.front());
		}
		sampleCount = 0;
	}
	return n * sizeof(float);
}

void Audio::play(int note) {
	init();
	if (note < 0) {
		buf.stop = true;
		buf.currentFadeSample = 0;
		return;
	}
	buf.frequency = 440 * std::pow(2, (note - 9) / 12.0);
	buf.stop = false;
	buf.currentFadeSample = 0;
}

void Audio::stop() {
	if (!ok) {
		return;
	}
	sink->stop();
	source->stop();
	sink.reset();
	source.reset();
	ok = false;
}

void Audio::init() {
	if (ok) {
		return;
	}

	connect(&recording, &PitchBuffer::noteDetected, this, &Audio::handleNote);

	auto fmt = QAudioFormat();
	fmt.setChannelCount(1);
	fmt.setSampleRate(buf.samplerate);
	fmt.setSampleFormat(QAudioFormat::SampleFormat::Float);

	sink = std::make_unique<QAudioSink>(fmt);
	sink->start(&buf);

	source = std::make_unique<QAudioSource>(fmt);
	source->start(&recording);
	ok = true;
}

void Audio::handleNote(const fftune::note_estimate &note) {
	detectedNote = note.note - fftune::MidiA4 + 9;
	emit detectedNoteChanged();
}
