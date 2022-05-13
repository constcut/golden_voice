#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

using namespace diaryth;



void VisualReport::paint(QPainter* painter)
{
    QFile f; //full report
    QJsonDocument doc;
    QJsonArray events;

    //Draw silence as blue, and word as green
    //x is time*coef = 100, for a start
}
