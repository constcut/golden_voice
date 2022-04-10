#ifndef SQLBASE_HPP
#define SQLBASE_HPP

#include <QObject>
#include <QVariantList>
#include <QSqlQuery>

namespace diaryth {


    class SQLBase : public QObject
    {
        Q_OBJECT

    public:

        SQLBase();


        QSqlQuery executeRequest(const QString& requestBody) const;


        Q_INVOKABLE void addAudioRecord(const QString& date, const QString& time, int localId,
                                        const QString& name, quint64 durationMs) const;

        Q_INVOKABLE void editAudioRecord(const QString& date, int localId, const QString& name,
                                         const QString& tags, const QString& description) const;

        Q_INVOKABLE void removeAudioRecord(const QString& date, int localId) const;

        Q_INVOKABLE int getRecordsMaxLocalId(const QString& date) const;
        Q_INVOKABLE int getTotalRecords() const;

        Q_INVOKABLE QStringList findSingleRecord(const QString& date, int localId) const;
        Q_INVOKABLE QVariantList findRecords(const QString& date) const;

        Q_INVOKABLE QVariantList findRecordsByNameMask(const QString& nameMask) const;
        Q_INVOKABLE QVariantList findRecordsByTagMask(const QString& tagMask) const;

        Q_INVOKABLE QVariantList findRecordsByNameMaskAndDate(const QString& date,
                                                              const QString& nameMask) const;

        Q_INVOKABLE QVariantList findRecordsByTagMaskAndDate(const QString& date,
                                                             const QString& tagMask) const;


        Q_INVOKABLE void addText(const QString& name, const QString& text,
                                 const QString& tags, const QString& description) const;

        Q_INVOKABLE void editText(const QString& date, int localId, const QString& name,
                                  const QString& tags, const QString& description) const;

        Q_INVOKABLE void removeText(const QString& date, int localId) const;

        Q_INVOKABLE int getTextsMaxLocalId(const QString& date) const;
        Q_INVOKABLE int getTotalTexts() const;

        Q_INVOKABLE QStringList findSingleText(const QString& date, int localId) const;
        Q_INVOKABLE QVariantList findTexts(const QString& date) const;

        Q_INVOKABLE QVariantList findTextsByNameMask(const QString& nameMask) const;
        Q_INVOKABLE QVariantList findTextsByTagMask(const QString& tagMask) const;

        Q_INVOKABLE QVariantList findTextsByNameMaskAndDate(const QString& date,
                                                            const QString& nameMask) const;

        Q_INVOKABLE QVariantList findTextsByTagMaskAndDate(const QString& date,
                                                           const QString& tagMask) const;


        Q_INVOKABLE QStringList getAllCardsNames() const;
        Q_INVOKABLE int getTotalCards() const;

        Q_INVOKABLE bool checkCardNameExists(const QString& name) const;

        Q_INVOKABLE int getCardId(const QString& name) const;
        Q_INVOKABLE QString getCardDescription(const QString& name) const;
        Q_INVOKABLE QString getCardJSON(const QString& name) const;

        Q_INVOKABLE void addCard(const QString& name, const QString& json) const;
        Q_INVOKABLE void addCardFromFile(const QString& name, const QString& filename) const;

        Q_INVOKABLE void editCard(const QString& name, const QString& json) const;
        Q_INVOKABLE void editCardFromFile(const QString& name, const QString& filename) const;

        Q_INVOKABLE void removeCard(const QString& name) const;

        Q_INVOKABLE void setCardDescription(const QString& name, const QString& description) const;



        Q_INVOKABLE int getTotalCardRecords() const;

        Q_INVOKABLE int getCardRecordsMaxLocalId() const;
        Q_INVOKABLE int getCardRecordsMaxLocalIdonDate(const QString& date) const;

        Q_INVOKABLE void addCardRecord(const QString& cardName, const QString& cardDate,
                                       const QString& groupName, QVariantList groupFields);

        Q_INVOKABLE int getCardRecordsCountOnDate(const QString& cardDate) const;

        Q_INVOKABLE QVariantList getAllCardRecordsOnDate(const QString& cardDate) const;
        Q_INVOKABLE QVariantList getAllCardRecords() const;
        Q_INVOKABLE QVariantList getAllCardRecordsForCard(const QString& cardName) const;
        Q_INVOKABLE QVariantList getAllCardRecordsForCardAndDate(const QString& cardName,
                                                                 const QString& cardDate) const;

        Q_INVOKABLE QStringList getAllTestsNames() const;
        Q_INVOKABLE int getTotalTests() const;

        Q_INVOKABLE bool checkTestNameExists(const QString& name) const;
        Q_INVOKABLE QString getTestJSON(const QString& name) const;

        Q_INVOKABLE void addTest(const QString& name, const QString& json) const;
        Q_INVOKABLE void addTestFromFile(const QString& name, const QString& filename) const;

        Q_INVOKABLE void editTest(const QString& name, const QString& json) const;
        Q_INVOKABLE void editTestFromFile(const QString& name, const QString& filename) const;

        Q_INVOKABLE void removeTest(const QString& name) const;

        Q_INVOKABLE void setTestDescription(const QString& name, const QString& description) const;
        Q_INVOKABLE QString getTestDescription(const QString& name) const;



        Q_INVOKABLE int getTotalTestsResults() const;
        Q_INVOKABLE int getTestsResulstCount(const QString& name) const;
        Q_INVOKABLE int getTotalTetsResultsByDate(const QString& date) const;

        //Q_INVOKABLE QVariantList getAllTestsResults(QString& name) const;

        Q_INVOKABLE QVariantList getAllTestsResults(const QString& name) const;
        Q_INVOKABLE QVariantList getAllTestsResultsOnDate(const QString& date) const;

        Q_INVOKABLE void addTestResult(const QString& testName, const QString& testRate,
                                       const QString& rateText) const;



    private:

        QSqlError initBase() const;
        void createTablesIfNeeded() const;

        QVariantList fillRecordsSearchResults(QSqlQuery& query) const;

        bool logIfError(QSqlQuery& query, const QString& request) const;

        QVariantList findByFieldMaskAndDate(const QString& table, const QString& field,
                                            const QString& date, const QString& mask) const;

        QVariantList findByFieldMask(const QString& table, const QString& field,
                                     const QString& mask) const;

        QVariantList findByDate(const QString& table, const QString& date) const;

        QStringList findSingle(const QString& table, int fieldsCount,
                               const QString& date, int localId) const;

        int getTotalRows(const QString& keyField, const QString& table) const;

        int getMaxLocalId(const QString& table, const QString& date) const;

        void removeRow(const QString& table, const QString& date, int localId) const;

        void editRow(const QString& table, const QString& nameField, const QString& date,
                     int localId, const QString& name, const QString& tags,
                     const QString& description) const;

        QString loadTextFromFile(const QString& filename) const;


        const int audioFieldsCount = 7; //Later use walkaround with query.last() and query.at() + 1
        const int textFieldsCount = 7;
        const int cardRecordsFieldsCount = 9;
        const int testsResultsFieldsCount = 6;
    };

}

#endif // SQLBASE_HPP
