#ifndef POS_UTILS_H
#define POS_UTILS_H

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
} // namespace pos

#endif // POS_UTILS_H