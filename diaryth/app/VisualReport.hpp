#ifndef VISUALREPORT_H
#define VISUALREPORT_H

#include <QQuickPaintedItem>
#include <QObject>

#include <QJsonArray>

namespace diaryth
{

    class VisualReport : public QQuickPaintedItem
    {
        Q_OBJECT
    public:
        VisualReport();

        void paint(QPainter* painter);

        enum VisualTypes {
            Pitch,
            Amplitude
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

    private:

        VisualTypes _type = VisualTypes::Pitch;

        QJsonArray _events;

        double _fullWidth;
        double _zoomCoef;
    };


}



#endif // VISUALREPORT_H
