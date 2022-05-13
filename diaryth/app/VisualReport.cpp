#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QPainter>


using namespace diaryth;

//TODO конструктор

//TODO precalc full width функция для установки размера скрола после загрузки файла


void VisualReport::paint(QPainter* painter)
{
    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report

    f.open(QIODevice::ReadOnly);

    if (f.isOpen())
        qDebug() << "JSON File opened";

    QString fullString = f.readAll();

    //QJsonParseError *error = new QJsonParseError(); //TODO leak
    QJsonDocument doc = QJsonDocument::fromJson(fullString.toUtf8()); //, error);

    auto root = doc.object();
    QJsonArray events = root["events"].toArray();

    qDebug() << "Events total count: " << events.size();

    for (const auto& e: events)
    {
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();
        double start = eObj["startTime"].toDouble();
        double end = eObj["endTime"].toDouble();



        qDebug() << "Type " << type << " start " << start << " end " << end;

        int y = 20;
        QString word;

        if (type == "word") {
            y += 50;
            word = eObj["word"].toString();
        }

        const double zoomCoef = 200.0;
        const double rectWidth = 20;

        painter->drawRect(start * zoomCoef + 5, y, (end - start) * zoomCoef, rectWidth);

        if (type == "word")
            painter->drawText(start * zoomCoef + 5, y + rectWidth - 3, word);
    }

    //TODO отрисовка частоты

    //TODO отрисовка статистически значений
}
