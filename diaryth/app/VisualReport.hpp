#ifndef VISUALREPORT_H
#define VISUALREPORT_H


#include <QQuickPaintedItem>
#include <QObject>

#include <QJsonArray>
#include <QJsonObject>

#include <QVariantList>

#include <set>
#include <map>


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
        QColor color;
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

        Q_INVOKABLE double getFullWidth() {
            return _fullWidth + 20;
        }

        Q_INVOKABLE double getZoom() { //Time - добавить в название, чтобы было понятно
            return _zoomCoef;
        }

        Q_INVOKABLE void setZoom(double newZoom) { //TODO Move all to cpp even single liners, uses them first in file
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

        Q_INVOKABLE void addPraatField(QString name, QString color, double yCoef)
        {
            _praatFields[name] = {QColor(color), yCoef};
            //update();
        }

        Q_INVOKABLE int eventIdxOnClick(int mouseX, int mouseY);
        Q_INVOKABLE void selectEvent(int idx);

        Q_INVOKABLE QList<QList<qreal>> getSelectedEvents();
        Q_INVOKABLE QList<qreal> getChunkInfo(int idx); //TODO как и с selection убрать все в класс хранитель JSON общий для всех репортов подключать через QObject*
        Q_INVOKABLE QList<qreal> getFullInfo();
        Q_INVOKABLE QStringList getPraatFieldsNames();

        Q_INVOKABLE QString getWordByIdx(int idx);


        Q_INVOKABLE void removeAllSelections() {
            _selectedIdx.clear();
            update();
        }

        Q_INVOKABLE int getChunksCount();
        Q_INVOKABLE void selectChunk(int idx);

    private:

        void paintSequenceType(QPainter* painter, QJsonObject& event,
                               int idx, ReportPrevStats &prevStats);

        void paintPraatInfo(QPainter* painter, QJsonObject& event,
                            int idx, PraatPrevStats &prevStats);


        VisualTypes _type;

        double _fullWidth;
        double _zoomCoef;

        std::map<QString, PraatFieldDisplayInfo> _praatFields;

        QJsonArray _events; //TODO вероятно позже лучше хранить в отдельном классе, который связывать с множеством репортов - иначе дублировани
        QJsonArray _chunks; //Операции выделения итд можно тоже поместить в этот отдельный класс
        QJsonObject _fullPraat;

        std::set<int> _selectedIdx;
    };


}



#endif // VISUALREPORT_H
