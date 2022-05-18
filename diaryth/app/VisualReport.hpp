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


    Q_NAMESPACE
    enum VisualTypes {
        TypeNotSet,
        Pitch,
        Amplitude,
        PraatInfo,
        PraatInfoFullDiff,
        PraatInfoChunkDiff,
        ChunksOnly,
        PlainWords, //TODO обогатить морфологическим анализом
        ReportFields
        //Отображение статистических значений stats.praat_pitch.median etc - возможно понадобится наложение на PraatInfo чтобы например сравнить средний питч
        //Отображение letters_speed - как быстро произносятся буквы
    };
    Q_ENUM_NS(VisualTypes)

    class VisualReport : public QQuickPaintedItem
    {
        Q_OBJECT

    public:

        VisualReport();

        void paint(QPainter* painter);



        Q_INVOKABLE void setParent(JsonReport* report)
        {
            _parentReport = report;
            report->registerVisual(this);
            update();
        }

        Q_INVOKABLE void setType(int newType) {
            _type = static_cast<VisualTypes>(newType);
            update();
        }

        Q_INVOKABLE int getType() {
            return _type;
        }


        Q_INVOKABLE void addPraatField(QString name, QString color, double yCoef)
        {
            _praatFields[name] = {color, yCoef};
            //update();
        }

        Q_INVOKABLE void clearPraatFields() {
            _praatFields.clear();
        }

        Q_INVOKABLE QVariantList getPraatFields();



        Q_INVOKABLE void addReportField(QString name, QString color, double yCoef)
        {
            _reportFields[name] = {color, yCoef};
            //update();
        }

        Q_INVOKABLE void clearReportFields() {
            _reportFields.clear();
        }

        Q_INVOKABLE QVariantList getReportFields();

        Q_INVOKABLE void setShowBorder(bool value) {
            _showBorder = value;
            update();
        }


    private:

        void paintChunksOnly(QPainter* painter);

        void paintSequenceType(QPainter* painter, QJsonObject& event,
                               int idx, ReportPrevStats &prevStats);

        void paintPraatInfo(QPainter* painter, QJsonObject& event,
                            int idx, PraatPrevStats &prevStats);

        void paintReportFields(QPainter* painter, QJsonObject& event,
                               int idx, PraatPrevStats &prevStats);


        VisualTypes _type;
        bool _showBorder = false;

        std::map<QString, PraatFieldDisplayInfo> _praatFields;

        std::map<QString, PraatFieldDisplayInfo> _reportFields; //Later merge _praatFields into _reportFields TODO

        JsonReport* _parentReport;

    };


}



#endif // VISUALREPORT_H
