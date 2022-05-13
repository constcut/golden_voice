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
    };


}



#endif // VISUALREPORT_H
