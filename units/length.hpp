#ifndef UNIT_LENGTH_HPP
#define UNIT_LENGTH_HPP

#include "unit.hpp"

namespace units {

template<typename Rep, class Period = std::ratio<1>>
class length : public units::unit<length<Rep, Period>>
{
    using units::unit<length<Rep, Period>>::unit;
};

using nanometer  = length<int64_t, std::nano>;
using micrometer = length<int64_t, std::micro>;
using millimeter = length<int64_t, std::milli>;
using centimeter = length<int64_t, std::centi>;
using meter      = length<int64_t>;
using kilometer  = length<int64_t, std::kilo>;

} // namespace units

#endif