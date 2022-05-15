#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QVariantList>


using namespace diaryth;


VisualReport::VisualReport()
{
    _parentReport = nullptr;

    _praatFields["Jitter (local)"] = { "green", 20 };
    _praatFields["Shimmer (local)"] = { "blue", 10 };
}


QVariantList VisualReport::getPraatFields()
{
    QVariantList allFields;

    for (const auto& [fieldName, fieldData]: _praatFields)
    {
        QStringList fieldLine;
        fieldLine << fieldName << fieldData.color << QString::number(fieldData.yCoef);
        allFields << QVariant::fromValue(fieldLine);
    }

    qDebug() << "Returning praat fields size: " << allFields.size();

    return allFields;
}


void VisualReport::paint(QPainter* painter)
{
    if (_parentReport == nullptr)
        return;

    ReportPrevStats prevStats;
    PraatPrevStats prevPraats;

    painter->drawRect(2, 2, width() - 4, height() - 4); // Обводка Для удобства тестирования

    const auto& events = _parentReport->getEvents();

    for (int i = 0; i < events.size(); ++i)
    {
        auto e = events[i];
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

    const double zoomCoef = _parentReport->getZoom();
    const auto& selectedIdx = _parentReport->getSelectedIdx();

    if (type == "word")
    {
        auto x = start * zoomCoef + 5;
        double w = (end - start) * zoomCoef;

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
            auto color = QColor(fieldDisplayInfo.color);

            if (selectedIdx.count(idx)) //Возможно это лишнее
                color = color.lighter();

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

    const double zoomCoef = _parentReport->getZoom();
    const auto& selectedIdx = _parentReport->getSelectedIdx();

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

        auto x = start * zoomCoef + 5;
        double w = (end - start) * zoomCoef;
        y = value["min"].toDouble() * verticalZoom + 40;
        eventH = value["max"].toDouble() * verticalZoom + 40 - y;

        if (selectedIdx.count(idx))
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
        painter->drawText(start * zoomCoef + 5, fullHeight - y  + 20, word);
}
