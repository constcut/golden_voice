#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QPainter>


using namespace diaryth;

//TODO конструктор

VisualReport::VisualReport()
{
    _zoomCoef = 500.0;
    _type = VisualTypes::TypeNotSet;

    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report
    f.open(QIODevice::ReadOnly); //Проверка на открытие и реакция TODO

    QString fullString = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8()); //, error); //QJsonParseError *error = new QJsonParseError();

    auto root = doc.object();

    _events = root["events"].toArray();
    _fullWidth = _events[_events.size() - 1].toObject()["endTime"].toDouble() * _zoomCoef;

    _chunks = root["chunks"].toArray();
    _fullPraat = root["praat_report"].toObject(); //TODO + statistics of sequences

    _praatFields["Jitter (local)"] = { QColor("green"), 20 };
    _praatFields["Shimmer (local)"] = { QColor("blue"), 10 };
}


int VisualReport::getChunksCount()
{
    return _chunks.size();
}


void VisualReport::selectChunk(int idx)
{
    _selectedIdx.clear();

    for (int i = 0; i < _events.size(); ++i)
    {
        auto e = _events[i];
        int chunkId = e.toObject()["chunkId"].toInt();

        if (chunkId == idx)
            _selectedIdx.insert(i);
    }

    update();
}



int VisualReport::eventIdxOnClick(int mouseX, [[maybe_unused]] int mouseY)
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



QVariantList VisualReport::getSelectedEvents()
{

    QVariantList fullList;

    for (int i = 0; i < _events.size(); ++i)
    {
        if (_selectedIdx.count(i) == 0)
            continue;

        auto e = _events[i];
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();

        if (type == "pause")
            continue;

        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();
        QString word = eObj["word"].toString();

        QStringList eventLine; //yet best way to send matrix of "variant"
        eventLine << word << QString::number(start) << QString::number(end); //TODO more

        //Another way is to use first name then QList<qreal> или может даже QMap<QString, qreal>

        fullList.append(eventLine);
    }

    return fullList;
}


QList<qreal> VisualReport::getChunkInfo(int idx) //TODO ещё одну функцию для общей статистики
{
    QList<qreal> chunkLine;

    auto chunkPraatInfo = _chunks[idx].toObject()["praat_report"].toObject();

    const auto& keys = chunkPraatInfo.keys();
    for (const auto& key: keys)
        chunkLine << chunkPraatInfo[key].toDouble();

    return chunkLine;
}


QList<qreal> VisualReport::getFullInfo()
{
    QList<qreal> fullFileLine;

    const auto& keys = _fullPraat.keys();
    for (const auto& key: keys)
        fullFileLine << _fullPraat[key].toDouble();

    return fullFileLine;
}


QStringList VisualReport::getPraatFieldsNames()
{
    QStringList fullFileLine;

    for (const auto& key: _fullPraat.keys())
        fullFileLine << key;

    return fullFileLine;
}




void VisualReport::selectEvent(int idx)
{
    if (_selectedIdx.count(idx))
        _selectedIdx.erase(idx);
    else
        _selectedIdx.insert(idx);

    update();
}




void VisualReport::paint(QPainter* painter)
{

    ReportPrevStats prevStats;
    PraatPrevStats prevPraats;

    painter->drawRect(2, 2, width() - 4, height() - 4); // Для удобства тестирования

    for (int i = 0; i < _events.size(); ++i)
    {
        auto e = _events[i];
        auto event = e.toObject();

        if (_type == VisualTypes::Pitch || _type == VisualTypes::Amplitude)
            paintSequenceType(painter, event, i, prevStats);

        if (_type == VisualTypes::PraatInfo)
            paintPraatInfo(painter, event, i, prevPraats);
    }
}


void VisualReport::paintPraatInfo(QPainter* painter, QJsonObject& event,
                                  int idx, PraatPrevStats &prevStats)
{

    auto type = event["type"].toString(); //Возвращать как structure binding?
    double start = event["startTime"].toDouble();
    double end = event["endTime"].toDouble();

    if (type == "word")
    {
        auto x = start * _zoomCoef + 5;
        double w = (end - start) * _zoomCoef;

        auto paintFun = [&](QString infoName, QColor color, double yCoef) //TODO Возможно разумнее вынести в отдельную фукцию с кучей аргументов
        {
            double value = 0.0;

            if (event["info"].isObject())
            {
                auto info = event["info"].toObject();
                value = info[infoName].toDouble();
            }

            int y = height() - value * yCoef;

            painter->setPen(color);
            painter->drawLine(x, y, x + w, y);

            if (prevStats.prevXEnd != 0.0)
            {
                painter->drawLine(prevStats.prevXEnd,
                                  prevStats.prevValues[infoName], x, y);
            }

            prevStats.prevValues[infoName] = y;
        };

        for (const auto& [name, fieldDisplayInfo]: _praatFields)
        {
            auto color = fieldDisplayInfo.color;

            if (_selectedIdx.count(idx)) //Возможно это лишнее
                color = color.darker();

            paintFun(name, color, fieldDisplayInfo.yCoef);
        }

        prevStats.prevXEnd = x + w;
    }
}



void VisualReport::paintSequenceType(QPainter* painter, QJsonObject& event,
                                 int idx, ReportPrevStats& prevStats)
{
    const auto fullHeight = height() ;

    double verticalZoom = 1.0;

    if (_type == VisualTypes::Amplitude)
        verticalZoom = 1.5;

    auto type = event["type"].toString(); //Возвращать как structure binding?
    double start = event["startTime"].toDouble();
    double end = event["endTime"].toDouble();

    int y = 0;
    QString word;

    double eventH = 0.0;

    if (type == "word")
    {

        word = event["word"].toString();

        QJsonObject value;

        if (_type == VisualTypes::Pitch) {
            value = event["stats"].toObject()["praat_pitch"].toObject();
        }
        else
        if (_type == VisualTypes::Amplitude) {
            value = event["stats"].toObject()["intensity"].toObject();
        }
        else
            qDebug() << "Unknow visualization type";

        auto x = start * _zoomCoef + 5;
        double w = (end - start) * _zoomCoef;
        y = value["min"].toDouble() * verticalZoom + 40;
        eventH = value["max"].toDouble() * verticalZoom + 40 - y;

        if (_selectedIdx.count(idx))
            painter->fillRect(x, fullHeight - y, w, - eventH, QBrush(QColor("darkgray")));
        else
            painter->fillRect(x, fullHeight - y, w, - eventH, QBrush(QColor(240, 240, 240)));

        painter->drawEllipse(x, fullHeight - value["min"].toDouble() * verticalZoom - 40, 4, 4);
        painter->drawEllipse(x, fullHeight - value["max"].toDouble() * verticalZoom - 40, 4, 4);

        painter->drawEllipse(x + w, fullHeight - value["min"].toDouble() * verticalZoom - 40, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["max"].toDouble() * verticalZoom - 40, 4, 4);

        auto pen = painter->pen();

        painter->setPen(QColor("green"));
        painter->drawEllipse(x, fullHeight - value["median"].toDouble() * verticalZoom - 40, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["median"].toDouble() * verticalZoom - 40, 4, 4);

        if (prevStats.prevMedian != 0.0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMedian, x,
                              fullHeight - value["median"].toDouble() * verticalZoom - 40);

        prevStats.prevMedian = fullHeight - value["median"].toDouble() * verticalZoom - 40;

        painter->setPen(QColor("blue"));
        painter->drawEllipse(x, fullHeight - value["mean"].toDouble() * verticalZoom - 40, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["mean"].toDouble() * verticalZoom - 40, 4, 4);

        if (prevStats.prevMean != 0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMean, x,
                              fullHeight - value["mean"].toDouble() * verticalZoom - 40);

        prevStats.prevMean = fullHeight - value["mean"].toDouble() * verticalZoom - 40;
        prevStats.prevXEnd = x + w;


        QJsonArray sequence;
        if (_type == VisualTypes::Pitch)
            sequence = event["praat_pitch"].toArray();

        if (_type == VisualTypes::Amplitude)
            sequence = event["praat_intensity"].toArray();

        painter->setPen(QColor("red"));

        double pixelPerSample = w / sequence.size();

        double prevX = 0.0;
        double prevY = 0.0;

        for (int i = 0; i < sequence.size(); ++i)
        {
            double pY = sequence[i].toDouble() * verticalZoom;

            double newX = x + i * pixelPerSample;
            double newY = fullHeight - pY - 40;

            if (pY != 0.0 && prevY != fullHeight - 40)
            {
                if (prevX != 0.0 || prevY != 0.0)
                    painter->drawLine(prevX, prevY, newX, newY);
            }

            prevX = x + i * pixelPerSample;
            prevY = fullHeight - pY - 40;
        }

        painter->setPen(QColor("gray"));
        painter->drawRect(x, fullHeight - y, w, - eventH);
        painter->setPen(pen);

    }
    else
        ;//painter->drawRect(start * zoomCoef + 5, fullHeight - y - 20, (end - start) * zoomCoef, rectH);


    if (type == "word")
        painter->drawText(start * _zoomCoef + 5, fullHeight - y  + 20, word);
}
