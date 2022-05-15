#include "JsonReport.hpp"

#include <QDebug>
#include <QFile>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "VisualReport.hpp"

using namespace diaryth;



JsonReport::JsonReport(QObject *parent) : QObject(parent)
{
    _zoomCoef = 500.0;

    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report
    f.open(QIODevice::ReadOnly); //Проверка на открытие и реакция TODO

    QString fullString = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8()); //, error); //QJsonParseError *error = new QJsonParseError();

    auto root = doc.object();

    _events = root["events"].toArray();
    _fullWidth = _events[_events.size() - 1].toObject()["endTime"].toDouble() * _zoomCoef;

    _chunks = root["chunks"].toArray();
    _fullPraat = root["praat_report"].toObject(); //TODO + statistics of sequences
}


void JsonReport::updateAllVisualReports()
{
    for (auto visualPtr: _connectedVisuals)
        visualPtr->update();
}


void JsonReport::registerVisual(VisualReport* visual)
{
    _connectedVisuals.insert(visual);
}

void JsonReport::removeVisual(VisualReport* visual)
{
    _connectedVisuals.erase(visual);
}

void JsonReport::clearVisuals()
{
    _connectedVisuals.clear();
}


int JsonReport::getChunksCount()
{
    return _chunks.size();
}


void JsonReport::selectChunk(int idx)
{
    _selectedIdx.clear();

    for (int i = 0; i < _events.size(); ++i)
    {
        auto e = _events[i];
        int chunkId = e.toObject()["chunkId"].toInt();

        if (chunkId == idx)
            _selectedIdx.insert(i);
    }

    updateAllVisualReports();
}



int JsonReport::eventIdxOnClick(int mouseX, [[maybe_unused]] int mouseY)
{
    double second = mouseX / _zoomCoef;

    for (int i = 0; i < _events.size(); ++i)
    {
        auto e = _events[i];
        auto eObj = e.toObject();
        auto type = eObj["type"].toString();
        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();

        if (second >= start && second <= end && type == "word")
            return i; //yet we skip pauses
    }

    return -1;
}



QVariant JsonReport::getSelectedEvents()
{

    QList<QList<qreal>> fullList;

    for (int i = 0; i < _events.size(); ++i)
    {
        if (_selectedIdx.count(i) == 0)
            continue;

        auto e = _events[i];
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();

        if (type == "pause")
            continue;

        QList<qreal> eventLine;

        eventLine << i;

        if (eObj["info"].isObject())
        {
            auto praatInfo = eObj["info"].toObject();

            const auto& keys = praatInfo.keys();
            for (const auto& key: keys)
                eventLine << praatInfo[key].toDouble();
        }

        fullList << eventLine;
    }

    return QVariant::fromValue(fullList);
}



QString JsonReport::getWordByIdx(int idx)
{
    auto e = _events[idx];
    auto eObj = e.toObject();

    QString type = eObj["type"].toString();

    if (type == "pause")
        return "";

    return eObj["word"].toString();
}


QList<qreal> JsonReport::getChunkInfo(int idx) //TODO ещё одну функцию для общей статистики
{
    QList<qreal> chunkLine;

    auto chunkPraatInfo = _chunks[idx].toObject()["praat_report"].toObject();

    const auto& keys = chunkPraatInfo.keys();
    for (const auto& key: keys)
        chunkLine << chunkPraatInfo[key].toDouble();

    return chunkLine;
}


QList<qreal> JsonReport::getFullInfo()
{
    QList<qreal> fullFileLine;

    const auto& keys = _fullPraat.keys();
    for (const auto& key: keys)
        fullFileLine << _fullPraat[key].toDouble();

    return fullFileLine;
}


QStringList JsonReport::getPraatFieldsNames()
{
    QStringList fullFileLine;

    for (const auto& key: _fullPraat.keys())
        fullFileLine << key;

    return fullFileLine;
}


void JsonReport::selectEvent(int idx)
{
    if (_selectedIdx.count(idx))
        _selectedIdx.erase(idx);
    else
        _selectedIdx.insert(idx);

    updateAllVisualReports();
}


