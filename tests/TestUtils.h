#ifndef TEST_UTILS_H
#define TEST_UTILS_H


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

typedef std::function<void(void)> TestCaseFunc;
struct TestCase
{
    std::string m_name;
    TestCaseFunc m_func;
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

#define TEST_CASE(NAME)\
    { #NAME, NAME }

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
    const std::string& currency1,
    const std::string& currency2,
    const RateList& rateList)
{
    Result res;
    for (const auto& rate : rateList)
    {
        if (rate.m_toSet)
        {
            res = mng.addExchangeRate(currency1, currency2, rate.m_from, rate.m_to, rate.m_rate);
        }
        else
        {
            res = mng.addExchangeRate(currency1, currency2, rate.m_from, rate.m_rate);
        }
        TC_REQUIRE(Result::SUCCESS == res);
    }
}
} // namespace test
} // namespace pos
#endif // TEST_UTILS_Hs