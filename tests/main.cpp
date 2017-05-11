#include <vector>
#include <list>
#include <cstdlib>
#include <iostream>
#include <tuple>

#include <POSTransaction.h>


namespace pos
{
namespace test
{

struct TestException
{
    const char* m_file;
    int32_t m_line;
    TestException(const char* file, const int32_t line):
        m_file(file), m_line(line)
    {}
};

#define TC_REQUIRE(EXPR) \
if (!(EXPR))\
{\
    throw TestException(__FILE__, __LINE__); \
}


#define TC_REQUIRE_NO_THROW(EXPR) \
try\
{\
    EXPR;\
}\
catch (...)\
{\
    throw TestException(__FILE__, __LINE__);\
}

#define TC_REQUIRE_THROW(EXPR, TYPE) \
{\
    bool thrown = false;\
    try\
    {\
        EXPR;\
    }\
    catch (const TYPE& e)\
    {\
        thrown = true;\
    }\
    if (!thrown)\
    {\
        throw TestException(__FILE__, __LINE__);\
    }\
}

time_t timeFromString(const std::string& str)
{
    struct tm timeStruct = {0};
    strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct);
    return mktime(&timeStruct);
}

struct RateEntity
{
    time_t m_from;
    bool m_toSet = false;
    time_t m_to;
    double m_rate;
    RateEntity(const time_t from, const double rate):
        m_from(from), m_rate(rate)
    {}
    RateEntity(const time_t from, const time_t to, const double rate):
        m_from(from), m_toSet(true), m_to(to), m_rate(rate)
    {}
};
typedef std::list<RateEntity> RateList;

static void fillPOSTransactionManager(
    POSTransactionManager& mng,
    const std::string& baseCurrency,
    const std::string& currency,
    const RateList& rateList)
{
    Result res;
    for (const auto& rate : rateList)
    {
        if (rate.m_toSet)
        {
            res = mng.addExchangeRate(baseCurrency, currency, rate.m_from, rate.m_to, rate.m_rate);
        }
        else
        {
            res = mng.addExchangeRate(baseCurrency, currency, rate.m_from, rate.m_rate);
        }
        TC_REQUIRE(Result::SUCCESS == res);
    }
}

void tc_init()
{
    TC_REQUIRE_NO_THROW(POSTransactionManager mng("USD"));
    TC_REQUIRE_NO_THROW(POSTransactionManager mng("some random long long string"));
    {
        std::string currency = "USD";
        TC_REQUIRE_NO_THROW(POSTransactionManager mng(currency));
        TC_REQUIRE_NO_THROW(POSTransactionManager mng(std::move(currency)));
    }
    {
        std::string currency = "some random long long string";
        TC_REQUIRE_NO_THROW(POSTransactionManager mng(currency));
        TC_REQUIRE_NO_THROW(POSTransactionManager mng(std::move(currency)));
    }

    {
        std::string emptyString;
        TC_REQUIRE_THROW(POSTransactionManager mng(emptyString), std::runtime_error);
    }
}

void tc_addExchangeRateFailed()
{
    std::string baseCurrency("USD");
    std::string currecy1("EUR");
    std::string currecy2("RUB");
    POSTransactionManager mng(baseCurrency);

    // valid dates
    // not valid currency
    Result res = mng.addExchangeRate(
        currecy1, currecy2,
        timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-2 00:00:00"),
        1);
    TC_REQUIRE(Result::CURRENCY_NOT_MATCH == res);

    // valid dates
    // same currency
    res = mng.addExchangeRate(
        baseCurrency, baseCurrency,
        timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-2 00:00:00"),
        1);
    TC_REQUIRE(Result::SAME_CURRECY == res);

    // not valid dates (from > to)
    // valid currency
    res = mng.addExchangeRate(
        baseCurrency, currecy1,
        timeFromString("2000-1-2 00:00:00"), timeFromString("2000-1-1 00:00:00"),
        1);
    TC_REQUIRE(Result::INVALID_DATE == res);

    // not valid dates (from == to)
    // valid currency
    res = mng.addExchangeRate(
        baseCurrency, currecy1,
        timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-1 00:00:00"),
        1);
    TC_REQUIRE(Result::INVALID_DATE == res);
}

void tc_addExchangeRateSuccess()
{
    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        POSTransactionManager mng(baseCurrency);

        RateList rateList;
        POSTransactionManager::RateTrend rateTrendToCheck;
        int i;
        for (i = 0; i < 29; ++i)
        {
            double rate = i + rand() % 1000 / 1000.;
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(i + 1) + " 00:00:00");
            rateList.emplace_back(fromDate, toDate, rate);
            rateTrendToCheck.emplace(fromDate, rate);
        }
        double rate = i + rand() % 1000 / 1000.;
        time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
        rateList.emplace_back(fromDate, rate);
        rateTrendToCheck.emplace(fromDate, rate);
        
        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
        auto currencyTrendMap = mng.getExchangeRates();
        auto currencyIt = currencyTrendMap.find(currency1);
        TC_REQUIRE(currencyTrendMap.end() != currencyIt);
        TC_REQUIRE(currencyIt->second == rateTrendToCheck);
    }

    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        POSTransactionManager mng(baseCurrency);


        RateList rateList;
        POSTransactionManager::RateTrend rateTrendToCheck;
        int i;
        for (i = 0; i < 30; ++i)
        {
            double rate = i + rand() % 1000 / 1000.;
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            rateList.emplace_back(fromDate, rate);
            rateTrendToCheck.emplace(fromDate, rate);
        }
        
        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
        auto currencyTrendMap = mng.getExchangeRates();
        auto currencyIt = currencyTrendMap.find(currency1);
        TC_REQUIRE(currencyTrendMap.end() != currencyIt);
        TC_REQUIRE(currencyIt->second == rateTrendToCheck);
    }

    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        POSTransactionManager mng(baseCurrency);

        RateList rateList;
        POSTransactionManager::RateTrend rateTrendToCheck;
        double firstLastRate = rand() % 1000 / 1000.;
        time_t fromDate = timeFromString("2000-1-1 00:00:00");
        time_t toDate = timeFromString("2000-1-5 00:00:00");
        rateList.emplace_back(fromDate, toDate, firstLastRate);
        rateTrendToCheck.emplace(fromDate, firstLastRate);
        for (int i = 5; i < 24; ++i)
        {
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(i + 1) + " 00:00:00");
            double rate = i + rand() % 1000 / 1000.;
            rateList.emplace_back(fromDate, toDate, rate);
        }
        fromDate = timeFromString("2000-1-24 00:00:00");
        toDate = timeFromString("2000-1-30 00:00:00");
        rateList.emplace_back(fromDate, toDate, firstLastRate);
        rateTrendToCheck.emplace(toDate, -1);

        fromDate = timeFromString("2000-1-2 00:00:00");
        toDate = timeFromString("2000-1-25 00:00:00");
        rateList.emplace_back(fromDate, toDate, firstLastRate);

        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
        auto currencyTrendMap = mng.getExchangeRates();
        auto currencyIt = currencyTrendMap.find(currency1);
        TC_REQUIRE(currencyTrendMap.end() != currencyIt);
        TC_REQUIRE(currencyIt->second == rateTrendToCheck);
    }

    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        POSTransactionManager mng(baseCurrency);

        RateList rateList;
        POSTransactionManager::RateTrend rateTrendToCheck;
        for (int i = 5; i < 24; ++i)
        {
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(i + 1) + " 00:00:00");
            
            double rate = i + rand() % 1000 / 1000.;
            rateList.emplace_back(fromDate, toDate, rate);
        }

        time_t fromDate = timeFromString("2000-1-2 00:00:00");
        time_t toDate = timeFromString("2000-1-25 00:00:00");
        double rate = rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate);
        rateTrendToCheck.emplace(fromDate, rate);
        rateTrendToCheck.emplace(toDate, -1);

        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
        auto currencyTrendMap = mng.getExchangeRates();
        auto currencyIt = currencyTrendMap.find(currency1);
        TC_REQUIRE(currencyTrendMap.end() != currencyIt);
        TC_REQUIRE(currencyIt->second == rateTrendToCheck);
    }

    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        POSTransactionManager mng(baseCurrency);

        RateList rateList;
        POSTransactionManager::RateTrend rateTrendToCheck;
        int i;
        for (i = 1; i < 14; ++i)
        {
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(30 - i) + " 00:00:00");
            time_t toDateMinusOne = timeFromString("2000-1-" + std::to_string(30 - i - 1) + " 00:00:00");
            double rate = i + rand() % 1000 / 1000.;
            rateList.emplace_back(fromDate, toDate, rate);
            rateTrendToCheck.emplace(fromDate, rate);
            rateTrendToCheck.emplace(toDateMinusOne, rate);
        }
        time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
        time_t toDate = timeFromString("2000-1-" + std::to_string(30 - i) + " 00:00:00");
        double rate = i + rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate);
        rateTrendToCheck.emplace(fromDate, rate);

        fromDate = timeFromString("2000-1-29 00:00:00");
        rateTrendToCheck.emplace(fromDate, -1);

        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
        auto currencyTrendMap = mng.getExchangeRates();
        auto currencyIt = currencyTrendMap.find(currency1);
        TC_REQUIRE(currencyTrendMap.end() != currencyIt);
        TC_REQUIRE(currencyIt->second == rateTrendToCheck);
    }
}

void tc_convertPOSTransactionOtherToSame()
{
    std::string baseCurrency("USD");

    POSTransactionManager mng(baseCurrency);

    std::vector<std::string> currencys = { "RUR", "EUR", "GBP"};

    for (auto& currency : currencys)
    {
        const size_t cyclesCount = 100 + rand() % 100;
        for (size_t i = 0; i < cyclesCount; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-" + std::to_string(1 + rand() % 12) + "-" + std::to_string(1 + rand() % 29) + " 00:00:00");
            POSTransaction fromTransaction =
                {total, currency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency);
            TC_REQUIRE(Result::SUCCESS == res);
            TC_REQUIRE(toTransaction.m_total == total);
            TC_REQUIRE(toTransaction.m_currency == currency);
            TC_REQUIRE(toTransaction.m_date == date);
        }
    }
}

void tc_convertPOSTransactionBaseToOther()
{
    std::string baseCurrency("USD");
    std::string currency1("RUR");
    std::string currency2("EUR");
    POSTransactionManager mng(baseCurrency);

    RateList rateList;

    // the first month
    time_t fromDate = timeFromString("2000-1-1 00:00:00");
    time_t toDate = timeFromString("2000-2-1 00:00:00");
    double rate1 = 1 + rand() % 1000 / 1000.;
    rateList.emplace_back(fromDate, toDate, rate1);

    // the third month
    fromDate = timeFromString("2000-3-1 00:00:00");
    toDate = timeFromString("2000-4-1 00:00:00");
    double rate3 = 3 + rand() % 1000 / 1000.;
    rateList.emplace_back(fromDate, toDate, rate3);

    // the fourth month
    fromDate = timeFromString("2000-4-1 00:00:00");
    toDate = timeFromString("2000-5-1 00:00:00");
    rateList.emplace_back(fromDate, toDate, -1);

    fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);

    {
        // the first month
        for (int i = 1; i < 31; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            POSTransaction fromTransaction =
                {total, baseCurrency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            TC_REQUIRE(Result::SUCCESS == res);
            TC_REQUIRE(toTransaction.m_total == total * rate1);
            TC_REQUIRE(toTransaction.m_currency == currency1);
            TC_REQUIRE(toTransaction.m_date == date);
        }

        // the second month
        for (int i = 1; i < 29; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-2-" + std::to_string(i) + " 00:00:00");
            POSTransaction fromTransaction =
                {total, baseCurrency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            TC_REQUIRE(Result::NO_RATE == res);
        }

        // the third month
        for (int i = 1; i < 31; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-3-" + std::to_string(i) + " 00:00:00");
            POSTransaction fromTransaction =
                {total, baseCurrency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            TC_REQUIRE(Result::SUCCESS == res);
            TC_REQUIRE(toTransaction.m_total == total * rate3);
            TC_REQUIRE(toTransaction.m_currency == currency1);
            TC_REQUIRE(toTransaction.m_date == date);
        }

        // the fourth month
        for (int i = 1; i < 31; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-4-" + std::to_string(i) + " 00:00:00");
            POSTransaction fromTransaction =
                {total, baseCurrency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            TC_REQUIRE(Result::NO_RATE == res);
        }
    }
}

typedef std::function<void(void)> TestCaseFunc;
struct TestCase
{
    std::string m_name;
    TestCaseFunc m_func;
};
static std::vector<TestCase> tests =
{
    { "init", tc_init },
    { "addExchangeRateFailed", tc_addExchangeRateFailed },
    { "addExchangeRate", tc_addExchangeRateSuccess },
    { "convertPOSTransactionOtherToSame", tc_convertPOSTransactionOtherToSame },
    { "convertPOSTransactionBaseToOther", tc_convertPOSTransactionBaseToOther },
};

} // namespace test
} // namespace pos

using namespace pos::test;

int main(int agrc, char* argv[])
{
    for (auto& test : tests)
    {
        try
        {
            fprintf(stderr, "Running '%s' testcase\n", test.m_name.c_str());
            test.m_func();
        }
        catch (const TestException& e)
        {
            fprintf(stderr, "'%s' testcase failed (%s:%d)\n", test.m_name.c_str(),
                e.m_file, e.m_line);
        }
    }
    return 0;
}