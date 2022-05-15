#ifndef VISUALREPORT_H
#define VISUALREPORT_H


#include <QQuickPaintedItem>
#include <QObject>

#include <QJsonArray>

#include <QVariantList>

#include <set>


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

        double prevJitter = 0.0;
        double prevShimmer = 0.0;
    };


    class VisualReport : public QQuickPaintedItem
    {
        Q_OBJECT
    public:
        VisualReport();

        void paint(QPainter* painter);

        enum VisualTypes {
            Pitch,
            Amplitude,
            PraatInfo
        };

        Q_INVOKABLE double getFullWidth() {
            return _fullWidth + 20;
        }

        Q_INVOKABLE double getZoom() {
            return _zoomCoef;
        }

        Q_INVOKABLE void setZoom(double newZoom) {
            _zoomCoef = newZoom;
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

        Q_INVOKABLE int eventIdxOnClick(int mouseX, int mouseY);

        Q_INVOKABLE void selectEvent(int idx);

        //Get selected idxs?

        Q_INVOKABLE QVariantList getSelectedEvents();

    private:

        void paintSequenceType(QPainter* painter, QJsonObject& event,
                               int idx, ReportPrevStats &prevStats);

        void paintPraatInfo(QPainter* painter, QJsonObject& event,
                            int idx, PraatPrevStats &prevStats);


        VisualTypes _type = VisualTypes::Pitch;

        QJsonArray _events;

        double _fullWidth;
        double _zoomCoef;

        std::set<int> _selectedIdx;
    };


}



#endif // VISUALREPORT_H
