#ifndef VISUALREPORT_H
#define VISUALREPORT_H


#include <QQuickPaintedItem>
#include <QObject>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include <set>
#include <map>

#include "JsonReport.hpp"


namespace diaryth
{


    struct ReportPrevStats
    {
        double prevMean = 0.0;
        double prevMedian = 0.0;
        double prevXEnd = 0.0;
    };

    struct PraatPrevStats
    {
        double prevXEnd = 0.0;

        std::map<QString, double> prevValues;

        double prevJitter = 0.0;
        double prevShimmer = 0.0;
    };

    struct PraatFieldDisplayInfo
    {
        QString color;
        double yCoef;
    };


    class VisualReport : public QQuickPaintedItem
    {
        Q_OBJECT

    public:

        VisualReport();

        void paint(QPainter* painter);

        enum VisualTypes {
            TypeNotSet,
            Pitch,
            Amplitude,
            PraatInfo
        };

        Q_INVOKABLE void setParent(JsonReport* report)
        {
            _parentReport = report;
            report->registerVisual(this);
            update();
        }

        Q_INVOKABLE void setPitchType()
        {
            _type = VisualTypes::Pitch;
            update();
        }

        Q_INVOKABLE void setAmpitudeType()
        {
            _type = VisualTypes::Amplitude;
            update();
        }

        Q_INVOKABLE void setPraatType()
        {
            _type = VisualTypes::PraatInfo;
            update();
        }

        Q_INVOKABLE void addPraatField(QString name, QString color, double yCoef)
        {
            _praatFields[name] = {color, yCoef};
            //update();
        }

        Q_INVOKABLE void clearPraatFields() {
            _praatFields.clear();
        }

        Q_INVOKABLE QVariant getPraatFields();


    private:

        void paintSequenceType(QPainter* painter, QJsonObject& event,
                               int idx, ReportPrevStats &prevStats);

        void paintPraatInfo(QPainter* painter, QJsonObject& event,
                            int idx, PraatPrevStats &prevStats);


        VisualTypes _type;

        std::map<QString, PraatFieldDisplayInfo> _praatFields;

        JsonReport* _parentReport;

    };


}



#endif // VISUALREPORT_H
