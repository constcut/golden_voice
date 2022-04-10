#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <memory>

#include <QAudioOutput>
#include <QAudioInput>
#include <QTimer>

#include "AudioReceiver.hpp"
#include "AudioSpeaker.hpp"


namespace diaryth {


    class AudioHandler : public QObject
    {
        Q_OBJECT

    public:
        AudioHandler();

        Q_INVOKABLE void startRecord();
        Q_INVOKABLE void stopRecord();

        Q_INVOKABLE void startPlayback();
        Q_INVOKABLE void stopPlayback();
        Q_INVOKABLE void resetBufer();

        Q_INVOKABLE void loadFile(const QString filename);
        Q_INVOKABLE void saveFile(const QString filename) const;
        Q_INVOKABLE void loadWavFile(const QString filename);
        Q_INVOKABLE void saveWavFile(const QString filename) const;
        Q_INVOKABLE void loadOnlyWindow(const QString filename, const quint64 position, const quint64 window);
        Q_INVOKABLE void loadWindowPCM(const QByteArray window);


        Q_INVOKABLE int getSampleRate() const {
            return _commonFormat.sampleRate();
        }
        Q_INVOKABLE int getBitRate() const {
            return _commonFormat.sampleSize();
        }

        Q_INVOKABLE void setSampleRate(const int newSampleRate);
        Q_INVOKABLE void requestPermission() const;

        Q_INVOKABLE QStringList getRecords() const;
        Q_INVOKABLE void saveRecordTimstamp() const;
        Q_INVOKABLE void deleteRecord(const QString filename) const;
        Q_INVOKABLE void renameRecord(const QString filename, const QString newFilename) const;

        void requestStopRecord();
        void requestStopPlayback();


    private:

        void initAudioHandler();

        void initRecorder();
        void initPlayer();

        std::unique_ptr<QAudioInput> _audioInput;
        std::unique_ptr<AudioReceiver> _audioReceiver;

        std::unique_ptr<QAudioOutput> _audioOutput;
        std::unique_ptr<AudioSpeaker> _audioPlayer;
        QTimer _audioStopRequestTimer;

        QByteArray _commonBufer;
        QAudioFormat _commonFormat;

        size_t _prevBufferSize;

        bool _isPlaying = false;
        bool _isRecording = false;
    };

}


#endif // AUDIOHANDLER_H
