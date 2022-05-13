#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>


using namespace diaryth;



void VisualReport::paint(QPainter* painter)
{
    QFile f = QFile("C:/Users/constcut/Desktop/local/full_report.json"); //full report

    f.open(QIODevice::ReadOnly);

    if (f.isOpen())
        qDebug() << "JSON File opened";

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    auto root = doc.object();
    QJsonArray events = root["events"].toArray();

    qDebug() << "Events total count: " << events.size();

    for (const auto& e: events)
    {
        auto eObj = e.toObject();

        auto type = eObj["type"].toString();
        double start = eObj["start"].toDouble();
        double end = eObj["end"].toDouble();

        qDebug() << "Type " << type << " start " << start << " end " << end;
    }

    //Draw silence as blue, and word as green
    //x is time*coef = 100, for a start
}
