#include "VisualReport.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QVariantList>


#include <map>


using namespace diaryth;


VisualReport::VisualReport()
{
    _parentReport = nullptr;

    _praatFields["Jitter (local)"] = { "green", 20 };
    _praatFields["Shimmer (local)"] = { "blue", 10 };

    _reportFields["stats.praat_pitch.SD"] = { "green", 5 }; //В поздних версиях отказаться от _praatFields и только позволят ввести их черех Combo в QML
    _reportFields["stats.intensity.SD"] = { "blue", 5 };
    _reportFields["letters_speed"] = {"red", 200};
}


QVariantList VisualReport::getPraatFields() const
{
    return getFields(_praatFields);
}


QVariantList VisualReport::getReportFields() const
{
    return getFields(_reportFields);
}


QVariantList VisualReport::getFields(const std::map<QString, FieldDisplayInfo>& fields) const
{
    QVariantList allFields;

    for (const auto& [fieldName, fieldData]: fields)
    {
        QStringList fieldLine;
        fieldLine << fieldName << fieldData.color << QString::number(fieldData.yCoef);
        allFields << QVariant::fromValue(fieldLine);
    }

    return allFields;
}


void VisualReport::paint(QPainter* painter)
{
    if (_parentReport == nullptr)
        return;

    ReportPrevStats prevStats;
    FieldPrevStats prevPraats;


    painter->fillRect(0, 0, width(), height(), QColor("lightgray"));
    if (_showBorder)
        painter->drawRect(2, 2, width() - 4, height() - 4);


    const auto& events = _parentReport->getEvents();

    for (int i = 0; i < events.size(); ++i)
    {
        auto event = events[i].toObject();

        if (_type == VisualTypes::Pitch || _type == VisualTypes::Amplitude)
            paintSequenceType(painter, event, i, prevStats);

        if (_type == VisualTypes::PraatInfo || _type == PraatInfoFullDiff || _type == PraatInfoChunkDiff)
            paintPraatInfo(painter, event, i, prevPraats);

        if (_type == VisualTypes::ReportFields)
            paintReportFields(painter, event, i, prevPraats);

        if (_type == VisualTypes::PlainWords)
            paintPlainWords(painter, event);
    }

    if (_type == VisualTypes::ChunksOnly) //Возможно потом объединить в paintSequenceType
        paintChunksOnly(painter);

}


void VisualReport::paintPlainWords(QPainter* painter, const QJsonObject& event) const
{
    if (event["type"].toString() != "word")
        return;

    double start = event["start"].toDouble();
    double end = event["end"].toDouble();

    const double zoom = _parentReport->getZoom();

    auto x = start * zoom + 5;
    double w = (end - start) * zoom;
    QString word = event["word"].toString();

    if (zoom > 150) //В идеале делать более умный рассчёт, проверять наложение и рисовать в 2-4 строки
        painter->drawText(x, 20, word);

    painter->setPen(QColor("red"));
    painter->drawLine(x - 1, 20, x - 1, 0);
    painter->setPen(QColor("blue"));
    painter->drawLine(x + w - 1, 20, x + w - 1, 0);
    painter->setPen(QColor("black"));

    if (zoom > 150 && height() > 35 && event.contains("morph")) //Условие как и выше
    {
        const auto morphObj = event["morph"].toObject();
        if (morphObj.contains("part_of_speech"))
        {
            QString partOfSpeech = morphObj["part_of_speech"].toString();

            std::map<QString, QString> translateMap {
                        std::make_pair("NOUN","сущ"),std::make_pair("ADJF","прил"), std::make_pair("ADJS","кр прил"),
                        std::make_pair("COMP","срав"),std::make_pair("VERB","гл"),std::make_pair("INFN","инф"),
                        std::make_pair("PRTF","прич"),std::make_pair("PRTS","кр прич"),std::make_pair("GRND","дпр"),
                        std::make_pair("NUMR","числ"),std::make_pair("ADVB","нар"),std::make_pair("NPRO","мест"),
                        std::make_pair("PRED","сост"),std::make_pair("PREP","пред"),std::make_pair("CONJ","союз"),
                        std::make_pair("PRCL","част"),std::make_pair("INTJ","межд")};


            qDebug() << partOfSpeech << " translated into " << translateMap.at(partOfSpeech);
            painter->drawText(x, 35, translateMap.at(partOfSpeech));
        }
    }
}



void VisualReport::paintReportFields(QPainter* painter, const QJsonObject &event,
                                     int idx, FieldPrevStats &prevStats) const
{

    auto type = event["type"].toString(); //Возвращать как structure binding?
    double start = event["start"].toDouble();
    double end = event["end"].toDouble();

    const double zoomCoef = _parentReport->getZoom();
    //const auto& selectedIdx = _parentReport->getSelectedIdx();

    if (type == "word")
    {
        auto x = start * zoomCoef + 5;
        double w = (end - start) * zoomCoef;

        auto paintFun = [&](QString fullName, QColor color, double value, double yCoef)
        {
            int y = height() - value * yCoef;

            painter->setPen(color);
            painter->drawLine(x, y, x + w, y);

            if (prevStats.prevXEnd != 0.0)
            {
                painter->drawLine(prevStats.prevXEnd,
                                  prevStats.prevValues[fullName], x, y);
            }

            prevStats.prevValues[fullName] = y;
        };


        for (const auto& [name, fieldDisplayInfo]: _reportFields)
        {

            auto nameParts = name.split(".");

            auto value = 0.0;
            auto currentObject = event;

            bool fieldNotFound = false;

            for (int i = 0; i < nameParts.size(); ++i)
            {
                if (i == nameParts.size() - 1)
                    value = currentObject[nameParts[i]].toDouble();
                else
                {
                    if (currentObject.contains(nameParts[i]))
                        currentObject = currentObject[nameParts[i]].toObject();
                    else
                    {
                        fieldNotFound = true;
                        break;
                    }
                }
            }

            if (fieldNotFound)
                continue;

            auto color = QColor(fieldDisplayInfo.color);
            paintFun(name, color, value, fieldDisplayInfo.yCoef);
        }

        prevStats.prevXEnd = x + w;
    }
}



void VisualReport::paintChunksOnly(QPainter* painter) const
{
    FieldPrevStats prevStats;

    for (int i = 0; i < _parentReport->getChunksCount(); ++i)
    {
        const auto& chunk = _parentReport->getChunks()[i];
        double start = chunk["start"].toDouble();
        double end = chunk["end"].toDouble();

        auto x = start * _parentReport->getZoom() + 5;
        double w = (end - start) * _parentReport->getZoom();

        auto paintFun = [&](QString infoName, QColor color, double yCoef)
        {
            double value = 0.0;

            if (chunk["info"].isObject())
            {
                auto info = chunk["info"].toObject();
                value = info[infoName].toDouble();
            }

            int y = 0;

            if (_type == VisualTypes::PraatInfo)
                y = height() - value * yCoef;
            else
                y = height() / 2  - value * yCoef;

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
            paintFun(name, color, fieldDisplayInfo.yCoef);
        }

         prevStats.prevXEnd = x + w;
    }
}


void VisualReport::paintPraatInfo(QPainter* painter, const QJsonObject &event,
                                  int idx, FieldPrevStats &prevStats) const
{

    auto type = event["type"].toString();
    double start = event["start"].toDouble();
    double end = event["end"].toDouble();

    const double zoomCoef = _parentReport->getZoom();
    const auto& selectedIdx = _parentReport->getSelectedIdx();

    if (type == "word")
    {
        auto x = start * zoomCoef + 5;
        double w = (end - start) * zoomCoef;

        auto paintFun = [&](QString infoName, QColor color, double yCoef)
        {
            double value = 0.0;

            if (event["info"].isObject())
            {
                auto info = event["info"].toObject();

                value = info[infoName].toDouble(); //default PraatInfo

                if (_type == VisualTypes::PraatInfoFullDiff)
                    value = value - _parentReport->getFullPraat()[infoName].toDouble();

                if (_type == VisualTypes::PraatInfoChunkDiff)
                {
                    auto praatInfo = _parentReport->getChunks()[_parentReport->getLastSelectedChunk()]["praat_report"].toObject();
                    value = value - praatInfo[infoName].toDouble();
                }
            }


            int y = 0;

            if (_type == VisualTypes::PraatInfo)
                y = height() - value * yCoef;
            else
                y = height() / 2  - value * yCoef;

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



void VisualReport::paintSequenceType(QPainter* painter, const QJsonObject &event,
                                 int idx, ReportPrevStats& prevStats) const
{
    const auto fullHeight = height() ;

    double verticalZoom = 0.75; //TODO CONFIGURABLE!!!!!!!!!! FOR ALL TYPES TODO TODO TODO TODO TODO TODO

    auto type = event["type"].toString(); //Возвращать как structure binding?
    double start = event["start"].toDouble();
    double end = event["end"].toDouble();

    const double zoomCoef = _parentReport->getZoom();
    const auto& selectedIdx = _parentReport->getSelectedIdx();

    int y = 0;
    QString word;

    double eventH = 0.0;

    const int verticalShift = 0;

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

        y = value["min"].toDouble() * verticalZoom + verticalShift;
        eventH = value["max"].toDouble() * verticalZoom + verticalShift - y;

        if (selectedIdx.count(idx))
            painter->fillRect(x, fullHeight - y, w, - eventH, QBrush(QColor("darkgray")));
        else
            painter->fillRect(x, fullHeight - y, w, - eventH, QBrush(QColor(240, 240, 240)));

        painter->drawEllipse(x, fullHeight - value["min"].toDouble() * verticalZoom - verticalShift, 4, 4);
        painter->drawEllipse(x, fullHeight - value["max"].toDouble() * verticalZoom - verticalShift, 4, 4);

        painter->drawEllipse(x + w, fullHeight - value["min"].toDouble() * verticalZoom - verticalShift, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["max"].toDouble() * verticalZoom - verticalShift, 4, 4);

        auto pen = painter->pen();

        painter->setPen(QColor("green"));
        painter->drawEllipse(x, fullHeight - value["median"].toDouble() * verticalZoom - verticalShift, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["median"].toDouble() * verticalZoom - verticalShift, 4, 4);

        if (prevStats.prevMedian != 0.0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMedian, x,
                              fullHeight - value["median"].toDouble() * verticalZoom - verticalShift);

        prevStats.prevMedian = fullHeight - value["median"].toDouble() * verticalZoom - verticalShift;

        painter->setPen(QColor("blue"));
        painter->drawEllipse(x, fullHeight - value["mean"].toDouble() * verticalZoom - verticalShift, 4, 4);
        painter->drawEllipse(x + w, fullHeight - value["mean"].toDouble() * verticalZoom - verticalShift, 4, 4);

        if (prevStats.prevMean != 0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMean, x,
                              fullHeight - value["mean"].toDouble() * verticalZoom - verticalShift);

        prevStats.prevMean = fullHeight - value["mean"].toDouble() * verticalZoom - verticalShift;


        painter->setPen(QColor("gray"));


        if (prevStats.prevMax != 0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMax,
                              x, fullHeight - value["max"].toDouble() * verticalZoom - verticalShift);

        prevStats.prevMax = fullHeight - value["max"].toDouble() * verticalZoom - verticalShift;


        if (prevStats.prevMin != 0)
            painter->drawLine(prevStats.prevXEnd, prevStats.prevMin, x,
                              fullHeight - value["min"].toDouble() * verticalZoom - verticalShift);

        prevStats.prevMin = fullHeight - value["min"].toDouble() * verticalZoom - verticalShift;


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
            double newY = fullHeight - pY - verticalShift;

            if (pY != 0.0 && prevY != fullHeight - verticalShift)
            {
                if (prevX != 0.0 || prevY != 0.0)
                    painter->drawLine(prevX, prevY, newX, newY);
            }

            prevX = x + i * pixelPerSample;
            prevY = fullHeight - pY - verticalShift;
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
