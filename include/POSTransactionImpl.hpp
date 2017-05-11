#ifndef POS_TRANSACTION_IMPL_HPP
#define POS_TRANSACTION_IMPL_HPP

namespace pos
{

template<class T>
POSTransactionManager::POSTransactionManager(T&& baseCurrency):
    m_baseCurrency(std::forward<T>(baseCurrency))
{
    if (m_baseCurrency.empty())
    {
        throw std::runtime_error("POSTransactionManager: currency cannot be empty");
    }
}

// get copy of currency trend
inline POSTransactionManager::CurrencyTrendMap POSTransactionManager::getExchangeRates() const
{
    std::unique_lock<std::mutex> l(m_currencyTrendMapGuard);
    return m_currencyTrendMap;
}

inline Result POSTransactionManager::checkCurrency(
    const std::string& fromCurrency,
    const std::string& toCurrency)
{
    if (m_baseCurrency != fromCurrency && m_baseCurrency != toCurrency)
    {
        return Result::CURRENCY_NOT_MATCH;
    }

    if (fromCurrency == toCurrency)
    {
        return Result::SAME_CURRECY;
    }
    return Result::SUCCESS;
}

template<class T1, class T2>
void POSTransactionManager::getCurrencyAndRate(
    std::string& currency,
    double& rate,
    T1&& fromCurrency,
    T2&& toCurrency)
{
    if (m_baseCurrency == toCurrency)
    {
        currency = std::forward<T1>(fromCurrency);
        rate = 1 / rate;
    }
    if (m_baseCurrency == fromCurrency)
    {
        currency = std::forward<T2>(toCurrency);
    }
}

POSTransactionManager::RateTrend::iterator POSTransactionManager::insertFromUnsafe(
    RateTrend& rateTrend,
    const time_t fromDate,
    const double rate)
{
    auto fromIt = rateTrend.end();
    auto nextFromIt = rateTrend.upper_bound(fromDate);
    if (rateTrend.begin() != nextFromIt)
    {
        auto prevFromIt = std::prev(nextFromIt);
        if (prevFromIt->second == rate)
        {
            fromIt = prevFromIt;
        }
    }
    if (rateTrend.end() == fromIt)
    {
        auto fromRes = rateTrend.emplace(fromDate, rate);
        if (!fromRes.second)
        {
            // value was not inserted. replace rate
            fromRes.first->second = rate;
        }
        fromIt = fromRes.first;
    }
    return fromIt;
}

POSTransactionManager::RateTrend::iterator POSTransactionManager::insertToUnsafe(
    RateTrend& rateTrend,
    const time_t toDate,
    const double rate)
{
    auto toIt = rateTrend.end();
    // get 'to' rate
    double toRate = -1;

    auto nextToIt = rateTrend.upper_bound(toDate);
    if (rateTrend.begin() != nextToIt)
    {
        // prev(nextToIt) date <= toDate. we need to save this currency value
        toRate = std::prev(nextToIt)->second;
        if (toRate == rate)
        {
            toIt = nextToIt;
        }
    }
    if (rateTrend.end() == toIt)
    {
        auto toRes = rateTrend.emplace(toDate, toRate);
        if (!toRes.second)
        {
            // value was not inserted. replace rate
            toRes.first->second = toRate;
        }
        toIt = toRes.first;
    }

    return toIt;
}

POSTransactionManager::RateTrend& POSTransactionManager::getCurrencyTrendUnsafe(
    std::string&& currency)
{
    auto currencyIt = m_currencyTrendMap.find(currency);
    if (m_currencyTrendMap.end() == currencyIt)
    {
        auto res = m_currencyTrendMap.emplace(std::move(currency), RateTrend());
        currencyIt = res.first;
    }

    return currencyIt->second;
}

template<class T1, class T2>
Result POSTransactionManager::addExchangeRate(
    T1&& fromCurrency,
    T2&& toCurrency,
    const time_t fromDate,
    const time_t toDate,
    double rate)
{
    Result r = checkCurrency(fromCurrency, toCurrency);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    if (fromDate >= toDate)
    {
        return Result::INVALID_DATE;
    }

    std::string currency;
    getCurrencyAndRate(currency, rate, fromCurrency, toCurrency);

    std::unique_lock<std::mutex> l(m_currencyTrendMapGuard);
    RateTrend& rateTrend = getCurrencyTrendUnsafe(std::move(currency));

    // empty trend. just insert
    if (rateTrend.empty())
    {
        rateTrend.emplace(fromDate, rate);
        rateTrend.emplace(toDate, -1);
        return Result::SUCCESS;
    }

    auto toIt = insertToUnsafe(rateTrend, toDate, rate);
    auto fromIt = insertFromUnsafe(rateTrend, fromDate, rate);

    // erase everything in (fromDate; toDate)
    rateTrend.erase(std::next(fromIt), toIt);
    return Result::SUCCESS;
}

template<class T1, class T2>
Result POSTransactionManager::addExchangeRate(
    T1&& fromCurrency,
    T2&& toCurrency,
    const time_t fromDate,
    double rate)
{
    Result r = checkCurrency(fromCurrency, toCurrency);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    std::string currency;
    getCurrencyAndRate(currency, rate, fromCurrency, toCurrency);

    std::unique_lock<std::mutex> l(m_currencyTrendMapGuard);
    RateTrend& rateTrend = getCurrencyTrendUnsafe(std::move(currency));

    // empty trend. just insert
    if (rateTrend.empty())
    {
        rateTrend.emplace(fromDate, rate);
        return Result::SUCCESS;
    }

    auto fromIt = insertFromUnsafe(rateTrend, fromDate, rate);

    // erase everything in (fromDate; end)
    rateTrend.erase(std::next(fromIt), rateTrend.end());
    return Result::SUCCESS;
}

template<class T>
Result POSTransactionManager::convertPOSTransaction(
    POSTransaction& toPosTransaction,
    const POSTransaction& fromPosTransaction,
    T&& toCurrency) const
{
    if (fromPosTransaction.m_currency == toCurrency)
    {
        toPosTransaction = fromPosTransaction;
        return Result::SUCCESS;
    }

    double fromRate = -1;
    if (m_baseCurrency == fromPosTransaction.m_currency)
    {
        fromRate = 1;
    }
    else
    {
        std::unique_lock<std::mutex> l(m_currencyTrendMapGuard);
        auto currencyIt = m_currencyTrendMap.find(fromPosTransaction.m_currency);
        if (m_currencyTrendMap.end() == currencyIt)
        {
            return Result::NO_CURRENCY;
        }
        const RateTrend& rateTrend = currencyIt->second;
        auto rateIt = rateTrend.upper_bound(fromPosTransaction.m_date);
        if (rateTrend.begin() == rateIt)
        {
            return Result::NO_RATE;
        }
        fromRate = std::prev(rateIt)->second;
    }
    if (fromRate <= 0)
    {
        return Result::NO_RATE;
    }

    double toRate = -1;
    if (m_baseCurrency == toCurrency)
    {
        toRate = 1;
    }
    else
    {
        std::unique_lock<std::mutex> l(m_currencyTrendMapGuard);
        auto currencyIt = m_currencyTrendMap.find(toCurrency);
        if (m_currencyTrendMap.end() == currencyIt)
        {
            return Result::NO_CURRENCY;
        }
        const RateTrend& rateTrend = currencyIt->second;
        auto rateIt = rateTrend.upper_bound(fromPosTransaction.m_date);
        if (rateTrend.begin() == rateIt)
        {
            return Result::NO_RATE;
        }
        toRate = std::prev(rateIt)->second;
    }
    if (toRate <= 0)
    {
        return Result::NO_RATE;
    }

    toPosTransaction.m_currency = std::forward<T>(toCurrency);
    toPosTransaction.m_date = fromPosTransaction.m_date;
    toPosTransaction.m_total = fromPosTransaction.m_total / fromRate * toRate;
    return Result::SUCCESS;
}
} // namespace pos

#endif // POS_TRANSACTION_IMPL_HPP