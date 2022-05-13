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

}

//TODO precalc full width функция для установки размера скрола после загрузки файла


void VisualReport::paint(QPainter* painter)
{

    const double zoomCoef = 200.0;
    const auto fullHeight = height() ;

    for (const auto& e: _events)
    {
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();
        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();

        int y = 0;
        QString word;

        if (type == "word")
        {

            word = eObj["word"].toString();
            QJsonObject value;

            if (_type == VisualTypes::Pitch) {
                value = eObj["stats"].toObject()["praat_pitch"].toObject();
                qDebug() << "Pitch object obtained";
            }
            else
            if (_type == VisualTypes::Amplitude) {
                value = eObj["stats"].toObject()["intensity"].toObject();
                qDebug() << "Amplitude object obtained";
            }
            else
                qDebug() << "WHAT";

            qDebug() << value["mean"].toDouble() << value["median"].toDouble()
                     << value["min"].toDouble() << value["max"].toDouble()
                     << value["SD"].toDouble()  ;

            auto x = start * zoomCoef + 5;
            auto w = (end - start) * zoomCoef;
            y = value["min"].toDouble() + 40;
            auto h = value["max"].toDouble() + 40 - y;

            qDebug() << "X="<<x<<" y="<<y<<" w="<<w<<" h="<<h << " and median " << value["median"].toDouble();

            painter->drawEllipse(x, fullHeight - value["min"].toDouble() - 40, 2, 2);
            painter->drawEllipse(x, fullHeight - value["max"].toDouble() - 40, 2, 2);

            auto pen = painter->pen();

            painter->setPen(QColor("green"));
            painter->drawEllipse(x, fullHeight - value["median"].toDouble() - 40, 2, 2);

            painter->setPen(QColor("blue"));
            painter->drawEllipse(x, fullHeight - value["mean"].toDouble() - 40, 2, 2);
            //TODO sd

            painter->setPen(pen);
            painter->drawRect(x, fullHeight - y, w, -h);
        }
        else
            ;//painter->drawRect(start * zoomCoef + 5, fullHeight - y - 20, (end - start) * zoomCoef, rectH);

        const double zoomCoef = 200.0;
        const double rectWidth = 20;


        if (type == "word")
        {
            //Debug freq value
            //painter->drawText(start * zoomCoef + 5, fullHeight - y + rectWidth*2 - 3, QString::number(y));
            painter->drawText(start * zoomCoef + 5, fullHeight - y + rectWidth - 3, word);
        }
    }

    //TODO отрисовка частоты

    //TODO отрисовка статистически значений
}
