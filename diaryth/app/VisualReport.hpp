#ifndef VISUALREPORT_H
#define VISUALREPORT_H

#include <QQuickPaintedItem>
#include <QObject>

namespace diaryth
{

    class VisualReport : public QQuickPaintedItem
    {
        Q_OBJECT
    public:
        VisualReport() = default;

        void paint(QPainter* painter);

        enum VisualTypes {
            Pitch,
            Amplitude
        };

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
    };


}



#endif // VISUALREPORT_H
