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
}


void JsonReport::loadFromFile(QString filename)
{
    QFile f(filename);
    f.open(QIODevice::ReadOnly);

    if (f.isOpen() == false)
    {
        qDebug() << "Failed to open JR file " << filename;
        return;
    }

    _lastFilename = filename;

    QString fullString = f.readAll();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "JR json parse error " << parseError.error << " : " << parseError.errorString();
        return;
    }

    _root = doc.object();
    _events = _root["events"].toArray();
    _chunks = _root["chunks"].toArray();
    _fullPraat = _root["info"].toObject();

    qDebug() << "Loaded JSON report: events " << _events.size() << " chunks " << _chunks.size() << " info " << _fullPraat.size();
}


void JsonReport::saveLocalConfig() const
{
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

        pReport->clearReportFields();

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


double JsonReport::getFullWidth() const
{
    auto fullWidth = _events[_events.size() - 1].toObject()["end"].toDouble() * _zoomCoef + 20;
    //qDebug() << "Full width: " << fullWidth;
    return fullWidth;
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


int JsonReport::getChunksCount() const
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



int JsonReport::eventIdxOnClick(int mouseX, [[maybe_unused]] int mouseY) const
{
    double second = mouseX / _zoomCoef;

    for (int i = 0; i < _events.size(); ++i)
    {
        auto e = _events[i];
        auto eObj = e.toObject();
        auto type = eObj["type"].toString();
        double start = eObj["start"].toDouble();
        double end = eObj["end"].toDouble();

        if (second >= start && second <= end && type == "word")
            return i; //yet we skip pauses
    }

    return -1;
}



QVariant JsonReport::getSelectedEvents() const
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



QString JsonReport::getWordByIdx(int idx) const
{
    auto e = _events[idx];
    auto eObj = e.toObject();

    QString type = eObj["type"].toString();

    if (type == "pause")
        return "";

    return eObj["word"].toString();
}


QList<qreal> JsonReport::getChunkInfo(int idx) const
{
    QList<qreal> chunkLine;

    auto chunkPraatInfo = _chunks[idx].toObject()["info"].toObject();

    const auto& keys = chunkPraatInfo.keys();
    for (const auto& key: keys)
        chunkLine << chunkPraatInfo[key].toDouble();

    return chunkLine;
}


QList<qreal> JsonReport::getFullInfo() const
{
    QList<qreal> fullFileLine;

    const auto& keys = _fullPraat.keys();
    for (const auto& key: keys)
        fullFileLine << _fullPraat[key].toDouble();

    return fullFileLine;
}


QStringList JsonReport::getPraatFieldsNames() const
{
    QStringList fullFileLine;

    const auto& keys = _fullPraat.keys();

    for (const auto& key: keys)
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



QString JsonReport::getSelectedEventsString() const
{
    QString selectedString = "[";

    for (auto idx: _selectedIdx)
        selectedString += QString::number(idx) + ", ";

    if (_selectedIdx.empty() == false)
        selectedString.chop(2);

    selectedString += "]";

    return selectedString;
}


QStringList JsonReport::getSelectedEventsMarkup() const
{
    QStringList tagsAndComments;

    if (_root.contains("markup"))
    {
        auto selectedEventsString = getSelectedEventsString();
        const auto& markupObject = _root["markup"].toObject();

        if (markupObject.contains(selectedEventsString))
        {
            const auto& singleMarkup = markupObject[selectedEventsString].toObject();
            tagsAndComments << singleMarkup["tags"].toString() << singleMarkup["comments"].toString();
        }
    }

    if (tagsAndComments.empty())
        tagsAndComments << "" << "";

    return tagsAndComments;
}


void JsonReport::saveSelectedEventsMarkup(QString tags, QString comments)
{
    QJsonObject newMarkup;

    newMarkup["tags"] = tags;
    newMarkup["comments"] = comments;

    auto selectedEventsString = getSelectedEventsString();

    if (_root.contains("markup"))
    {
        auto rootMarkupObject = _root["markup"].toObject();
        rootMarkupObject[selectedEventsString] = newMarkup;
        _root["markup"] = rootMarkupObject;
    }
    else
    {
        QJsonObject rootMarkupObject;
        rootMarkupObject[selectedEventsString] = newMarkup;
        _root["markup"] = rootMarkupObject;
    }

    QJsonDocument newJson(_root);
    auto bytes = newJson.toJson();

    QFile f(_lastFilename);
    f.open(QIODevice::WriteOnly);
    f.write(bytes);
}
