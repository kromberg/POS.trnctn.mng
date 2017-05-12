#include <cstdio>
#include <cstdint>

#include <POSTransaction.h>

static void printTransaction(FILE* file, const pos::POSTransaction& transaction)
{
    fprintf(file, "\ttotal: %f\n\tcurrnecy: %s\n\ttime: %s\n",
        transaction.m_total, transaction.m_currency.c_str(), pos::timeToString(transaction.m_date).c_str());
}

int main(int argc, char* argv[])
{
    using namespace pos;

    {
        POSTransactionManager mng("USD");
    }

    {
        std::string baseCurrency("USD");
        std::string currency("RUR");
        POSTransactionManager mng(baseCurrency);
        // USD->RUB [2000-1-1; 2000-1-2) -> 100
        Result res = mng.addExchangeRate(
            baseCurrency, currency,
            timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-2 00:00:00"),
            100.);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
        // RUB->USD [2000-2-1; 2000-3-1) -> 0.011
        res = mng.addExchangeRate(
            currency, baseCurrency,
            timeFromString("2000-2-1 00:00:00"), timeFromString("2000-3-1 00:00:00"),
            0.011);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
        // RUB->USD [2000-3-1; +infinity) -> 0.012
        res = mng.addExchangeRate(
            currency, baseCurrency,
            timeFromString("2000-3-1 00:00:00"),
            0.012);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
        // error
        res = mng.addExchangeRate(
            "EUR", "RUR",
            timeFromString("2000-2-1 00:00:00"), timeFromString("2000-3-1 00:00:00"),
            0.11);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));

        POSTransactionManager::CurrencyTrendMap currenciesTrendMap = mng.getExchangeRates();
        for (const auto& currencyTrend : currenciesTrendMap)
        {
            fprintf(stdout, "Currency: '%s'. Trend:\n", currencyTrend.first.c_str());
            for (const auto& rate : currencyTrend.second)
            {
                fprintf(stdout, "\t%s -> %f\n", timeToString(rate.first).c_str(), rate.second);
            }
        }
    }

    {
        std::string baseCurrency("USD");
        std::string currency1("RUR");
        std::string currency2("EUR");
        std::string currency3("GBP");
        POSTransactionManager mng(baseCurrency);
        // USD->RUB [2000-1-1; 2000-2-1) -> 100
        Result res = mng.addExchangeRate(
            baseCurrency, currency1,
            timeFromString("2000-1-1 00:00:00"), timeFromString("2000-2-1 00:00:00"),
            100.);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));

        // USD->EUR [2000-1-1; 2000-2-1) -> 1.1
        res = mng.addExchangeRate(
            baseCurrency, currency2,
            timeFromString("2000-1-1 00:00:00"), timeFromString("2000-2-1 00:00:00"),
            1.1);
        fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));

        {
            // USD -> RUR
            POSTransaction fromTransaction = {100, baseCurrency, timeFromString("2000-1-15 00:00:00")};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency1);
            fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
            fprintf(stdout, "From transaction:\n");
            printTransaction(stdout, fromTransaction);
            fprintf(stdout, "To transaction:\n");
            printTransaction(stdout, toTransaction);
        }

        {
            // RUR -> EUR
            POSTransaction fromTransaction = {100, currency1, timeFromString("2000-1-15 00:00:00")};
            POSTransaction toTransaction;
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency2);
            fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
            fprintf(stdout, "From transaction:\n");
            printTransaction(stdout, fromTransaction);
            fprintf(stdout, "To transaction:\n");
            printTransaction(stdout, toTransaction);
        }

        {
            // RUR -> GBP
            POSTransaction fromTransaction = {100, currency1, timeFromString("2000-1-15 00:00:00")};
            POSTransaction toTransaction;
            // error
            Result res = mng.convertPOSTransaction(
                toTransaction,
                fromTransaction,
                currency3);
            fprintf(stdout, "Result: %u(%s)\n", static_cast<uint32_t>(res), resultToStr(res));
        }
    }
    return 0;
}