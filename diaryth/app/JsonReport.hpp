#ifndef JSONREPORT_HPP
#define JSONREPORT_HPP

#include <QObject>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include <set>
#include <vector>


//TODO clean after JsonReport Merge
//TODO const correct for all JsonReport\Visual\Network

namespace diaryth
{

    class VisualReport;


    class JsonReport : public QObject
    {
        Q_OBJECT

    public:

        explicit JsonReport(QObject *parent = nullptr);

        Q_INVOKABLE double getFullWidth();

        Q_INVOKABLE double getZoom() { //Time - добавить в название, чтобы было понятно
            return _zoomCoef;
        }

        Q_INVOKABLE void setZoom(double newZoom) { //TODO Move all to cpp even single liners, uses them first in file
            _zoomCoef = newZoom;
            updateAllVisualReports();
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

        Q_INVOKABLE int getLastSelectedChunk() {
            return _lastSelectedChunk;
        }

        const QJsonArray& getEvents() { return _events; }
        const QJsonArray& getChunks() { return _chunks; }
        const QJsonObject& getFullPraat() { return _fullPraat; }
        const std::set<int>& getSelectedIdx() { return _selectedIdx; }

        void registerVisual(VisualReport* visual); //Not Q_INVOKABLE  beasue must be registered from VR set parent
        Q_INVOKABLE void removeVisual(VisualReport* visual);
        Q_INVOKABLE void clearVisuals();

        Q_INVOKABLE void saveLocalConfig();
        Q_INVOKABLE QList<int> loadLocalConfig();

        Q_INVOKABLE void setConfiпFilename(QString filename) {
            _configFilename = filename;
        }

    private:

        void updateAllVisualReports();

        double _fullWidth = 0.0;
        double _zoomCoef = 250.0;

        QJsonArray _events; //TODO вероятно позже лучше хранить в отдельном классе, который связывать с множеством репортов - иначе дублировани
        QJsonArray _chunks; //Операции выделения итд можно тоже поместить в этот отдельный класс
        QJsonObject _fullPraat;

        std::set<int> _selectedIdx;
        int _lastSelectedChunk = 0;

        std::set<VisualReport*> _connectedVisuals; //Todo replace with vector

        QString _configFilename;

    };

}

#endif // JSONREPORT_HPP
