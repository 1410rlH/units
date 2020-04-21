#ifndef UNIT_HPP
#define UNIT_HPP

#include <limits>
#include <ratio>
#include <type_traits>

#include "../helper/template_traits.hpp"

namespace units {

template<typename> class unit;

inline namespace traits {

template<typename T>
constexpr bool is_unit_v = std::is_base_of_v<unit<T>, T>;


template<typename T>
constexpr bool is_ratio_v = false;

template<template<intmax_t, intmax_t> class T, intmax_t Num, intmax_t Den>
constexpr bool is_ratio_v<T<Num, Den>> 
    = std::is_base_of_v<std::ratio<Num, Den>, T<Num, Den>>;


template<
    typename R1, typename R2, 
    bool = std::is_convertible_v<R2, typename std::common_type_t<R1, R2>>
> struct common_rep_type;

template<typename R1, typename R2>
struct common_rep_type<R1, R2, true>
{ using type = typename std::common_type_t<R1, R2>; };

template<typename R1, typename R2>
using common_rep_type_t = typename common_rep_type<R1, R2>::type;

} // inline namespace traits

} // namespace units

/* GLOBAL. Spec of std::common_type for units */
/*----------------------------------------------------------------------------*/
template<
    template<typename, class> class UT,
    typename R1, typename P1,
    typename R2, typename P2
> struct std::common_type<UT<R1, P1>, UT<R2, P2>>
{
private:
    static_assert(units::is_unit_v<UT<R1, P1>>, 
        "this common_type spec is for units only");

    using cr = typename std::common_type_t<R1, R2>;
    using gcd_num = std::__static_gcd<P1::num, P2::num>;
    using gcd_den = std::__static_gcd<P1::den, P2::den>;
    using r = std::ratio<gcd_num::value, (P1::den / gcd_den::value) * P2::den>;
public:
    using type = UT<cr, r>;
};
/*----------------------------------------------------------------------------*/

namespace units {

template<
    class ToUnit, template<typename, class> class UT,
    typename R, class P, typename = typename std::enable_if_t<
        units::is_unit_v<ToUnit> && units::is_unit_v<UT<R, P>> &&
        template_traits::has_same_template_class_v<
            UT<R, P>, typename ToUnit::unit_type
        >
    >
> ToUnit unit_cast(const UT<R, P> &u)
{
    using to_rep = typename ToUnit::rep;
    using to_period = typename ToUnit::period;
    using cr = typename std::common_type_t<R, to_rep, intmax_t>;
    using cf = std::ratio_divide<P, to_period>;
    return ToUnit(
        static_cast<to_rep>(
            static_cast<cr>(u.count())
            / static_cast<cr>(cf::den)
            * static_cast<cr>(cf::num) 
        )
    );
}

template<
    class ToUnit, template<typename, class> class UT,
    typename R, class P, typename = typename std::enable_if_t<
        units::is_unit_v<ToUnit> && units::is_unit_v<UT<R, P>> &&
        template_traits::has_same_template_class_v<
            UT<R, P>, typename ToUnit::unit_type
        >
    >
> constexpr ToUnit floor(const UT<R, P> &u)
{
	auto to = units::unit_cast<ToUnit>(u);
	if (to > u)
	  return to - ToUnit{1};
	return to;
}

template<
    class ToUnit, template<typename, class> class UT,
    typename R, class P, typename = typename std::enable_if_t<
        units::is_unit_v<ToUnit> && units::is_unit_v<UT<R, P>> &&
        template_traits::has_same_template_class_v<
            UT<R, P>, typename ToUnit::unit_type
        >
    >
> constexpr ToUnit ceil(const UT<R, P> &u)
{
	auto to = units::unit_cast<ToUnit>(u);
	if (to < u)
	  return to + ToUnit{1};
	return to;
}

template<
    class ToUnit, template<typename, class> class UT,
    typename R, class P, typename = typename std::enable_if_t<
        units::is_unit_v<ToUnit> && units::is_unit_v<UT<R, P>> &&
        template_traits::has_same_template_class_v<
            UT<R, P>, typename ToUnit::unit_type
        > && !std::is_floating_point_v<R>
    >
> constexpr ToUnit round(const UT<R, P> &u)
{
	ToUnit u0 = units::floor<ToUnit>(u);
	ToUnit u1 = u0 + ToUnit{1};
	auto diff0 = u  - u0;
	auto diff1 = u1 - u;
	if (diff0 == diff1)
	{
	    if (u0.count() & 1)
		    return u1;
	    return u0;
	}
	else if (diff0 < diff1)
	    return u0;
	return u1;
}

template<
    template<typename, class> class UT,
    typename R, typename P, typename = typename std::enable_if_t<
        units::is_unit_v<UT<R, P>> &&
        std::numeric_limits<R>::is_signed
    >
> constexpr UT<R, P> abs(UT<R, P> u) 
{ 
    if (u >= u.zero())
        return u;
    else 
        return -u;
}

template<typename R>
struct unit_values
{
	static constexpr R zero() 
    { return R(0); }

	static constexpr R max()
	{ return std::numeric_limits<R>::max(); }

	static constexpr R min()
	{ return std::numeric_limits<R>::lowest(); }
};

template<template<typename, class> class UT, typename R, class P>
class unit<UT<R, P>>
{
public:
    using rep = R;
    using period = P;

    using unit_type = UT<rep, period>;

    static_assert(!units::is_unit_v<rep>, "rep cannot be a unit");
    static_assert(units::is_ratio_v<period>, "period must be a spec of ratiod");
    static_assert(period::num > 0, "period must be positive");
public:
    unit() = default;

    template<
        typename R2, typename = typename std::enable_if_t<
            std::is_convertible_v<R2, rep> &&
            (std::is_floating_point_v<rep> || 
            !std::is_floating_point_v<R2>)
        >
    > constexpr explicit unit(const R2 &r) 
        : m_r(static_cast<rep>(r)) {}

    template<typename R2, class P2>
    constexpr unit(const UT<R2, P2> &u)
        : m_r(unit_cast<unit_type>(u).count()) {}
public:
    constexpr rep count() const { return m_r; } 

    constexpr unit_type operator+() { return unit_type(m_r); }

    constexpr unit_type operator-() { return unit_type(-m_r); }

    constexpr unit_type operator++(int) { return unit_type(m_r++); }

    constexpr unit_type operator--(int) { return unit_type(m_r--); }

    constexpr unit_type & operator++() 
    { ++m_r; return static_cast<unit_type &>(*this); }

    constexpr unit_type & operator--() 
    { --m_r; return static_cast<unit_type &>(*this); }

    constexpr unit_type & operator+=(const unit_type& u)
	{ m_r += u.count();return static_cast<unit_type &>(*this); }

    constexpr unit_type & operator-=(const unit_type& u)
	{ m_r -= u.count(); return static_cast<unit_type &>(*this); }

    constexpr unit_type & operator*=(const rep& r)
	{ m_r *= r; return static_cast<unit_type &>(*this); }

	constexpr unit_type & operator/=(const rep& r)
	{ m_r /= r; return static_cast<unit_type &>(*this); }

	template<typename R2 = rep>
	constexpr typename std::enable_if_t<
        !std::is_floating_point_v<R2>,
		unit_type &
    > operator%=(const rep &r) 
    { m_r %= r; return static_cast<unit_type &>(*this); }

	template<typename R2 = rep>
	constexpr typename std::enable_if_t<
        !std::is_floating_point_v<R2>,
		unit_type &
    > operator%=(const unit_type &u) 
    { m_r %= u.count(); return static_cast<unit_type &>(*this); }
public:
	static constexpr unit_type zero()
	{ return unit_type(unit_values<rep>::zero()); }

	static constexpr unit min()
	{ return unit_type(unit_values<rep>::min()); }

	static constexpr unit max()
	{ return unit_type(unit_values<rep>::max()); }
private:
    rep m_r;
};

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr typename std::common_type_t<U1, U2>
operator+(const U1 &lhs, const U2 &rhs)
{
    using cu = typename std::common_type_t<U1, U2>;
    return cu(cu(lhs).count() + cu(rhs).count());
}

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr typename std::common_type_t<U1, U2>
operator-(const U1 &lhs, const U2 &rhs)
{
    using cu = typename std::common_type_t<U1, U2>;
    return cu(cu(lhs).count() - cu(rhs).count());
}

template<
    template<typename, class> class UT,
    typename R, class P, typename R2, 
    typename = typename std::enable_if_t<units::is_unit_v<UT<R, P>>>
> constexpr UT<typename units::common_rep_type_t<R, R>, P>
operator*(const UT<R, P> &u, const R2 &r)
{
    using cu = UT<typename std::common_type_t<R, R2>, P>;
    return cu(cu(u).count() * r);
}

template<
    template<typename, class> class UT,
    typename R, class P, typename R2, 
    typename = typename std::enable_if_t<units::is_unit_v<UT<R, P>>>
> constexpr unit<UT<typename units::common_rep_type_t<R, R>, P>>
operator*(const R2 &r, const UT<R, P> &u)
{ return u * r; }

template<
    template<typename, class> class UT,
    typename R, class P, typename R2, typename = typename std::enable_if_t<
        units::is_unit_v<UT<R, P>> && !units::is_unit_v<R2>
    >
> constexpr UT<typename units::common_rep_type_t<R, R2>, P>
operator/(const UT<R, P> &u, const R2 &r)
{
    using cu = UT<typename std::common_type_t<R, R2>, P>;
    return cu(cu(u).count() / r);
}

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr typename std::common_type_t<typename U1::rep, typename U2::rep>
operator/(const U1 &lhs, const U2 &rhs)
{
    using cu = std::common_type_t<U1, U2>;
    return cu(lhs).count() / cu(rhs).count();
}

template<
    template<typename, class> class UT,
    typename R, class P, typename R2, typename = typename std::enable_if_t<
        units::is_unit_v<UT<R, P>> && !units::is_unit_v<R2>
    >
> constexpr UT<typename units::common_rep_type_t<R, R2>, P> 
operator%(const UT<R, P> &u, const R2 &r)
{
    using cu = UT<typename std::common_type_t<R, R2>, P>;
    return cu(cu(u).count() % r);
}

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr typename std::common_type_t<U1, U2>
operator%(const U1 &lhs, const U2 &rhs)
{
    using cu = typename std::common_type_t<U1, U2>;
    return cu(cu(lhs).count() % cu(rhs).count());
}

template<
    template<typename, class> class UT,
    typename R1, class P1,
    typename R2, class P2,
    typename = typename std::enable_if_t<
        units::is_unit_v<UT<R1, P1>>
    >
> constexpr bool operator==(const UT<R1, P1> &lhs, const UT<R2, P2> &rhs)
{
    using cu = typename std::common_type_t<UT<R1, P2>, UT<R2, P2>>;
    return cu(lhs).count() == cu(rhs).count();
}

template<
    template<typename, class> class UT,
    typename R1, class P1,
    typename R2, class P2,
    typename = typename std::enable_if_t<
        units::is_unit_v<UT<R1, P1>>
    >
> constexpr bool operator<(const UT<R1, P1> &lhs, const UT<R2, P2> &rhs)
{
    using cu = typename std::common_type_t<UT<R1, P2>, UT<R2, P2>>;
    return cu(lhs).count() < cu(rhs).count();
}

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr bool operator!=(const U1 &lhs, const U2 &rhs)
{ return !(lhs == rhs); }

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr bool operator<=(const U1 &lhs, const U2 &rhs)
{ return !(rhs < lhs); }

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr bool operator>(const U1 &lhs, const U2 &rhs)
{ return rhs < lhs; }

template<
    class U1, class U2, 
    typename = typename std::enable_if_t<
        units::is_unit_v<U1> && units::is_unit_v<U2> &&
        template_traits::has_same_template_class_v<
            typename U1::unit_type,
            typename U2::unit_type
        >
    >
> constexpr bool operator>=(const U1 &lhs, const U2 &rhs)
{ return !(lhs < rhs); }

} // namespace units
#endif // UNIT_HPP
