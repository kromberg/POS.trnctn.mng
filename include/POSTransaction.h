#ifndef POS_TRANSACTION_H
#define POS_TRANSACTION_H

#include <stdexcept>
#include <ctime>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>

#include "Utils.h"

namespace pos
{

struct POSTransaction
{
    double m_total;
    std::string m_currency;
    time_t m_date;
};

class POSTransactionManager
{
public:
    typedef std::map<time_t, double> RateTrend;
    typedef std::unordered_map<std::string, RateTrend> CurrencyTrendMap;

protected:
    std::string m_baseCurrency;
    CurrencyTrendMap m_currencyTrendMap;
    mutable std::mutex m_currencyTrendMapGuard;

private:
    Result checkCurrency(const std::string& fromCurrency, const std::string& toCurrency);
    template<class T1, class T2>
    void getCurrencyAndRate(
        std::string& currency,
        double& rate,
        T1&& fromCurrency,
        T2&& toCurrency);
    RateTrend& getCurrencyTrendUnsafe(std::string&& currency);

    RateTrend::iterator insertFromUnsafe(RateTrend& rateTrend, const time_t fromDate, const double rate);
    RateTrend::iterator insertToUnsafe(RateTrend& rateTrend, const time_t toDate, const double rate);

public:
    template<class T>
    POSTransactionManager(T&& baseCurrency);

    template<class T1, class T2>
    Result addExchangeRate(
        T1&& fromCurrency,
        T2&& toCurrency,
        const time_t fromDate,
        const time_t toDate,
        double rate);
    template<class T1, class T2>
    Result addExchangeRate(
        T1&& fromCurrency,
        T2&& toCurrency,
        const time_t fromDate,
        double rate);
    // get copy of currency trend
    CurrencyTrendMap getExchangeRates() const;
    template<class T>
    Result convertPOSTransaction(
        POSTransaction& toPosTransaction,
        const POSTransaction& fromPosTransaction,
        T&& toCurrency) const;
};
} // namespace pos

#include "POSTransactionImpl.hpp"

#endif // POS_TRANSACTION_H