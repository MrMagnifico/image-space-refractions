#include "numerical_utils.h"

float utils::zeroWithinEpsilon(float val) { return std::abs(val) < utils::ZERO_EPSILON; }

float utils::linearMap(float val, float domainMin, float domainMax, float rangeMin, float rangeMax) {
        float ratio         = (val - domainMin) / (domainMax - domainMin);
        float scaledValue   = ratio * (rangeMax - rangeMin);
        float mappedValue   = scaledValue + rangeMin;
        return mappedValue;
}
