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
        double prevMin = 0.0;
        double prevMax = 0.0;
    };

    struct FieldPrevStats
    {
        double prevXEnd = 0.0;

        std::map<QString, double> prevValues;

        double prevJitter = 0.0;
        double prevShimmer = 0.0;
    };

    struct FieldDisplayInfo
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
        PlainWords,
        ReportFields
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

        Q_INVOKABLE int getType() const {
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

        Q_INVOKABLE QVariantList getPraatFields() const;


        Q_INVOKABLE void addReportField(QString name, QString color, double yCoef)
        {
            _reportFields[name] = {color, yCoef};
            //update();
        }

        Q_INVOKABLE void clearReportFields() {
            _reportFields.clear();
        }

        Q_INVOKABLE QVariantList getReportFields() const;

        Q_INVOKABLE void setShowBorder(bool value) {
            _showBorder = value;
            update();
        }


    private:

        void paintChunksOnly(QPainter* painter) const;

        void paintSequenceType(QPainter* painter, const QJsonObject& event,
                               int idx, ReportPrevStats &prevStats) const;

        void paintPraatInfo(QPainter* painter, const QJsonObject& event,
                            int idx, FieldPrevStats &prevStats) const;

        void paintReportFields(QPainter* painter, const QJsonObject& event,
                               int idx, FieldPrevStats &prevStats) const;

        void paintPlainWords(QPainter* painter, const QJsonObject& event) const;


        QVariantList getFields(const std::map<QString, FieldDisplayInfo>& fields) const;


        VisualTypes _type;
        bool _showBorder = false;

        std::map<QString, FieldDisplayInfo> _praatFields;
        std::map<QString, FieldDisplayInfo> _reportFields;

        JsonReport* _parentReport;

    };


}



#endif // VISUALREPORT_H
