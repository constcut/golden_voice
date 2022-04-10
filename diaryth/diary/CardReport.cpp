#include "CardReport.hpp"

using namespace diaryth;

CardReport::CardReport(const SQLBase &database,
                       const DiaryCardEngine &cardEngine)
    :_database(database),
    _cardEngine(cardEngine)
{
}
