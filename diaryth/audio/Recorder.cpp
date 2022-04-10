#include "Recorder.hpp"

#include <QAudioEncoderSettings>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QDebug>

#include "diary/SQLBase.hpp"
#include "audio/features/FeatureExtractor.hpp"


using namespace diaryth;


Recorder::Recorder(SQLBase &database) : _database(database)
{
    _audioProbe = std::make_unique<QAudioProbe>();
    _audioRecorder = std::make_unique<QAudioRecorder>();

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setQuality(QMultimedia::HighQuality);

    _audioRecorder->setEncodingSettings(audioSettings); //We can also set volume here
    _audioProbe->setSource(_audioRecorder.get());

    connect(_audioProbe.get(), SIGNAL(audioBufferProbed(QAudioBuffer)),
            this, SLOT(processBuffer(QAudioBuffer)));

    _defaultInput = getInputDevice();
    _defaultCodec = getAudioCodec();
    _defaultContainer = getFileContainer();
    _defaultSampleRate = getSampleRate();

    _audioPlayer = std::make_unique<QMediaPlayer>();
    connect(_audioPlayer.get(), SIGNAL(positionChanged(qint64)),
            this, SLOT(playerPositionChanged(qint64)));
}



void Recorder::start()
{
    _lastDateString = QDate::currentDate().toString("yyyy-MM-dd");
    QString directory = QDir::currentPath() + "/recorder/" + _lastDateString + "/";

    QDir dir;
    if (dir.exists(directory) == false) { //Лучше создавать на старте приложения, а потом подчищать прошлые дни, тк создание даёт задержку
        if (dir.mkdir(directory) == false)
            qDebug() << "Failed to create recorder directory: " << directory;
    }

    _lastTimeString = QTime::currentTime().toString("HH:mm:ss");
    const int nextRecordId = _database.getRecordsMaxLocalId(_lastDateString) + 1;
    QString recordName = QString::number(nextRecordId);

    _durationMicroSeconds = 0;

    _audioRecorder->setOutputLocation(QUrl::fromLocalFile(directory + recordName));
    _audioRecorder->record();
}


QString Recorder::lastFilename() {
    return _audioRecorder->actualLocation().toString();
}


void Recorder::pause() {
    _audioRecorder->pause();
}


void Recorder::stop() {
    _audioRecorder->stop();

    const int nextRecordId = _database.getRecordsMaxLocalId(_lastDateString) + 1;
    QString simpleName = "Record #" + QString::number(nextRecordId);
    _database.addAudioRecord(_lastDateString, _lastTimeString, nextRecordId,
                             simpleName, _durationMicroSeconds / 1000);
}


void Recorder::cancel()
{
    _audioRecorder->stop();

    QString fn = lastFilename().mid(8); //Remove file:///
    QFile file(fn);

    if (file.remove() == false)
        qDebug() << "Failed to remove: " << fn;
}


void Recorder::processBuffer(const QAudioBuffer& buffer)
{
    _durationMicroSeconds += buffer.duration();

    //Возможно стоит уменьшить частоту обновления, и оптарвлять сигналы не каждый раз

    emit timeUpdate(_durationMicroSeconds / 1000);

    const int16_t* samples = reinterpret_cast<const int16_t*>(buffer.data());
    const int bitsInByte = 8;
    const int length = buffer.byteCount() / ( buffer.format().sampleSize() / bitsInByte);

    const double dbs = calc_dB(samples, length);
    emit dbsUpdate(dbs);
}



QStringList Recorder::inputDevices()
{
    QStringList list = {"Default"};
    list.append(_audioRecorder->audioInputs());
    return list;
}


QStringList Recorder::audioCodecs()
{
    QStringList list = {"Default"};
    list.append(_audioRecorder->supportedAudioCodecs());
    return list;
}


QStringList Recorder::fileContainers()
{
    QStringList list = {"Default"};
    list.append(_audioRecorder->supportedContainers());
    return list;
}


QStringList Recorder::sampleRates()
{
    QStringList list = {"Default"};

    auto supportedSampleRates = _audioRecorder->supportedAudioSampleRates();

    for (int sampleRate: supportedSampleRates)
       list.append(QString::number(sampleRate));

    return list;
}


QString Recorder::getInputDevice() {
    return _audioRecorder->audioInput();
}


QString Recorder::getAudioCodec() {
    return _audioRecorder->audioSettings().codec();
}


QString Recorder::getFileContainer() {
    return _audioRecorder->containerFormat();
}


int Recorder::getSampleRate() {
    return _audioRecorder->audioSettings().sampleRate();
}


void Recorder::setInputDevice(QString device)
{
    if (device == "Default")
        _audioRecorder->setAudioInput(_defaultInput);
    else
        _audioRecorder->setAudioInput(device);
}


void Recorder::setAudioCodec(QString codec)
{
    auto audioSettings = _audioRecorder->audioSettings();

    if (codec == "Default")
        audioSettings.setCodec(_defaultCodec);
    else
        audioSettings.setCodec(codec);

    _audioRecorder->setAudioSettings(audioSettings);
}


void Recorder::setFileContainer(QString container)
{
    if (container == "Default")
        _audioRecorder->setContainerFormat(_defaultContainer);
    else
        _audioRecorder->setContainerFormat(container);
}


void Recorder::setSampleRate(QString sampleRate)
{
    auto audioSettings = _audioRecorder->audioSettings();

    if (sampleRate == "Default")
        audioSettings.setSampleRate(_defaultSampleRate);
    else
        audioSettings.setSampleRate(sampleRate.toInt());

    _audioRecorder->setAudioSettings(audioSettings);
}


void Recorder::playFile(QString date, int localId)
{
    //? Stop if playing?

    QString filePath = QDir::currentPath() + "/recorder/" + date + "/"
            + QString::number(localId) + ".wav";

    _audioPlayer->setMedia(QUrl::fromLocalFile(filePath));
    _audioPlayer->setVolume(50);
    _audioPlayer->play();
}


void Recorder::playerPositionChanged(qint64 ms) {
    emit timeUpdate(ms); //Far from perfect try audio probe?
}
