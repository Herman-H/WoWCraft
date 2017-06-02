#ifndef TMP_MATH_H
#define TMP_MATH_H

#include <cstddef>
#include "tmp.h"

namespace tmp
{
    namespace math
    {

        namespace detail
        {
            template <typename T>
            struct numbers_to_type_impl;
            template <size_t ... VS>
            struct numbers_to_type_impl<tuple_i<VS...>>
            {
                typedef tuple_t<int_type<VS...>> type;
            };
            template <typename T>
            struct extract_numbers_impl;
            template <size_t ... VS>
            struct extract_numbers_impl<int_type<VS>...>
            {
                typedef tuple_i<VS...> type;
            };
            template <typename T>
            using extract_numbers = typename extract_numbers_impl<T>::type;
            template <typename T>
            using numbers_to_type = typename numbers_to_type_impl<T>::type;

            template <typename,typename>
            struct operation
            {
                struct add;
                struct subtract;
                struct multiply;
                struct divide;
                struct modulus;
            };

            template <size_t A,size_t B>
            struct operation<int_type<A>,int_type<B>>
            {
                using add = int_type<A+B>;
                using subtract = int_type<A-B>;
                using multiply = int_type<A*B>;
                using divide = int_type<A/B>;
                using modulus = int_type<A%B>;
            };

            template <size_t BASE, size_t POWER>
            struct power_impl
            {
                static constexpr auto value = power_impl<BASE,POWER-1>::value * BASE;
            };
            template <size_t BASE>
            struct power_impl<BASE,0>
            {
                static constexpr auto value = 1;
            };

            template <size_t N, typename T>
            struct sum_first_impl;
            template <size_t N, template <size_t...> typename T>
            struct sum_first_impl<N, T<>>
            {
                enum { value = 0 };
            };
            template <template <size_t...> typename T, size_t V, size_t ... VS>
            struct sum_first_impl<0,T<V,VS...>>
            {
                enum { value = 0 };
            };
            template <size_t N, template <size_t...> typename T, size_t V, size_t ... VS>
            struct sum_first_impl<N, T<V,VS...>>
            {
                enum { value = V + sum_first_impl<N-1,tuple_i<VS...>>::value };
            };
        } // detail

        template <size_t BASE, size_t POWER>
        constexpr auto power = detail::power_impl<BASE,POWER>::value;

        template <size_t ... VS>
        constexpr auto sum = detail::sum_first_impl<sizeof...(VS),tuple_i<VS...>>::value;

        template <typename T>
        constexpr auto sum_tuple = detail::sum_first_impl<tmp::cardinality<T>,T>::value;

        template <size_t N, typename T>
        constexpr auto sum_first = detail::sum_first_impl<N,T>::value;


        namespace detail
        {
            template <size_t N, size_t D, typename ... DIVISORS>
            struct greatest_divisor_impl;
            template <size_t N, size_t D, typename T, typename ... TS>
            struct greatest_divisor_impl<N,D,T,TS...>
            {
                enum { value = ((D < T::value) && (N % T::value == 0)) ? greatest_divisor_impl<N,T::value,TS...>::value : greatest_divisor_impl<N,D,TS...>::value };
            };
            template <size_t N, size_t D>
            struct greatest_divisor_impl<N,D>
            {
                enum { value = D };
            };

            template <size_t N, typename DIVISOR_SET>
            struct greatest_divisor;

            template <size_t N, template <typename ... INTTYPES> typename SET, typename ... INTTYPES>
            struct greatest_divisor<N,SET<INTTYPES...>>
            {
                enum { value = detail::greatest_divisor_impl<N,1,INTTYPES...>::value };
            };
        } // namespace detail

        template <size_t N, typename DIVISOR_SET>
        constexpr auto greatest_divisor = detail::greatest_divisor<N,DIVISOR_SET>::value;


    } // math

} // namespace tmp























#endif // TMP_MATH_H
