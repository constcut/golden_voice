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


    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report

    f.open(QIODevice::ReadOnly);

    if (f.isOpen())
        qDebug() << "JSON File opened";

    QString fullString = f.readAll();

    //QJsonParseError *error = new QJsonParseError(); //TODO leak
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8()); //, error);

    auto root = doc.object();
    _events = root["events"].toArray();

    qDebug() << "Events total count: " << _events.size();

    _fullWidth = _events[_events.size() - 1].toObject()["endTime"].toDouble() * _zoomCoef;
}


int VisualReport::eventIdxOnClick(int mouseX, int mouseY)
{
    double second = mouseX / _zoomCoef;
    int i = 0;
    for (const auto& e: _events)
    {
        auto eObj = e.toObject();
        auto type = eObj["type"].toString();
        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();

        if (second >= start && second <= end && type == "word")
            return i; //yet we skip pauses

        ++i;
    }

    return -1;
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
    const auto fullHeight = height() ;

    double prevMean = 0.0;
    double prevMedian = 0.0;
    double prevXEnd = 0.0;

    double verticalZoom = 1.0;

    if (_type == VisualTypes::Amplitude)
        verticalZoom = 1.5;

    for (const auto& e: _events)
    {
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();
        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();

        int y = 0;
        QString word;

        double eventH = 0.0;

        if (type == "word")
        {

            word = eObj["word"].toString();
            QJsonObject value;

            if (_type == VisualTypes::Pitch) {
                value = eObj["stats"].toObject()["praat_pitch"].toObject();
            }
            else
            if (_type == VisualTypes::Amplitude) {
                value = eObj["stats"].toObject()["intensity"].toObject();
            }
            else
                qDebug() << "Unknow visualization type";

            //qDebug() << value["mean"].toDouble() << value["median"].toDouble()
            //         << value["min"].toDouble() << value["max"].toDouble()
            //          << value["SD"].toDouble()  ;

            auto x = start * _zoomCoef + 5;
            double w = (end - start) * _zoomCoef;
            y = value["min"].toDouble() * verticalZoom + 40;
            eventH = value["max"].toDouble() * verticalZoom + 40 - y;

            //qDebug() << "X="<<x<<" y="<<y<<" w="<<w<<" h="<<eventH << " and median " << value["median"].toDouble();

            painter->drawEllipse(x, fullHeight - value["min"].toDouble() * verticalZoom - 40, 4, 4);
            painter->drawEllipse(x, fullHeight - value["max"].toDouble() * verticalZoom - 40, 4, 4);

            painter->drawEllipse(x + w, fullHeight - value["min"].toDouble() * verticalZoom - 40, 4, 4);
            painter->drawEllipse(x + w, fullHeight - value["max"].toDouble() * verticalZoom - 40, 4, 4);

            auto pen = painter->pen();

            painter->setPen(QColor("green"));
            painter->drawEllipse(x, fullHeight - value["median"].toDouble() * verticalZoom - 40, 4, 4);
            painter->drawEllipse(x + w, fullHeight - value["median"].toDouble() * verticalZoom - 40, 4, 4);

            if (prevMedian != 0.0)
                painter->drawLine(prevXEnd, prevMedian, x, fullHeight - value["median"].toDouble() * verticalZoom - 40);

            prevMedian = fullHeight - value["median"].toDouble() * verticalZoom - 40;

            painter->setPen(QColor("blue"));
            painter->drawEllipse(x, fullHeight - value["mean"].toDouble() * verticalZoom - 40, 4, 4);
            painter->drawEllipse(x + w, fullHeight - value["mean"].toDouble() * verticalZoom - 40, 4, 4);

            if (prevMean != 0)
                painter->drawLine(prevXEnd, prevMean, x, fullHeight - value["mean"].toDouble() * verticalZoom - 40);

            prevMean = fullHeight - value["mean"].toDouble() * verticalZoom - 40;

            prevXEnd = x + w;


            QJsonArray sequence;
            if (_type == VisualTypes::Pitch)
            {
                sequence = eObj["praat_pitch"].toArray();
            }

            if (_type == VisualTypes::Amplitude)
                sequence = eObj["praat_intensity"].toArray();

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
                    //painter->drawEllipse(newX, newY, 2, 2);

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
        {
            painter->drawText(start * _zoomCoef + 5, fullHeight - y  + 20, word);
        }
    }

    //TODO отрисовка частоты

    //TODO отрисовка статистически значений
}
