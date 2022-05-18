#include "JsonReport.hpp"

#include <QDebug>
#include <QFile>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

#include "VisualReport.hpp"

using namespace diaryth;



JsonReport::JsonReport(QObject *parent) : QObject(parent)
{
    _zoomCoef = 500.0;
    _configFilename = "config.json";

    //TODO вынести отдельно функцию открытия файла
    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report
    f.open(QIODevice::ReadOnly); //Проверка на открытие и реакция TODO

    QString fullString = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8()); //, error); //QJsonParseError *error = new QJsonParseError();

    auto root = doc.object();

    _events = root["events"].toArray();
    _fullWidth = _events[_events.size() - 1].toObject()["endTime"].toDouble() * _zoomCoef;

    _chunks = root["chunks"].toArray();
    _fullPraat = root["info"].toObject(); //TODO + statistics of sequences
}


void JsonReport::saveLocalConfig()
{
    //TODO insure all registered in proper sequence - there is a match between type and height

    QJsonObject root;

    root["zoom"] = _zoomCoef;

    QJsonArray visualReports;

    for (auto pReport: _connectedVisuals)
    {
        QJsonObject reportObject;

        reportObject["type"] = pReport->getType();
        reportObject["height"] = pReport->height();

        auto praatFields = pReport->getPraatFields();
        QJsonArray praatFieldsArr;

        for (int i = 0; i < praatFields.size(); ++i)
        {
            auto line = praatFields[i].toStringList();

            QJsonObject singleField;
            singleField["name"] = line[0];
            singleField["color"] = line[1];
            singleField["y_coef"] = line[2].toDouble();

            praatFieldsArr.append(singleField);
        }

        reportObject["praat_fields"] = praatFieldsArr;

        auto reportFields = pReport->getReportFields();
        QJsonArray reportFieldsArr;

        for (int i = 0; i < reportFields.size(); ++i)
        {
            auto line = reportFields[i].toStringList();

            QJsonObject singleField;
            singleField["name"] = line[0];
            singleField["color"] = line[1];
            singleField["y_coef"] = line[2].toDouble();

            reportFieldsArr.append(singleField);
        }

        reportObject["report_fields"] = reportFieldsArr;

        visualReports.append(reportObject);
    }

    root["visual_reports"] = visualReports;

    auto doc = QJsonDocument(root);
    auto bytes = doc.toJson();

    QFile f(_configFilename);
    f.open(QIODevice::WriteOnly);
    f.write(bytes);
    f.close();
}


QList<int> JsonReport::loadLocalConfig()
{
    QFile f(_configFilename);
    f.open(QIODevice::ReadOnly);

    QString fullString = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8());

    auto root = doc.object();
    _zoomCoef = root["zoom"].toDouble();

    auto visualReports = root["visual_reports"].toArray();

    QList<int> heights;

    for (int i = 0; i < visualReports.size(); ++i)
    {
        auto reportObject = visualReports[i].toObject();

        if (_connectedVisuals.size() <= static_cast<size_t>(i))
            break;

        auto pReport = _connectedVisuals[i];

        int type = reportObject["type"].toInt();
        int height = reportObject["height"].toInt(); //Нужно вынести наружу

        heights.append(height);

        auto praatFields = reportObject["praat_fields"].toArray();
        auto reportFields = reportObject["report_fields"].toArray();

        pReport->clearPraatFields();

        for (int j = 0; j < praatFields.size(); ++j)
        {
            auto singlePraatField = praatFields[j].toObject();

            auto name = singlePraatField["name"].toString();
            auto color = singlePraatField["color"].toString();
            auto yCoef = singlePraatField["y_coef"].toDouble();

            pReport->addPraatField(name, color, yCoef);
        }

        for (int j = 0; j < reportFields.size(); ++j)
        {
            auto singleReportField = reportFields[j].toObject();

            auto name = singleReportField["name"].toString();
            auto color = singleReportField["color"].toString();
            auto yCoef = singleReportField["y_coef"].toDouble();

            pReport->addReportField(name, color, yCoef);
        }

        pReport->setType(type);
    }

    updateAllVisualReports();

    return heights;
}


double JsonReport::getFullWidth()
{
    _fullWidth = _events[_events.size() - 1].toObject()["endTime"].toDouble() * _zoomCoef;
    return _fullWidth + 20; //TODO наверное перестать хранить в классе
}

void JsonReport::updateAllVisualReports()
{
    for (auto visualPtr: _connectedVisuals)
        visualPtr->update();
}


void JsonReport::registerVisual(VisualReport* visual)
{
    auto findResult = std::find(_connectedVisuals.begin(), _connectedVisuals.end(), visual);

    if (findResult == _connectedVisuals.end())
        _connectedVisuals.push_back(visual);
}

void JsonReport::removeVisual(VisualReport* visual)
{
    auto findResult = std::find(_connectedVisuals.begin(), _connectedVisuals.end(), visual);

    if (findResult != _connectedVisuals.end())
        _connectedVisuals.erase(findResult);
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

    _lastSelectedChunk = idx;
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

    auto chunkPraatInfo = _chunks[idx].toObject()["info"].toObject();

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


