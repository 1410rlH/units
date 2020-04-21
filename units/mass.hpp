#ifndef UNIT_MASS_HPP
#define UNIT_MASS_HPP

#include "unit.hpp"

namespace units {

template<typename Rep, class Period = std::ratio<1>>
class mass : public units::unit<mass<Rep, Period>>
{
    using units::unit<mass<Rep, Period>>::unit;
};

using nanogram  = mass<int64_t, std::nano>;
using microgram = mass<int64_t, std::micro>;
using milligram = mass<int64_t, std::milli>;
using gram      = mass<int64_t>;
using kilogram  = mass<int64_t, std::kilo>;
using tonne     = mass<int64_t, std::mega>; 

} // namespace units

#endif