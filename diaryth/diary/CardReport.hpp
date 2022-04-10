#ifndef CARDREPORT_H
#define CARDREPORT_H

#include <QObject>

#include "diary/SQLBase.hpp"
#include "diary/DiaryCardEngine.hpp"


namespace diaryth {


    class CardReport : public QObject
    {
        Q_OBJECT

    public:
        explicit CardReport(const SQLBase& database,
                            const DiaryCardEngine& cardEngine);


        //exportByDate
        //exportByDatesRange

    private:

        const SQLBase& _database;
        const DiaryCardEngine& _cardEngine;

    };

}



#endif // CARDREPORT_H
