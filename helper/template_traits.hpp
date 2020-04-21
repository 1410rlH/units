#ifndef TEMPLATE_TRAITS_HPP
#define TEMPLATE_TRAITS_HPP

#include <type_traits>

inline namespace template_traits {

template<template<typename...> class, template<typename...> class> 
struct is_same_template : std::false_type {};

template<template<typename...> class Tp> 
struct is_same_template<Tp, Tp> : std::true_type {};

template<template<typename...> class T, template<typename...> class U> 
constexpr bool is_same_template_v = is_same_template<T, U>::value;


template<typename, typename> 
struct has_same_template_class : std::false_type {};

template<template<typename...> class C, typename ...Args1, typename ...Args2> 
struct has_same_template_class<C<Args1...>, C<Args2...>> : std::true_type {};

template<class T, class U>
constexpr bool has_same_template_class_v = has_same_template_class<T, U>::value;


template<typename T, typename...> 
struct change_templates { using type = T; };

template<template<typename...> class C, typename ...Args, typename ...Ts>
struct change_templates<C<Args...>, Ts...> { using type = C<Ts...>; };

template<class T, typename ...Ts> 
using change_templates_t = typename change_templates<T, Ts...>::type;

} // inline namespace template_traits

#endif // TEMPLATE_TRAITS_HPP
