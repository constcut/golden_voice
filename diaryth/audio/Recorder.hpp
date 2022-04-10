#ifndef RECORDER_H
#define RECORDER_H

#include <memory>

#include <QObject>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QMediaPlayer> //Move it later


namespace diaryth {

    class SQLBase;

    class Recorder : public QObject
    {
        Q_OBJECT

    public:
        Recorder(SQLBase& database);

        Q_INVOKABLE void start();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void cancel();

        Q_INVOKABLE QString lastFilename();

        Q_INVOKABLE QStringList inputDevices();
        Q_INVOKABLE QStringList audioCodecs();
        Q_INVOKABLE QStringList fileContainers();
        Q_INVOKABLE QStringList sampleRates();

        Q_INVOKABLE QString getInputDevice();
        Q_INVOKABLE QString getAudioCodec();
        Q_INVOKABLE QString getFileContainer();
        Q_INVOKABLE int getSampleRate();

        Q_INVOKABLE void setInputDevice(QString device);
        Q_INVOKABLE void setAudioCodec(QString codec);
        Q_INVOKABLE void setFileContainer(QString container);
        Q_INVOKABLE void setSampleRate(QString sampleRate);


        Q_INVOKABLE void playFile(QString date, int localId); //Move it later

    public slots:
        void processBuffer(const QAudioBuffer& buffer);

        void playerPositionChanged(qint64 ms); //Move it later (and replace with probe)

    signals:
        void timeUpdate(quint64 ms);
        void dbsUpdate(qreal dbs);

    private:

        std::unique_ptr<QAudioProbe> _audioProbe;
        std::unique_ptr<QAudioRecorder> _audioRecorder;

        std::unique_ptr<QMediaPlayer> _audioPlayer;

        quint64 _durationMicroSeconds;
        QString _lastDateString;
        QString _lastTimeString;

        QString _defaultInput;
        QString _defaultCodec;
        QString _defaultContainer;
        int _defaultSampleRate;

        SQLBase& _database;

    };


}

#endif // RECORDER_H
