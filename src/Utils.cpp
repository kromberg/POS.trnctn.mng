#include <Utils.h>

namespace pos
{
const char* resultToStr(const Result r)
{
    switch (r)
    {
        case Result::SUCCESS:
            return "Success";
        case Result::INVALID_DATE:
            return "Invalid date(s) specified (from >= to)";
        case Result::CURRENCY_NOT_MATCH:
            return "One of currencies does not match base one";
        case Result::SAME_CURRECY:
            return "Same currency specified while adding rate";
        case Result::NO_CURRENCY:
            return "No currency found for conversion in manager";
        case Result::NO_RATE:
            return  "No rate found for conversion in manager";
    }
    return "Unknown";
}

time_t timeFromString(const std::string& str)
{
    struct tm timeStruct = {0};
    strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct);
    return mktime(&timeStruct);
}

std::string timeToString(const time_t t)
{
    thread_local char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::gmtime(&t));
    return std::string(buf);
}

} // namespace pos