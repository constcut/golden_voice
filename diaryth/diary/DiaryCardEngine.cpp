#include "DiaryCardEngine.hpp"

#include <QJsonDocument>
#include <QDebug>
#include <QDate>
#include <QFile>


using namespace diaryth;


void DiaryCardEngine::readFromFile(const QString& filename)
{
    parseJSON(readTextFile(filename));
}


void DiaryCardEngine::mergeFromFile(const QString& filename)
{
    mergeJSON(readTextFile(filename));
}


QString DiaryCardEngine::readTextFile(const QString& filename) const
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    return in.readAll();
}


void DiaryCardEngine::parseJSON(const QString& json)
{
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    if (hasRootErros(doc))
        return;

    clear();
    auto rootObject = doc.object();

    _cardName = rootObject["name"].toString();

    if (rootObject.contains("description"))
        _cardDescription = rootObject["descrition"].toString();

    loadFromJsonObject(rootObject);

    qDebug() << "Diary card json loaded";
}



void DiaryCardEngine::mergeJSON(const QString& json)
{
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    if (hasRootErros(doc))
        return;

    loadFromJsonObject(doc.object());
    //Группы и поля не должны пересекаться, на это пока нет проверки
}


void DiaryCardEngine::loadFromJsonObject(const QJsonObject& rootObject)
{
    if (rootObject.contains("enums") && rootObject["enums"].isArray())
        addEnums(rootObject["enums"].toArray());

    addGroups(rootObject["groups"].toArray());
}


void DiaryCardEngine::clear()
{
    _cardName = "";
    _cardDescription = "";

    _enums.clear();
    _groups.clear();
}


void DiaryCardEngine::addGroups(const QJsonArray& groupsArray)
{
    for (const auto& group: groupsArray)
    {
        const auto& groupObject = group.toObject();
        CardGroup cardGroup;
        fillGroupProperties(cardGroup, groupObject);

        const auto& fieldsArray = groupObject["fields"].toArray();
        for (const auto& field: fieldsArray)
        {
            const auto& fieldObj = field.toObject();
            CardField cardField;
            fillFieldProperties(cardField, fieldObj);

            cardGroup.fields[cardField.name.toStdString()] = cardField;
        }

        _groups[cardGroup.name.toStdString()] = cardGroup;
    }
}


void DiaryCardEngine::fillFieldProperties(CardField& cardField, const QJsonObject& fieldObject) const
{
    cardField.name = fieldObject["name"].toString();

    if (fieldObject.contains("type"))
        cardField.type = fieldObject["type"].toString();

    if (fieldObject.contains("description"))
        cardField.description = fieldObject["description"].toString();

    if (cardField.type == "enum")
        cardField.enumName = fieldObject["enumName"].toString();

    if (cardField.type == "range")
    {
        if (fieldObject.contains("rangeMin"))
            cardField.rangeMin = fieldObject["rangeMin"].toInt();

        if (fieldObject.contains("rangeMax"))
            cardField.rangeMax = fieldObject["rangeMax"].toInt();
    }
}


void DiaryCardEngine::fillGroupProperties(CardGroup& cardGroup, const QJsonObject& groupObject) const
{
    cardGroup.name = groupObject["name"].toString();
    if (groupObject.contains("description"))
        cardGroup.description = groupObject["description"].toString();

    if (groupObject.contains("mandatory"))
        cardGroup.mandatory = groupObject["mandatory"].toBool();

    if (groupObject.contains("onWeekDays"))
    {
        const auto& weekDaysArray = groupObject["onWeekDays"].toArray();
        for (const auto& weekDay: weekDaysArray)
            cardGroup.onWeekDays.append(weekDay.toInt());
    }

    if (groupObject.contains("onMonthDays"))
    {
        const auto& monthDaysArray = groupObject["onMonthDays"].toArray();
        for (const auto& monthDay: monthDaysArray)
            cardGroup.onMonthDays.append(monthDay.toInt());
    }

    if (cardGroup.onMonthDays.empty() && cardGroup.onWeekDays.empty())
    {
        if (groupObject.contains("daysFrequency"))
            cardGroup.daysFrequency = groupObject["daysFrequency"].toInt();
    }
}



bool DiaryCardEngine::hasRootErros(const QJsonDocument& doc) const
{
    if (doc.isObject() == false) {
        qDebug() << "Failed to parse: JSON root is not an object.";
        return true;
    }

    auto rootObject = doc.object();

    if (rootObject.contains("groups") == false) {
        qDebug() << "Failed to prase: JSON root doesn't contain groups field.";
        return true;
    }

    if (rootObject["groups"].isArray() == false) {
        qDebug() << "Failed to parse: JSON groups field isn't array";
        return true;
    }

    if (rootObject.contains("name") == false) {
        qDebug() << "Failed to parse: JSON name field not found.";
        return true;
    }

    return false;
}


void DiaryCardEngine::addEnums(const QJsonArray& enumsArray)
{
    for (const auto& singleEnum: enumsArray)
    {
        const auto& enumObj = singleEnum.toObject();
        auto valuesArray = enumObj["values"].toArray();
        auto vNamesArray = enumObj["names"].toArray();

        if (valuesArray.size() != vNamesArray.size())
        {
            qDebug() << "Parse error: enum values and names arrays has different sizes "
                     << valuesArray.size() << " and " << vNamesArray.size();
            continue;
        }

        CardEnum cardEnum;
        cardEnum.name = enumObj["name"].toString();

        if (enumObj.contains("description"))
            cardEnum.description = enumObj["description"].toString();

        bool showValues = enumObj["showValues"].toBool();

        for (int i = 0; i < valuesArray.size(); ++i)
        {
            const auto value = valuesArray[i].toInt();
            const auto valueName = vNamesArray[i].toString();

            cardEnum.valuesNames.append(valueName);
            cardEnum.values.append(value);

            if (showValues) {
                const QString fullName = QString::number(value) + " " + vNamesArray[i].toString();
                cardEnum.displayNames.append(fullName);
            }
            else
                cardEnum.displayNames.append(valueName);
        }

        _enums[cardEnum.name.toStdString()] = cardEnum;
    }
}


void DiaryCardEngine::setEnumValues(const QString& name, const QList<int>& values,
                                    const QStringList& valuesNames, bool showValues)
{
    if (_enums.count(name.toStdString()) == 0) {
        qDebug() << "Failed to find enum to set values: " << name;
        return;
    }

    _enums[name.toStdString()].values = values;
    _enums[name.toStdString()].valuesNames = valuesNames;

    if (showValues == false)
        _enums[name.toStdString()].displayNames = valuesNames;
    else
    {
        _enums[name.toStdString()].displayNames.clear();

        for (int i = 0; i < values.size(); ++i)
            _enums[name.toStdString()].displayNames.append(
                         QString::number(values[i]) + " " + valuesNames[i]);
    }
}



QStringList DiaryCardEngine::getAllEnumsNames() const
{
    QStringList allNames;

    for (const auto& [enumName, _]: _enums)
        allNames.append(enumName.c_str());

    return allNames;
}


QStringList DiaryCardEngine::getEnumNames(const QString& name) const
{
    if (_enums.count(name.toStdString()) == 0)
        return {};

    return _enums.at(name.toStdString()).valuesNames;
}


QStringList DiaryCardEngine::getEnumDisplayNames(const QString& name) const
{
    if (_enums.count(name.toStdString()) == 0)
        return {};

    return _enums.at(name.toStdString()).displayNames;
}


QList<int> DiaryCardEngine::getEnumValues(const QString& name) const
{
    if (_enums.count(name.toStdString()) == 0)
        return {};

    return _enums.at(name.toStdString()).values;
}


QString DiaryCardEngine::getEnumDescription(const QString& name) const
{
    if (_enums.count(name.toStdString()) == 0)
        return {};

    return _enums.at(name.toStdString()).description;
}


QStringList DiaryCardEngine::getAllGroupsNames() const //Refact generalize
{
    QStringList allNames;

    for (const auto& [groupName, _]: _groups)
        allNames.append(groupName.c_str());

    return allNames;
}


QString DiaryCardEngine::getGroupDescription(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return {};

    return _groups.at(name.toStdString()).description;
}


bool DiaryCardEngine::isGroupMandatory(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return false;

    return _groups.at(name.toStdString()).mandatory;
}


int DiaryCardEngine::getGroupDaysFrequency(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return 0;

    return _groups.at(name.toStdString()).daysFrequency;
}


QList<int> DiaryCardEngine::getGroupWeekDays(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return {};

    return _groups.at(name.toStdString()).onWeekDays;
}


QList<int> DiaryCardEngine::getGroupMonthDays(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return {};

    return _groups.at(name.toStdString()).onMonthDays;
}


bool DiaryCardEngine::isItGroupDay(const QString& name) const
{
    QDate date = QDate::currentDate();
    return isItGroupDay(date, name);
}


bool DiaryCardEngine::isItGroupDay(const QString& date, const QString& name) const
{
    QDate dateObject = QDate::fromString(date); //возможно тут нужна маска
    return isItGroupDay(dateObject, name);
}


bool DiaryCardEngine::isItGroupDay(const QDate& date, const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return false;

     const auto& group = _groups.at(name.toStdString());

     if (group.daysFrequency == 1) //Подумать над другими значениями
         return true;

    if (group.onMonthDays.empty() == false)
        for (int day: group.onMonthDays)
            if (day == date.day())
                return true;

    if (group.onWeekDays.empty() == false)
        for (int day: group.onWeekDays)
            if (day == date.dayOfWeek())
                return true;

    return false;
}


QStringList DiaryCardEngine::getAllGroupFields(const QString& name) const
{
    if (_groups.count(name.toStdString()) == 0)
        return {};

    QStringList allFields;
    for (const auto& [fieldName, _]: _groups.at(name.toStdString()).fields)
        allFields.append(fieldName.c_str());

    return allFields;
}


QString DiaryCardEngine::getFieldType(const QString& group, const QString& field) const
{
    if (isFieldMissing(group, field))
        return "";

    return _groups.at(group.toStdString()).fields.at(field.toStdString()).type;
}


QString DiaryCardEngine::getFieldDescription(const QString& group, const QString& field) const
{
    if (isFieldMissing(group, field))
        return "";

    return _groups.at(group.toStdString()).fields.at(field.toStdString()).description;
}


QString DiaryCardEngine::getFieldEnum(const QString& group, const QString& field) const
{
    if (isFieldMissing(group, field))
        return "";

    return _groups.at(group.toStdString()).fields.at(field.toStdString()).enumName;
}

int DiaryCardEngine::getFieldRangeMin(const QString& group, const QString& field) const
{
    if (isFieldMissing(group, field))
        return -1;

    return _groups.at(group.toStdString()).fields.at(field.toStdString()).rangeMin;
}


int DiaryCardEngine::getFieldRangeMax(const QString& group, const QString& field) const
{
    if (isFieldMissing(group, field))
        return -1;

    return _groups.at(group.toStdString()).fields.at(field.toStdString()).rangeMax;
}


bool DiaryCardEngine::isFieldMissing(const QString& group, const QString& field) const
{
    if (_groups.count(group.toStdString()) == 0)
        return true;

    const auto& groupObj = _groups.at(group.toStdString());
    if (groupObj.fields.count(field.toStdString()) == false)
        return true;

    return false;
}



void DiaryCardEngine::addNewEnum(const QString& name)
{
    if (_enums.count(name.toStdString())) {
        qDebug() << "Warning: attemp to create enum with existing name";
        return;
    }

    CardEnum cardEnum;
    cardEnum.name = name;
    _enums[name.toStdString()] = cardEnum;
}


void DiaryCardEngine::removeEnum(const QString& name)
{
    _enums.erase(name.toStdString());
}


void DiaryCardEngine::addNewGroup(const QString& name)
{
    if (_groups.count(name.toStdString())) {
        qDebug() << "Warning: attemp to create enum with existing name";
        return;
    }

    CardGroup cardGroup;
    cardGroup.name = name;
    _groups[name.toStdString()] = cardGroup;
}


void DiaryCardEngine::removeGroup(const QString& name)
{
    _groups.erase(name.toStdString());
}


void DiaryCardEngine::changeEnumDescription(const QString& enumName, const QString& description)
{
    if (_enums.count(enumName.toStdString()) == 0) {
        qDebug() << "Failed to find enum to change description: " << enumName;
        return;
    }

    _enums[enumName.toStdString()].description = description;
}


void DiaryCardEngine::setGroupDescription(const QString& groupName, const QString& description)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group to change description: " << groupName;
        return;
    }

    _groups[groupName.toStdString()].description = description;
}


void DiaryCardEngine::setGroupMandatory(const QString& groupName, bool value)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group to change mandatory status" << groupName;
        return;
    }

    _groups[groupName.toStdString()].mandatory = value;
}


void DiaryCardEngine::setGroupMonthDays(const QString& groupName, const QList<int>& monthDays)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group to set month days" << groupName;
        return;
    }

    _groups[groupName.toStdString()].onMonthDays = monthDays;
}


void DiaryCardEngine::setGroupWeekDays(const QString& groupName, const QList<int>& weekDays)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group to set week days" << groupName;
        return;
    }

    _groups[groupName.toStdString()].onWeekDays = weekDays;
}


void DiaryCardEngine::setFieldToGroup(const QString& groupName, const QString& fieldName,
                                      const QString& fieldType)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group " << groupName << " to add field " << fieldName;
        return;
    }

    _groups[groupName.toStdString()].fields[fieldName.toStdString()].name = fieldName;
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].type = fieldType;
}


void DiaryCardEngine::setEnumFieldToGroup(const QString& groupName, const QString& fieldName,
                                          const QString& enumName)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group " << groupName << " to add enum field " << fieldName;
        return;
    }

    _groups[groupName.toStdString()].fields[fieldName.toStdString()].name = fieldName;
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].type = "enum";
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].enumName = enumName;
}


void DiaryCardEngine::setRangeFieldToGroup(const QString& groupName, const QString& fieldName,
                                           int rangeMin, int rangeMax)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group " << groupName << " to add range field " << fieldName;
        return;
    }

    _groups[groupName.toStdString()].fields[fieldName.toStdString()].name = fieldName;
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].type = "range";
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].rangeMin = rangeMin;
    _groups[groupName.toStdString()].fields[fieldName.toStdString()].rangeMin = rangeMax;
}


void DiaryCardEngine::removeGroupField(const QString& groupName, const QString& fieldName)
{
    if (_groups.count(groupName.toStdString()) == 0) {
        qDebug() << "Failed to find group " << groupName << " to remove field " << fieldName;
        return;
    }

    _groups.erase(fieldName.toStdString());
}
