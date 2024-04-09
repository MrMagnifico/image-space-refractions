#ifndef _NUMERICAL_UTILS_HPP_
#define _NUMERICAL_UTILS_HPP_

#include <utils/constants.h>

#include <cmath>

namespace utils {
    float zeroWithinEpsilon(float val) { return std::abs(val) < utils::ZERO_EPSILON; }

    float linearMap(float val, float domainMin, float domainMax, float rangeMin, float rangeMax) {
        float ratio         = (val - domainMin) / (domainMax - domainMin);
        float scaledValue   = ratio * (rangeMax - rangeMin);
        float mappedValue   = scaledValue + rangeMin;
        return mappedValue;
    }

    template<typename T>
    inline bool inRangeInclusive(T val, T low, T high) { return low <= val && val <= high; }
}


#endif // _NUMERICAL_UTILS_HPP_
