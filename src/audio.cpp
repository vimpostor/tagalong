#include "audio.hpp"

AudioBuffer::AudioBuffer() {
	open(QIODeviceBase::ReadOnly);
}

void AudioBuffer::toggle(int note) {
	auto it = std::ranges::find_if(notes, [=](const auto &n) { return n.note == note; });
	if (it == notes.end()) {
		// add new note
		NotePlayback n {note, fftune::midi_to_freq(note), false, 0};
		notes.push_back(n);
	} else if (it->stop) {
		// fade in previously stopped note
		it->stop = false;
	} else {
		// fade out
		it->stop = true;
		it->currentFadeSample = 0;
	}
}

qint64 AudioBuffer::readData(char *data, qint64 maxSize) {
	const auto n = currentSample + (maxSize ? maxSize / sizeof(float) : bufferSize);
	auto d = reinterpret_cast<float *>(data);
	for (; currentSample < n; ++currentSample) {
		float sample = 0;
		for (auto &n : notes) {
			float tone = 0;
			float volume = static_cast<float>(n.currentFadeSample) / (fadeSamples - 1);
			if (n.stop) {
				volume = 1 - volume;
			}
			if (n.currentFadeSample < fadeSamples - 1) {
				n.currentFadeSample++;
			}
			const constexpr int overtones = 8;
			for (int overtone = 1; overtone <= 8; ++overtone) {
				const constexpr float dampeningConstant = 0.2;
				const float dampening = 1.f - (static_cast<float>(overtone - 1) / static_cast<float>(overtones - 1) * (1.f - dampeningConstant));
				tone += dampening * std::sin(2 * M_PI * overtone * n.freq * currentSample / samplerate);
			}
			tone *= volume / (overtones - 1);
			sample += tone;
		}
		*d = sample;
		++d;
	}

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

void Audio::toggle(int note) {
	init();
	buf.toggle(note);
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
	detectedNote = note.note;
	emit detectedNoteChanged();
}
