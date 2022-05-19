#ifndef JSONREPORT_HPP
#define JSONREPORT_HPP

#include <QObject>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include <set>
#include <vector>



namespace diaryth
{

    class VisualReport;


    class JsonReport : public QObject
    {
        Q_OBJECT

    public:

        explicit JsonReport(QObject *parent = nullptr);

        Q_INVOKABLE void loadFromFile(QString filename);

        Q_INVOKABLE double getFullWidth() const;

        Q_INVOKABLE double getZoom() const {
            return _zoomCoef;
        }

        Q_INVOKABLE void setZoom(double newZoom) {
            _zoomCoef = newZoom;
            updateAllVisualReports();
        }

        Q_INVOKABLE int eventIdxOnClick(int mouseX, int mouseY) const;
        Q_INVOKABLE void selectEvent(int idx);

        Q_INVOKABLE QVariant getSelectedEvents() const;
        Q_INVOKABLE QList<qreal> getChunkInfo(int idx) const;
        Q_INVOKABLE QList<qreal> getFullInfo() const;
        Q_INVOKABLE QStringList getPraatFieldsNames() const;


        Q_INVOKABLE QString getSelectedEventsString() const;
        Q_INVOKABLE QStringList getSelectedEventsMarkup() const;
        Q_INVOKABLE void saveSelectedEventsMarkup(QString tags, QString comments);


        Q_INVOKABLE QString getWordByIdx(int idx) const;


        Q_INVOKABLE void removeAllSelections() {
            _selectedIdx.clear();
            updateAllVisualReports();
        }

        Q_INVOKABLE int getChunksCount() const;
        Q_INVOKABLE void selectChunk(int idx);

        Q_INVOKABLE int getLastSelectedChunk() const {
            return _lastSelectedChunk;
        }

        const QJsonArray& getEvents() const { return _events; }
        const QJsonArray& getChunks() const { return _chunks; }
        const QJsonObject& getFullPraat() const { return _fullPraat; }
        const std::set<int>& getSelectedIdx() const { return _selectedIdx; }


        void registerVisual(VisualReport* visual);
        Q_INVOKABLE void removeVisual(VisualReport* visual);
        Q_INVOKABLE void clearVisuals();


        Q_INVOKABLE void saveLocalConfig() const;
        Q_INVOKABLE QList<int> loadLocalConfig();


    private:

        void updateAllVisualReports();

        double _zoomCoef = 250.0;

        QJsonObject _root; //Возможно стоит хранить только этот объект
        QString _lastFilename;

        QJsonArray _events;
        QJsonArray _chunks;
        QJsonObject _fullPraat;

        std::set<int> _selectedIdx;
        int _lastSelectedChunk = 0;

        std::vector<VisualReport*> _connectedVisuals;

        QString _configFilename;

    };

}

#endif // JSONREPORT_HPP
