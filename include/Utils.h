#ifndef POS_UTILS_H
#define POS_UTILS_H

#include <ctime>
#include <string>

namespace pos
{
enum class Result : uint16_t
{
    SUCCESS,
    INVALID_DATE,
    CURRENCY_NOT_MATCH,
    SAME_CURRECY,
    NO_CURRENCY,
    NO_RATE,
};

const char* resultToStr(const Result r);

time_t timeFromString(const std::string& str);
std::string timeToString(const time_t t);

} // namespace pos

#endif // POS_UTILS_H