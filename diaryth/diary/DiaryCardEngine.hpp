#ifndef DIARYCARDENGINE_HPP
#define DIARYCARDENGINE_HPP

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <unordered_map>


namespace diaryth {


    struct CardEnum
    {
        QString name;
        QString description;

        QStringList valuesNames;
        QStringList displayNames;
        QList<int> values;
    };


    struct CardField
    {
        QString name;
        QString type = "bool";
        QString description;

        QString enumName;
        int rangeMin = 0;
        int rangeMax = 10;
    };


    struct CardGroup
    {
        QString name;
        QString description;
        bool mandatory = true;

        int daysFrequency = 1;
        QList<int> onWeekDays;
        QList<int> onMonthDays;

        std::unordered_map<std::string, CardField> fields;
    };

    //Возможно добавить новый тип names для записи нескольких значений, например список лекарств, или же делать это как множество отдельных записей

    class DiaryCardEngine : public QObject
    {
        Q_OBJECT
    public:

        DiaryCardEngine() = default;

        Q_INVOKABLE void readFromFile(const QString& filename);
        Q_INVOKABLE void mergeFromFile(const QString& filename);

        Q_INVOKABLE void parseJSON(const QString& json);
        Q_INVOKABLE void mergeJSON(const QString& json);

        //Q_INVOKABLE QString exportJSON(); //Потребуется на этапе констркутора дневниковых карточек

        Q_INVOKABLE QString getCardName() const { return _cardName; }
        Q_INVOKABLE QString getCardDescription() const { return _cardDescription; }

        Q_INVOKABLE QStringList getAllEnumsNames() const;
        Q_INVOKABLE QString getEnumDescription(const QString& name) const;

        Q_INVOKABLE QStringList getEnumNames(const QString& name) const;
        Q_INVOKABLE QStringList getEnumDisplayNames(const QString& name) const;
        Q_INVOKABLE QList<int> getEnumValues(const QString& name) const;

        Q_INVOKABLE QStringList getAllGroupsNames() const;
        Q_INVOKABLE QString getGroupDescription(const QString& name) const;
        Q_INVOKABLE bool isGroupMandatory(const QString& name) const;

        Q_INVOKABLE int getGroupDaysFrequency(const QString& name) const;
        Q_INVOKABLE QList<int> getGroupWeekDays(const QString& name) const;
        Q_INVOKABLE QList<int> getGroupMonthDays(const QString& name) const;

        Q_INVOKABLE bool isItGroupDay(const QString& name) const;
        Q_INVOKABLE bool isItGroupDay(const QString& date, const QString& name) const;

        Q_INVOKABLE QStringList getAllGroupFields(const QString& name) const;
        Q_INVOKABLE QString getFieldType(const QString& group, const QString& field) const;
        Q_INVOKABLE QString getFieldDescription(const QString& group, const QString& field) const;
        Q_INVOKABLE QString getFieldEnum(const QString& group, const QString& field) const;

        Q_INVOKABLE int getFieldRangeMin(const QString& group, const QString& field) const;
        Q_INVOKABLE int getFieldRangeMax(const QString& group, const QString& field) const;


        Q_INVOKABLE void setName(const QString& name) { _cardName = name; }
        Q_INVOKABLE void setDescription(const QString& description) { _cardDescription = description; }

        Q_INVOKABLE void addNewEnum(const QString& name);
        Q_INVOKABLE void removeEnum(const QString& name);

        Q_INVOKABLE void changeEnumDescription(const QString& enumName, const QString& description);

        Q_INVOKABLE void setEnumValues(const QString& name, const QList<int>& values,
                                       const QStringList& valuesNames, bool showValues);


        Q_INVOKABLE void addNewGroup(const QString& name);
        Q_INVOKABLE void removeGroup(const QString& name);

        Q_INVOKABLE void setGroupDescription(const QString& groupName, const QString& description);
        Q_INVOKABLE void setGroupMandatory(const QString& groupName, bool value);

        Q_INVOKABLE void setGroupMonthDays(const QString& groupName, const QList<int>& monthDays);
        Q_INVOKABLE void setGroupWeekDays(const QString& groupName, const QList<int>& weekDays);
        //setEveryday - подумать над daysFreq полем, может его сделать bool

        Q_INVOKABLE void setFieldToGroup(const QString& groupName, const QString& fieldName,
                                         const QString& fieldType);

        Q_INVOKABLE void setEnumFieldToGroup(const QString& groupName, const QString& fieldName,
                                             const QString& enumName);

        Q_INVOKABLE void setRangeFieldToGroup(const QString& groupName, const QString& fieldName,
                                              int rangeMin, int rangeMax);

        Q_INVOKABLE void removeGroupField(const QString& groupName, const QString& fieldName);

    private:

        void clear();
        void loadFromJsonObject(const QJsonObject& rootObject);

        bool isItGroupDay(const QDate& date, const QString& name) const;

        void addEnums(const QJsonArray& enumsArray);
        void addGroups(const QJsonArray& groupsArray);

        void fillGroupProperties(CardGroup& cardGroup,
                                 const QJsonObject& groupObject) const;

        void fillFieldProperties(CardField& cardField,
                                 const QJsonObject& fieldObject) const;

        bool hasRootErros(const QJsonDocument& doc) const;

        bool isFieldMissing(const QString& group, const QString& field) const;

        QString readTextFile(const QString& filename) const;


        std::unordered_map<std::string, CardEnum> _enums;
        std::unordered_map<std::string, CardGroup> _groups;

        QString _cardName;
        QString _cardDescription;

    };

}


#endif // DIARYCARDENGINE_HPP
