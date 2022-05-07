#include "SQLBase.hpp"

#include <QDebug>
#include <QSqlError>
#include <QFile>
#include <QDate>
#include <QTime>

#include <QVariant>
#include <QVariantList>


using namespace diaryth;


SQLBase::SQLBase() {
    createTablesIfNeeded();
}


QSqlError SQLBase::initBase() const
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("base.sqlite");

    if (!db.open())
        return db.lastError();

    qDebug() << "List of existing sql tables";
    const QStringList tables = db.tables();
    for (const auto& table: tables)
        qDebug() << ":" << table;

    return QSqlError();
}


QSqlQuery SQLBase::executeRequest(const QString& requestBody) const
{
    QSqlQuery request;
    if (request.exec(requestBody) == false)
        qDebug() << "Request error: " << request.lastError() << " for " << requestBody;
    return request;
}


void SQLBase::createTablesIfNeeded() const {

    auto initResult = initBase();
    if (initResult.text().isEmpty() == false)
        qDebug() << "Init base error: " << initResult;

    const QString audioTableCreate("CREATE TABLE IF NOT EXISTS audio ("
                           "audioId integer primary key autoincrement NOT NULL,"
                           "datePart text NOT NULL,"
                           "timePart text NOT NULL,"
                           "localId integer NOT NULL,"
                           "audioName text NOT NULL,"
                           "durationMs integer,"
                           "tags text,"
                           "description text);");

    executeRequest(audioTableCreate); //QSqlQuery audioTableQuery =

    const QString textTableCreate("CREATE TABLE IF NOT EXISTS texts ("
                           "textId integer primary key autoincrement NOT NULL,"
                           "datePart text NOT NULL,"
                           "timePart text NOT NULL,"
                           "localId integer NOT NULL,"
                           "textName text NOT NULL,"
                           "textValue text NOT NULL,"
                           "tags text,"
                           "description text);");

    //Если разрешить редактирование текста, возможно стоит завести таблицу, где хранить несколько последних версий
    executeRequest(textTableCreate); //QSqlQuery textTableQuery =


    const QString cardRecordsTableCreate("CREATE TABLE IF NOT EXISTS diaryCardRecords ("
                           "diaryRecordId integer primary key autoincrement NOT NULL,"
                           "datePart text NOT NULL,"
                           "timePart text NOT NULL,"
                           "localId integer NOT NULL,"
                           "cardId integer NOT NULL,"
                           "cardDate text NOT NULL,"
                           "groupName text NOT NULL,"
                           "fieldName text NOT NULL,"
                           "fieldValue integer,"
                           "fieldText text);");

    executeRequest(cardRecordsTableCreate);

    const QString diaryCardsTableCreate("CREATE TABLE IF NOT EXISTS diaryCards ("
                           "diaryCardId integer primary key autoincrement NOT NULL,"
                           "cardName text NOT NULL UNIQUE,"
                           "cardDescription text,"
                           "jsonText text NOT NULL);");

    executeRequest(diaryCardsTableCreate);


    const QString testsTableCreate("CREATE TABLE IF NOT EXISTS tests ("
                           "testId integer primary key autoincrement NOT NULL,"
                           "testName text NOT NULL UNIQUE,"
                           "testDescription text,"
                           "jsonText text NOT NULL);");

    executeRequest(testsTableCreate);


    const QString testsResultsTableCreate("CREATE TABLE IF NOT EXISTS testsResults ("
                           "resultId integer primary key autoincrement NOT NULL,"
                           "testName text NOT NULL,"
                           "datePart text NOT NULL,"
                           "timePart text NOT NULL,"
                           "testRate text NOT NULL,"
                           "rateText text NOT NULL);");

    executeRequest(testsResultsTableCreate);
}


void SQLBase::addAudioRecord(const QString& date, const QString& time,
                             int localId, const QString& name, quint64 durationMs) const
{
    QString addAudioRequest =
            QString("INSERT INTO audio (datePart, timePart, localId, audioName, durationMs) "
            "VALUES('%1','%2','%3','%4','%5');")
            .arg(date,time).arg(localId).arg(name).arg(durationMs);

    executeRequest(addAudioRequest);  //Вероятно в начале была бы полезна проверка, что пары date + localId ещё нет
}


void SQLBase::editAudioRecord(const QString& date, int localId, const QString& name,
                              const QString& tags, const QString& description) const
{
    editRow("audio", "audioName", date, localId, name, tags, description);
}


void SQLBase::removeAudioRecord(const QString& date, int localId) const
{
    removeRow("audio", date, localId);
}


int SQLBase::getRecordsMaxLocalId(const QString& date) const
{
    return getMaxLocalId("audio", date);
}


bool SQLBase::logIfError(QSqlQuery& query, const QString &request) const
{
    if (query.lastError().text().isEmpty() == false) {
        qDebug() << "Update request failed " << request
                 << "\n Error: " << query.lastError();
        return true;
    }
    return false;
}


int SQLBase::getTotalRecords() const
{
    return getTotalRows("audioId", "audio");
}


QStringList SQLBase::findSingleRecord(const QString& date, int localId) const
{
    return findSingle("audio", audioFieldsCount, date, localId);
}


QVariantList SQLBase::findRecords(const QString& date) const
{
    return findByDate("audio", date);
}


QVariantList SQLBase::findRecordsByNameMask(const QString& nameMask) const
{
    return findByFieldMask("audio", "audioName", nameMask);
}


QVariantList SQLBase::findRecordsByTagMask(const QString& tagMask) const
{
    return findByFieldMask("audio", "tags", tagMask);
}


QVariantList SQLBase::findRecordsByNameMaskAndDate(const QString& date,
                                                   const QString& nameMask) const
{
    return findByFieldMaskAndDate("audio", "audioName", date, nameMask);
}


QVariantList SQLBase::findRecordsByTagMaskAndDate(const QString& date,
                                                  const QString& tagMask) const
{
    return findByFieldMaskAndDate("audio", "tags", date, tagMask);
}


QVariantList SQLBase::fillRecordsSearchResults(QSqlQuery& query) const
{
     QVariantList records;

     while(query.next())
     {
         QStringList singleRecord;
         for (int i = 1; i <= audioFieldsCount; ++i)
             singleRecord << query.value(i).toString();

         records << singleRecord;
     }

     return records;
}


void SQLBase::addText(const QString& name, const QString& text,
                      const QString& tags, const QString& description) const
{
    auto date = QDate::currentDate().toString("yyyy-MM-dd");
    auto time = QTime::currentTime().toString("HH:mm:ss");

    int localId = getTextsMaxLocalId(date) + 1;

    QString addTextRequest =
            QString("INSERT INTO texts (datePart, timePart, localId, textName, textValue, tags, description) "
            "VALUES('%1','%2','%3','%4','%5','%6','%7');")
            .arg(date,time).arg(localId).arg(name, text, tags, description);

    executeRequest(addTextRequest);
}


void SQLBase::editText(const QString& date, int localId, const QString& name,
                       const QString& tags, const QString& description) const
{
    editRow("texts", "textName", date, localId, name, tags, description);
}


void SQLBase::removeText(const QString& date, int localId) const
{
    removeRow("texts", date, localId);
}


int SQLBase::getTextsMaxLocalId(const QString& date) const
{
    return getMaxLocalId("texts", date);
}


int SQLBase::getTotalTexts() const
{
    return getTotalRows("textId", "texts");
}


QStringList SQLBase::findSingleText(const QString& date, int localId) const
{
    return findSingle("texts", textFieldsCount, date, localId);
}


QVariantList SQLBase::findTexts(const QString& date) const
{
    return findByDate("texts", date); //Attention if fields count will differ it would be an issue
    //must be as param
}


QVariantList SQLBase::findTextsByNameMask(const QString& nameMask) const
{
    return findByFieldMask("texts", "textName", nameMask);
}


QVariantList SQLBase::findTextsByTagMask(const QString& tagMask) const
{
    return findByFieldMask("texts", "tags", tagMask);
}


QVariantList SQLBase::findTextsByNameMaskAndDate(const QString& date,
                                                 const QString& nameMask) const
{
    return findByFieldMaskAndDate("texts", "textName", date, nameMask);
}


QVariantList SQLBase::findTextsByTagMaskAndDate(const QString& date,
                                                const QString& tagMask) const
{
    return findByFieldMaskAndDate("texts", "tags", date, tagMask);
}


QVariantList SQLBase::findByFieldMaskAndDate(const QString& table, const QString& field,
                                             const QString& date, const QString& mask) const
{
    QString findRequest =
            QString("SELECT * FROM %1 WHERE datePart='%2' AND %3 LIKE '%%4%';")
            .arg(table, date, field, mask);

    QSqlQuery requestQuery = executeRequest(findRequest);
    return fillRecordsSearchResults(requestQuery);
}


QVariantList SQLBase::findByFieldMask(const QString& table, const QString& field,
                                      const QString& mask) const
{
    QString findRequest =
            QString("SELECT * FROM %1 WHERE %2 LIKE '%%3%';")
            .arg(table, field, mask);

    QSqlQuery requestQuery = executeRequest(findRequest);
    return fillRecordsSearchResults(requestQuery);
}


QVariantList SQLBase::findByDate(const QString& table, const QString& date) const
{
    QString findRequest =
            QString("SELECT * FROM %1 WHERE datePart='%2';")
            .arg(table, date);

    QSqlQuery requestQuery = executeRequest(findRequest);
    return fillRecordsSearchResults(requestQuery);
}


QStringList SQLBase::findSingle(const QString& table, int fieldsCount,
                                const QString& date, int localId) const
{
    QString requestSingle =
            QString("SELECT * FROM %1 WHERE datePart='%2' AND localId='%3';")
            .arg(table, date).arg(localId);

    QSqlQuery singleQuery = executeRequest(requestSingle);
    QStringList single;

    if (singleQuery.next())
        for (int i = 1; i <= fieldsCount; ++i)
            single << singleQuery.value(i).toString();

    return single;
}


int SQLBase::getTotalRows(const QString& keyField, const QString& table) const
{
    QString requestTotal = QString("SELECT COUNT(%1) FROM %2")
                           .arg(keyField, table);

    QSqlQuery totalQuery = executeRequest(requestTotal);

    if (totalQuery.next())
        return totalQuery.value(0).toInt();

    return 0;
}


int SQLBase::getMaxLocalId(const QString& table, const QString& date) const
{
    QString requestMaxId =
            QString("SELECT MAX(localId) FROM %1 WHERE datePart='%2';")
            .arg(table, date);

    QSqlQuery maxIdQuery = executeRequest(requestMaxId);

    if (logIfError(maxIdQuery, requestMaxId) == false
            && maxIdQuery.next())
            return maxIdQuery.value(0).toInt();

    return 0;
}


void SQLBase::removeRow(const QString& table, const QString& date, int localId) const
{
    QString deleteRequest =
            QString("DELETE FROM %1 WHERE datePart='%2' AND localId='%3';")
            .arg(table, date).arg(localId);

    QSqlQuery deleteQuery = executeRequest(deleteRequest);
    logIfError(deleteQuery, deleteRequest);
}


void SQLBase::editRow(const QString& table, const QString& nameField, const QString& date, int localId,
                      const QString& name, const QString& tags, const QString& description) const
{
    QString updateRequest =
            QString("UPDATE %1 SET tags='%2', description='%3', %4='%5' "
                    "WHERE datePart='%6' AND localId='%7';")
            .arg(table, tags, description, nameField, name, date).arg(localId);

    QSqlQuery updateQuery = executeRequest(updateRequest);
    logIfError(updateQuery, updateRequest);
}



bool SQLBase::checkCardNameExists(const QString& name) const
{
    QString checkRequest =
            QString("SELECT * FROM diaryCards WHERE cardName='%1';").arg(name);

    QSqlQuery checkQuery = executeRequest(checkRequest);

    if (checkQuery.next())
        return true;

    return false;
}


QStringList SQLBase::getAllCardsNames() const
{
     QString allNamesRequest = QString("SELECT cardName FROM diaryCards;");
     QSqlQuery allNamesQuery = executeRequest(allNamesRequest);

     QStringList allNames;
     while (allNamesQuery.next())
        allNames.append(allNamesQuery.value(0).toString());

     return allNames;
}


QString SQLBase::getCardDescription(const QString& name) const
{
    QString descriptionRequest =
            QString("SELECT cardDescription FROM diaryCards WHERE cardName='%1';").arg(name);

    QSqlQuery descriptionQuery = executeRequest(descriptionRequest);

    if (descriptionQuery.next())
        return descriptionQuery.value(0).toString();

    return {};
}


QString SQLBase::getCardJSON(const QString& name) const
{
    QString jsonRequest =
            QString("SELECT jsonText FROM diaryCards WHERE cardName='%1';").arg(name); //can be done tiny ref ^1,2 v,1

    QSqlQuery jsonQuery = executeRequest(jsonRequest);

    if (jsonQuery.next()) {
        auto base64Json =  jsonQuery.value(0).toString();
        return QByteArray::fromBase64(base64Json.toLocal8Bit());
    }

    return {};
}


int SQLBase::getCardId(const QString& name) const
{
    QString descriptionRequest =
            QString("SELECT diaryCardId FROM diaryCards WHERE cardName='%1';").arg(name);

    QSqlQuery descriptionQuery = executeRequest(descriptionRequest);

    if (descriptionQuery.next())
        return descriptionQuery.value(0).toInt();

    return -1;
}


int SQLBase::getTotalCards() const
{
    return getTotalRows("cardName", "diaryCards");
}


void SQLBase::addCard(const QString& name, const QString& json) const
{
    QString base64Json = json.toLocal8Bit().toBase64();

    QString addCardRequest =
            QString("INSERT INTO diaryCards (cardName, jsonText) VALUES('%1', '%2');")
            .arg(name, base64Json);

    executeRequest(addCardRequest);
}


void SQLBase::addCardFromFile(const QString& name, const QString& filename) const //Возможно удобней сразу из файла находить имя и описание
{
    addCard(name, loadTextFromFile(filename));
}


void SQLBase::editCard(const QString& name, const QString& json) const
{
    QString base64Json = json.toLocal8Bit().toBase64();

    QString updateCardRequest =
            QString("UPDATE diaryCards SET jsonText='%1' WHERE cardName='%2';")
            .arg(base64Json, name);

    executeRequest(updateCardRequest);
}


void SQLBase::editCardFromFile(const QString& name, const QString& filename) const
{
    editCard(name, loadTextFromFile(filename));
}


void SQLBase::removeCard(const QString& name) const
{
    QString deleteRequest =
            QString("DELETE FROM diaryCards WHERE cardName='%1';")
            .arg(name);

    QSqlQuery deleteQuery = executeRequest(deleteRequest);
    logIfError(deleteQuery, deleteRequest);
}


QString SQLBase::loadTextFromFile(const QString& filename) const
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    return in.readAll();
}


void SQLBase::setCardDescription(const QString& name, const QString& description) const
{
    QString updateCardRequest =
            QString("UPDATE diaryCards SET cardDescription='%1' WHERE cardName='%2';")
            .arg(description, name);

    executeRequest(updateCardRequest);
}



int SQLBase::getCardRecordsMaxLocalId() const
{
    QString requestMaxId =
            QString("SELECT MAX(localId) FROM diaryCardRecords;");

    QSqlQuery maxIdQuery = executeRequest(requestMaxId);

    if (logIfError(maxIdQuery, requestMaxId) == false
            && maxIdQuery.next())
            return maxIdQuery.value(0).toInt();

    return 0;
}


int SQLBase::getCardRecordsMaxLocalIdonDate(const QString& date) const
{
    return getMaxLocalId("diaryCardRecords", date);
}


int SQLBase::getTotalCardRecords() const
{
    return getTotalRows("diaryCardRecords", "diaryRecordId");
}


int SQLBase::getCardRecordsCountOnDate(const QString& cardDate) const
{
    QString countRequest =
            QString("SELECT COUNT(localId) FROM diaryCardRecords WHERE cardDate='%1';")
            .arg(cardDate);

    QSqlQuery countQuery = executeRequest(countRequest);

    if (countQuery.next())
        return countQuery.value(0).toInt();

    return 0;
}


QVariantList SQLBase::getAllCardRecordsOnDate(const QString& cardDate) const
{
    QString findRequest =
            QString("SELECT * FROM diaryCardRecords WHERE cardDate='%1';")
            .arg(cardDate);

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList cardRecords;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= cardRecordsFieldsCount; ++i)
            singleRecord << requestQuery.value(i).toString();

        cardRecords.append(singleRecord);
    }

    return cardRecords;
}


QVariantList SQLBase::getAllCardRecords() const //May refact together ^ v
{
    QString findRequest =
            QString("SELECT * FROM diaryCardRecords;");

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList cardRecords;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= cardRecordsFieldsCount; ++i)
            singleRecord << requestQuery.value(i).toString();

        cardRecords.append(singleRecord);
    }

    return cardRecords;
}


QVariantList SQLBase::getAllCardRecordsForCard(const QString& cardName) const
{
    int cardId = getCardId(cardName);

    QString findRequest =
            QString("SELECT * FROM diaryCardRecords WHERE cardId='%1';")
            .arg(cardId);

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList cardRecords;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= cardRecordsFieldsCount; ++i)
            singleRecord << requestQuery.value(i).toString();

        cardRecords.append(singleRecord);
    }

    return cardRecords;
}


QVariantList SQLBase::getAllCardRecordsForCardAndDate(const QString& cardName,
                                             const QString& cardDate) const
{
    int cardId = getCardId(cardName);

    QString findRequest =
            QString("SELECT * FROM diaryCardRecords WHERE cardId='%1' AND cardDate='%2';")
            .arg(cardId).arg(cardDate);

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList cardRecords;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= cardRecordsFieldsCount; ++i)
            singleRecord << requestQuery.value(i).toString();

        cardRecords.append(singleRecord);
    }

    return cardRecords;
}



void SQLBase::addCardRecord(const QString& cardName, const QString& cardDate,
                            const QString& groupName, QVariantList groupFields)
{

    int localId = getCardRecordsMaxLocalId() + 1;
    auto date = QDate::currentDate().toString("yyyy-MM-dd");
    auto time = QTime::currentTime().toString("HH:mm:ss");
    int cardId = getCardId(cardName);

    for (int i = 0; i < groupFields.size(); ++i)
    {
        auto fieldInfo = groupFields[i].toList();
        auto fieldName = fieldInfo[0].toString();
        auto fieldType = fieldInfo[1].toString();
        auto fieldValue = fieldInfo[2].toString();
        auto isEmpty = fieldInfo[3].toBool();

        if (isEmpty && fieldType != "enum")
            continue;

        QString storeType = "fieldValue";
        if (fieldType == "text" || fieldType == "real")
            storeType = "fieldText";

        QString addCardRecordRequest = QString("INSERT INTO diaryCardRecords "
                "(datePart, timePart, localId, cardId, cardDate, groupName, fieldName, %1) "
                "VALUES('%2','%3','%4','%5','%6','%7','%8','%9');")
                .arg(storeType, date, time).arg(localId).arg(cardId)
                .arg(cardDate, groupName, fieldName, fieldValue);

        executeRequest(addCardRecordRequest);
    }
}




void SQLBase::addTest(const QString& name, const QString& json) const
{
    QString base64Json = json.toLocal8Bit().toBase64();

    QString addTestRequest =
            QString("INSERT INTO tests (testName, jsonText) VALUES('%1', '%2');")
            .arg(name, base64Json);

    executeRequest(addTestRequest);
}


void SQLBase::addTestFromFile(const QString& name, const QString& filename) const
{
    addTest(name, loadTextFromFile(filename));
}


void SQLBase::editTest(const QString& name, const QString& json) const
{
    QString base64Json = json.toLocal8Bit().toBase64();

    QString updateTestRequest =
            QString("UPDATE tests SET jsonText='%1' WHERE cardName='%2';")
            .arg(base64Json, name);

    executeRequest(updateTestRequest);
}


void SQLBase::editTestFromFile(const QString& name, const QString& filename) const
{
    editTest(name, loadTextFromFile(filename));
}


void SQLBase::setTestDescription(const QString& name, const QString& description) const
{
    QString updateTestRequest =
            QString("UPDATE tests SET testDescription='%1' WHERE testName='%2';")
            .arg(description, name);

    executeRequest(updateTestRequest);
}


QString SQLBase::getTestDescription(const QString& name) const
{
    QString descriptionRequest =
            QString("SELECT testDescription FROM tests WHERE testName='%1';").arg(name);

    QSqlQuery descriptionQuery = executeRequest(descriptionRequest);

    if (descriptionQuery.next())
        return descriptionQuery.value(0).toString();

    return {};
}


QStringList SQLBase::getAllTestsNames() const
{
    QString allNamesRequest = QString("SELECT testName FROM tests;");
    QSqlQuery allNamesQuery = executeRequest(allNamesRequest);

    QStringList allNames;
    while (allNamesQuery.next())
       allNames.append(allNamesQuery.value(0).toString());

    return allNames;
}


int SQLBase::getTotalTests() const
{
    return getTotalRows("testName", "tests");
}


bool SQLBase::checkTestNameExists(const QString& name) const
{
    QString checkRequest =
            QString("SELECT * FROM tests WHERE testName='%1';").arg(name);

    QSqlQuery checkQuery = executeRequest(checkRequest);

    if (checkQuery.next())
        return true;

    return false;
}


QString SQLBase::getTestJSON(const QString& name) const
{
    QString jsonRequest =
            QString("SELECT jsonText FROM tests WHERE testName='%1';").arg(name);

    QSqlQuery jsonQuery = executeRequest(jsonRequest);

    if (jsonQuery.next()) {
        auto base64Json =  jsonQuery.value(0).toString();
        return QByteArray::fromBase64(base64Json.toLocal8Bit());
    }

    return {};
}


void SQLBase::removeTest(const QString& name) const
{
    QString deleteRequest =
            QString("DELETE FROM tests WHERE testName='%1';")
            .arg(name);

    QSqlQuery deleteQuery = executeRequest(deleteRequest);
    logIfError(deleteQuery, deleteRequest);
}



int SQLBase::getTotalTestsResults() const
{
    return getTotalRows("resultId", "testsResults");
}


int SQLBase::getTestsResulstCount(const QString& name) const
{
    QString requestTotal =
            QString("SELECT COUNT(resultId) FROM testsResults WHERE testName='%1';")
            .arg(name);

    QSqlQuery totalQuery = executeRequest(requestTotal);

    if (totalQuery.next())
        return totalQuery.value(0).toInt();

    return 0;
}


int SQLBase::getTotalTetsResultsByDate(const QString& date) const
{
    QString requestTotal =
            QString("SELECT COUNT(resultId) FROM testsResults WHERE datePart='%1';")
            .arg(date);

    QSqlQuery totalQuery = executeRequest(requestTotal);

    if (totalQuery.next())
        return totalQuery.value(0).toInt();

    return 0;
}


void SQLBase::addTestResult(const QString& testName, const QString& testRate,
                            const QString& rateText) const
{
    auto date = QDate::currentDate().toString("yyyy-MM-dd");
    auto time = QTime::currentTime().toString("HH:mm:ss");

    QString addTestResultRequest =
            QString("INSERT INTO testsResults (testName, datePart, timePart, testRate, rateText) "
            "VALUES('%1','%2','%3','%4','%5');")
            .arg(testName, date, time, testRate, rateText);

    executeRequest(addTestResultRequest);
}


QVariantList SQLBase::getAllTestsResultsOnDate(const QString& date) const
{
    QString findRequest =
            QString("SELECT * FROM testsResults WHERE datePart='%1';")
            .arg(date);

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList testsResults;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= testsResultsFieldsCount; ++i) //insure <=
            singleRecord << requestQuery.value(i).toString();

        testsResults.append(singleRecord);
    }

    return testsResults;
}


QVariantList SQLBase::getAllTestsResults(const QString& name) const
{
    QString findRequest =
            QString("SELECT * FROM testsResults WHERE testName='%1';")
            .arg(name);

    QSqlQuery requestQuery = executeRequest(findRequest);
    QVariantList testsResults;

    while(requestQuery.next())
    {
        QStringList singleRecord;
        for (int i = 1; i <= testsResultsFieldsCount; ++i) //insure <= (everywhere)
            singleRecord << requestQuery.value(i).toString();

        testsResults.append(singleRecord);
    }

    return testsResults;
}
