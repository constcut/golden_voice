#ifndef JSONREPORT_HPP
#define JSONREPORT_HPP

#include <QObject>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include <set>


//TODO clean after JsonReport Merge


namespace diaryth
{

    class JsonReport : public QObject
    {
        Q_OBJECT

    public:

        explicit JsonReport(QObject *parent = nullptr);

        Q_INVOKABLE double getFullWidth() {
            return _fullWidth + 20; //TODO recalvulate
        }

        Q_INVOKABLE double getZoom() { //Time - добавить в название, чтобы было понятно
            return _zoomCoef;
        }

        Q_INVOKABLE void setZoom(double newZoom) { //TODO Move all to cpp even single liners, uses them first in file
            _zoomCoef = newZoom;
            //update(); //TODO notify call all connected objects
        }

        Q_INVOKABLE int eventIdxOnClick(int mouseX, int mouseY);
        Q_INVOKABLE void selectEvent(int idx);

        Q_INVOKABLE QVariant getSelectedEvents();
        Q_INVOKABLE QList<qreal> getChunkInfo(int idx); //TODO как и с selection убрать все в класс хранитель JSON общий для всех репортов подключать через QObject*
        Q_INVOKABLE QList<qreal> getFullInfo();
        Q_INVOKABLE QStringList getPraatFieldsNames();

        Q_INVOKABLE QString getWordByIdx(int idx);


        Q_INVOKABLE void removeAllSelections() {
            _selectedIdx.clear();
            updateAllVisualReports();
        }

        Q_INVOKABLE int getChunksCount();
        Q_INVOKABLE void selectChunk(int idx);

        //void addVisualReport
        //void removeVisualReport
        //void clearAllVisualReport

    private:

        void updateAllVisualReports();

        double _fullWidth;
        double _zoomCoef;

        QJsonArray _events; //TODO вероятно позже лучше хранить в отдельном классе, который связывать с множеством репортов - иначе дублировани
        QJsonArray _chunks; //Операции выделения итд можно тоже поместить в этот отдельный класс
        QJsonObject _fullPraat;

        std::set<int> _selectedIdx;

    };

}

#endif // JSONREPORT_HPP
