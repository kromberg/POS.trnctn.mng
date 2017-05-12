# retailnext.task

## Creating POS Transactions Manager
Manager shall be created with base currency specified

```c++
POSTransactionManager mng("USD");
```

## Adding Exchange Rates
One of currencies that are passed to the methods shall be baseCurrency

```c++
// Adding rate for [fromDate, toDate)
template<class T1, class T2>
Result addExchangeRate(
    T1&& fromCurrency,
    T2&& toCurrency,
    const time_t fromDate,
    const time_t toDate,
    double rate);
// Adding rate for [fromDate, +infinity)
template<class T1, class T2>
Result addExchangeRate(
    T1&& fromCurrency,
    T2&& toCurrency,
    const time_t fromDate,
    double rate);
```

### Example
```c++
std::string baseCurrency("USD");
std::string currency("RUR");
POSTransactionManager mng(baseCurrency);
// USD->RUB [2000-1-1; 2000-1-2) -> 100
mng.addExchangeRate(
    baseCurrency, currecy,
    timeFromString("2000-1-1 00:00:00"), timeFromString("2000-1-2 00:00:00"),
    100.);
// RUB->USD [2000-2-1; 2000-3-1) -> 0.011
mng.addExchangeRate(
    currecy, baseCurrency,
    timeFromString("2000-2-1 00:00:00"), timeFromString("2000-3-1 00:00:00"),
    0.011);
// RUB->USD [2000-3-1; +infinity) -> 0.012
mng.addExchangeRate(
    currecy, baseCurrency,
    timeFromString("2000-3-1 00:00:00"),
    0.012);
// error
mng.addExchangeRate(
    "EUR", "RUR",
    timeFromString("2000-2-1 00:00:00"), timeFromString("2000-3-1 00:00:00"),
    0.11);
```

## Currncy trend can be copied for output or other reasons
// get copy of currency trend
CurrencyTrendMap getExchangeRates() const;


## Converting POS Transactions

```c++
template<class T>
Result convertPOSTransaction(
    POSTransaction& toPosTransaction,
    const POSTransaction& fromPosTransaction,
    T&& toCurrency) const;
```

### Example
```c++
std::string baseCurrency("USD");
std::string currency1("RUR");
std::string currency2("EUR");
std::string currency3("GBP");
POSTransactionManager mng(baseCurrency);
// USD->RUB [2000-1-1; 2000-2-1) -> 100
mng.addExchangeRate(
    baseCurrency, currency1,
    timeFromString("2000-1-1 00:00:00"), timeFromString("2000-2-1 00:00:00"),
    100.);
// USD->EUR [2000-1-1; 2000-2-1) -> 1.1
mng.addExchangeRate(
    baseCurrency, currency2,
    timeFromString("2000-1-1 00:00:00"), timeFromString("2000-2-1 00:00:00"),
    1.1);

{
    // USD -> RUR
    POSTransaction fromTransaction = {100, baseCurrency, timeFromString("2000-1-15 00:00:00")};
    POSTransaction toTransaction;
    Result res = mng.convertPOSTransaction(
        toTransaction,
        fromTransaction,
        currency1);
    // toTransaction.m_total == 100 * 100.
}

{
    // RUR -> EUR
    POSTransaction fromTransaction = {100, currency1, timeFromString("2000-1-15 00:00:00")};
    POSTransaction toTransaction;
    Result res = mng.convertPOSTransaction(
        toTransaction,
        fromTransaction,
        currency2);
    // toTransaction.m_total == 100 / 100. * 1.1
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
}
```

## Possible results

SUCCESS - success
INVALID_DATE - invalid date(s) specified (from >= to)
CURRENCY_NOT_MATCH - one of currencies does not match base one
SAME_CURRECY - same currency specified while adding rate
NO_CURRENCY - no currency found for conversion in manager
NO_RATE - no rate found for conversion in manager