#ifndef TESTSENGINE_HPP
#define TESTSENGINE_HPP

#include <QObject>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <vector>


namespace diaryth {


    struct TestOption
    {
        QString text;
        qreal rate;

        QString description;
    };


    struct TestQuestion
    {
        QString text;
        QString type; //radio, check, input

        int groupId = -1; //for alt-question

        std::vector<TestOption> options;
        //map? optionName->rate ? for fast rating
    };


    struct TestRate
    {
        QString text;

        qreal rangeBegin;
        qreal rangeEnd;

        QString description;

        bool isInRange(qreal rate) const
        {
            return rate >= rangeBegin &&
                   rate <= rangeEnd;
        }
    };



    class TestsEngine : public QObject
    {
        Q_OBJECT

    public:
        TestsEngine() = default;

        Q_INVOKABLE void parseJSON(const QString& json);
        Q_INVOKABLE void parseFromFile(const QString& filename);

        Q_INVOKABLE QString getName() { return _testName; }
        Q_INVOKABLE QString getDescription() { return _testDescription; }


        Q_INVOKABLE int questionsCount() { return _questions.size(); }

        Q_INVOKABLE QString getQuestionText(int idx) { return _questions[idx].text; }
        Q_INVOKABLE QString getQuestionType(int idx) { return _questions[idx].type; }
        Q_INVOKABLE int getQuestionGroupId(int idx) { return _questions[idx].groupId; }

        Q_INVOKABLE int getOptionsCount(int questionIdx) { return _questions[questionIdx].options.size(); }

        Q_INVOKABLE QStringList getOptionsTexts(int questionIdx);
        Q_INVOKABLE QVariantList getOptions(int questionIdx);


        Q_INVOKABLE void answerQuestion(int idx, QString option);
        Q_INVOKABLE void answerCheckQuestion(int idx, QStringList options);
        Q_INVOKABLE QStringList getAnswers(int idx);


        Q_INVOKABLE int questionsLeft();
        Q_INVOKABLE bool isTestFinished();
        Q_INVOKABLE qreal testCurrentRate();

        Q_INVOKABLE QString getRateName();
        Q_INVOKABLE QString getRateDescription();

    private:

        void addQuestions(const QJsonArray& questions);
        void addOptions(TestQuestion& question, const QJsonArray& options);
        void addRates(const QJsonArray& rates);

        bool hasRootErros(const QJsonDocument& root);

        QString readTextFile(const QString& filename);

        QString _testName;
        QString _testDescription;

        std::vector<TestQuestion> _questions;
        std::vector<TestRate> _rates;
        //_subRates -> Question list, way to summate
        //Или возможно другой метод задать рейты - разные типы рейтов и разные методы подсчёта

        std::vector<QStringList> _answers;
    };


}


#endif // TESTSENGINE_HPP
