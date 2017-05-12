#include <vector>
#include <list>
#include <cstdlib>
#include <iostream>
#include <tuple>

#include <POSTransaction.h>
#include "TestUtils.h"

namespace pos
{
namespace test
{

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
    // not valid currency
    res = mng.addExchangeRate(
        currecy1, currecy1,
        timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-2 00:00:00"),
        1);
    TC_REQUIRE(Result::CURRENCY_NOT_MATCH == res);

    // valid date
    // not valid currency
    res = mng.addExchangeRate(
        currecy1, currecy2,
        timeFromString("2000-1-1 00:00:00"),
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
        for (i = 0; i < 29; ++i)
        {
            double rate = i + rand() % 1000 / 1000.;
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(i + 1) + " 00:00:00");
            rateList.emplace_back(fromDate, toDate, rate);
            rateTrendToCheck.emplace(fromDate, 1 / rate);
        }
        double rate = i + rand() % 1000 / 1000.;
        time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
        rateList.emplace_back(fromDate, rate);
        rateTrendToCheck.emplace(fromDate, 1 / rate);
        
        fillPOSTransactionManager(mng, currency1, baseCurrency, rateList);
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
        for (i = 28; i >= 1; --i)
        {
            double rate = i + rand() % 1000 / 1000.;
            time_t fromDate = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
            time_t toDate = timeFromString("2000-1-" + std::to_string(i + 1) + " 00:00:00");
            rateList.emplace_back(fromDate, toDate, rate);
            rateTrendToCheck.emplace(fromDate, rate);
        }
        i = 29;
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

void tc_convertPOSTransactionBaseOther()
{
    static constexpr size_t monthsCount = 5;

    std::string baseCurrency("USD");
    std::string currency1("RUR");
    std::string currency2("EUR");
    POSTransactionManager mng(baseCurrency);

    RateList rateList;
    double rate[monthsCount];

    // the first month
    int month = 1;

    // the second month
    ++ month;
    time_t fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
    time_t toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
    rate[month - 1] = month - 1 + rand() % 1000 / 1000.;
    rateList.emplace_back(fromDate, toDate, rate[month - 1]);

    // the third month
    ++ month;

    // the fourth month
    ++ month;
    fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
    toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
    rate[month - 1] = month - 1 + rand() % 1000 / 1000.;
    rateList.emplace_back(fromDate, toDate, rate[month - 1]);

    // the fifth month
    ++ month;
    fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
    toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
    rateList.emplace_back(fromDate, toDate, -1);

    fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);

    Result monthResults[monthsCount] =
    {
        Result::NO_RATE,
        Result::SUCCESS,
        Result::NO_RATE,
        Result::SUCCESS,
        Result::NO_RATE,
    };

    for (int month = 0; month < monthsCount; ++month)
    {
        for (int i = 1; i < 29; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-" + std::to_string(month + 1) + "-" + std::to_string(i) + " 00:00:00");
            {
                // base -> other
                POSTransaction fromTransaction =
                    {total, baseCurrency, date};
                POSTransaction toTransaction;
                Result res = mng.convertPOSTransaction(
                    toTransaction,
                    fromTransaction,
                    currency1);
                TC_REQUIRE(monthResults[month] == res);
                if (Result::SUCCESS == res)
                {
                    TC_REQUIRE(toTransaction.m_total == total * rate[month]);
                    TC_REQUIRE(toTransaction.m_currency == currency1);
                    TC_REQUIRE(toTransaction.m_date == date);
                }
            }
            {
                // other -> base
                POSTransaction fromTransaction =
                    {total, currency1, date};
                POSTransaction toTransaction;
                Result res = mng.convertPOSTransaction(
                    toTransaction,
                    fromTransaction,
                    baseCurrency);
                TC_REQUIRE(monthResults[month] == res);
                if (Result::SUCCESS == res)
                {
                    TC_REQUIRE(toTransaction.m_total == total / rate[month]);
                    TC_REQUIRE(toTransaction.m_currency == baseCurrency);
                    TC_REQUIRE(toTransaction.m_date == date);
                }
            }
        }
    }

    for (int i = 1; i < 29; ++i)
    {
        double total = rand() % 2000 / 1000.;
        time_t date = timeFromString("2000-1-" + std::to_string(i) + " 00:00:00");
        {
            POSTransaction fromTransaction =
                {total, baseCurrency, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency2);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
        {
            POSTransaction fromTransaction =
                {total, currency2, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                baseCurrency);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
    }
}

void tc_convertPOSTransactionOtherOther()
{
    static constexpr size_t monthsCount = 5;

    std::string baseCurrency("USD");
    std::string currency1("RUR");
    std::string currency2("EUR");
    std::string currency3("GBP");
    POSTransactionManager mng(baseCurrency);

    std::pair<double, double> rate[monthsCount];
    {
        // currency #1
        RateList rateList;

        // the first month
        int month = 1;

        // the second month
        ++ month;
        time_t fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
        time_t toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
        rate[month - 1].first = month - 1 + rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate[month - 1].first);

        // the third month
        ++ month;
        fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
        toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
        rate[month - 1].first = month - 1 + rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate[month - 1].first);

        // the fourth month
        ++ month;

        // the fifth month
        ++ month;
        fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
        toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
        rateList.emplace_back(fromDate, toDate, -1);

        fillPOSTransactionManager(mng, baseCurrency, currency1, rateList);
    }

    {
        // currency #2
        RateList rateList;

        // the first month
        int month = 1;

        // the second month
        ++ month;

        // the third month
        ++ month;
        time_t fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
        time_t toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
        rate[month - 1].second = month - 1 + rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate[month - 1].second);

        // the fourth month
        ++ month;
        fromDate = timeFromString("2000-" + std::to_string(month) + "-1 00:00:00");
        toDate = timeFromString("2000-" + std::to_string(month + 1) + "-1 00:00:00");
        rate[month - 1].second = month - 1 + rand() % 1000 / 1000.;
        rateList.emplace_back(fromDate, toDate, rate[month - 1].second);

        // the fifth month
        ++ month;

        fillPOSTransactionManager(mng, baseCurrency, currency2, rateList);
    }


    Result monthResults[monthsCount] =
    {
        Result::NO_RATE,
        Result::NO_RATE,
        Result::SUCCESS,
        Result::NO_RATE,
        Result::NO_RATE,
    };

    for (int month = 0; month < monthsCount; ++month)
    {
        for (int i = 1; i < 29; ++i)
        {
            double total = rand() % 2000 / 1000.;
            time_t date = timeFromString("2000-" + std::to_string(month + 1) + "-" + std::to_string(i) + " 00:00:00");
            {
                // base -> other
                POSTransaction fromTransaction =
                    {total, currency1, date};
                POSTransaction toTransaction;
                Result res = mng.convertPOSTransaction(
                    toTransaction,
                    fromTransaction,
                    currency2);
                TC_REQUIRE(monthResults[month] == res);
                if (Result::SUCCESS == res)
                {
                    TC_REQUIRE(toTransaction.m_total == total / rate[month].first * rate[month].second);
                    TC_REQUIRE(toTransaction.m_currency == currency2);
                    TC_REQUIRE(toTransaction.m_date == date);
                }
            }
            {
                // other -> base
                POSTransaction fromTransaction =
                    {total, currency2, date};
                POSTransaction toTransaction;
                Result res = mng.convertPOSTransaction(
                    toTransaction,
                    fromTransaction,
                    currency1);
                TC_REQUIRE(monthResults[month] == res);
                if (Result::SUCCESS == res)
                {
                    TC_REQUIRE(toTransaction.m_total == total / rate[month].second * rate[month].first);
                    TC_REQUIRE(toTransaction.m_currency == currency1);
                    TC_REQUIRE(toTransaction.m_date == date);
                }
            }
        }
    }

    for (int i = 1; i < 29; ++i)
    {
        double total = rand() % 2000 / 1000.;
        time_t date = timeFromString("2000-3-" + std::to_string(i) + " 00:00:00");
        {
            POSTransaction fromTransaction =
                {total, currency1, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency3);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
        {
            POSTransaction fromTransaction =
                {total, currency2, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency3);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
        {
            POSTransaction fromTransaction =
                {total, currency3, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
        {
            POSTransaction fromTransaction =
                {total, currency3, date};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency2);
            TC_REQUIRE(Result::NO_CURRENCY == res);
        }
    }
}

static std::vector<TestCase> tests =
{
    TEST_CASE(tc_init),
    TEST_CASE(tc_addExchangeRateFailed),
    TEST_CASE(tc_addExchangeRateSuccess),
    TEST_CASE(tc_convertPOSTransactionOtherToSame),
    TEST_CASE(tc_convertPOSTransactionBaseOther),
    TEST_CASE(tc_convertPOSTransactionOtherOther),
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