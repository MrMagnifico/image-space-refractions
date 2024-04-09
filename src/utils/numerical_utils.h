#pragma once
#ifndef _NUMERICAL_UTILS_H_
#define _NUMERICAL_UTILS_H_

#include <utils/constants.h>

#include <cmath>

namespace utils {
    float zeroWithinEpsilon(float val);
    float linearMap(float val, float domainMin, float domainMax, float rangeMin, float rangeMax);

    template<typename T>
    bool inRangeInclusive(T val, T low, T high) { return low <= val && val <= high; }
}


#endif // _NUMERICAL_UTILS_H_
