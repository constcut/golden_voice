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
            PraatInfo,
            PraatInfoFullDiff,
            PraatInfoChunkDiff,
            ChunksOnly,
            PlainWords, //TODO обогатить морфологическим анализом
            ReportFields
            //Отображение статистических значений stats.praat_pitch.median etc - возможно понадобится наложение на PraatInfo чтобы например сравнить средний питч
            //Отображение letters_speed - как быстро произносятся буквы
        };

        Q_INVOKABLE void setParent(JsonReport* report)
        {
            _parentReport = report;
            report->registerVisual(this);
            update();
        }

        Q_INVOKABLE void setPitchType() //TODO setType + QML registered enum
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

        Q_INVOKABLE void setPraatFullDiffType()
        {
            _type = VisualTypes::PraatInfoFullDiff;
            update();
        }

        Q_INVOKABLE void setPraatInfoChunkType()
        {
            _type = VisualTypes::PraatInfoChunkDiff;
            update();
        }

        Q_INVOKABLE void setPlainWordsType()
        {
            _type = VisualTypes::PlainWords;
            update();
        }


        Q_INVOKABLE void setChunksOnlyType()
        {
            _type = VisualTypes::ChunksOnly;
            update();
        }


        Q_INVOKABLE void setReportFieldsType()
        {
            _type = VisualTypes::ReportFields;
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


    private:

        void paintChunksOnly(QPainter* painter);

        void paintSequenceType(QPainter* painter, QJsonObject& event,
                               int idx, ReportPrevStats &prevStats);

        void paintPraatInfo(QPainter* painter, QJsonObject& event,
                            int idx, PraatPrevStats &prevStats);

        void paintReportFields(QPainter* painter, QJsonObject& event,
                               int idx, PraatPrevStats &prevStats);


        VisualTypes _type;

        std::map<QString, PraatFieldDisplayInfo> _praatFields;

        std::map<QString, PraatFieldDisplayInfo> _reportFields; //Later merge _praatFields into _reportFields TODO

        JsonReport* _parentReport;

    };


}



#endif // VISUALREPORT_H
