#ifndef TMP_H
#define TMP_H

#include <cstddef>
#include <type_traits>

namespace tmp
{

struct no_type{};

template <typename T>
using identity = T;

template <size_t N>
struct int_type
{
    enum { value = N };
};

template <size_t N1, size_t N2>
struct int_pair_type
{
    enum { v1 = N1 };
    enum { v2 = N2 };
};

template <typename,typename>
struct pair_t;

template <size_t,size_t>
struct pair_i;

template <typename ...>
struct tuple_t;

template <size_t ...>
struct tuple_i;

namespace detail
{
    template <typename T>
    struct cardinality_impl;
    template <template <typename...> typename T, typename ... TS>
    struct cardinality_impl<T<TS...>>
    {
        enum { value = sizeof...(TS) };
    };
    template <template <size_t...> typename T, size_t ... VS>
    struct cardinality_impl<T<VS...>>
    {
        enum { value = sizeof...(VS) };
    };

    template <typename,typename>
    struct types_equal_impl
    {
        enum { value = false };
    };

    template <typename T>
    struct types_equal_impl<T,T>
    {
        enum { value = true };
    };

    template <typename E>
    struct expression_is_valid_impl
    {
        constexpr bool eval(E){ return true; }
        template <typename S>
        constexpr bool eval(S){ return false; }
    };

} // namespace detail

/* The use of template <typename t> constexpr auto = enum_value,
 * Referring to [dcl.enum] section 7 in the C++ standard:
 *
 * Quote:
 *      "For an enumeration whose underlying type is not fixed, the underlying type is an integral type that can
 *      represent all the enumerator values defined in the enumeration. If no integral type can represent all the
 *      enumerator values, the enumeration is ill-formed. It is implementation-defined which integral type is used
 *      as the underlying type except that the underlying type shall not be larger than int unless the value of an
 *      enumerator cannot fit in an int or unsigned int. If the enumerator-list is empty, the underlying type is as
 *      if the enumeration had a single enumerator with value 0."
 *
 * Conclusion: The type deduced in "auto" will probably be "int", unless the value is too large to be represented by "int".
 */

template <typename T>
constexpr auto cardinality = detail::cardinality_impl<T>::value;

template <typename T1, typename T2>
constexpr bool types_equal = detail::types_equal_impl<T1,T2>::value;

namespace detail
{

    template <size_t N, typename ... TS>
    struct get_element_at_impl
    {
        static_assert(sizeof...(TS) >= N, "Element at index does not exist. Index too large.\n");
    };

    template <typename T, typename ... TS>
    struct get_element_at_impl<0, T, TS...>
    {
        typedef T type;
    };

    template <size_t N, typename T, typename ... TS>
    struct get_element_at_impl<N, T, TS...>
    {
        typedef typename get_element_at_impl<N-1,TS...>::type type;
    };

} // namespace detail

template <size_t N, typename ... TS>
using get_element_at = typename detail::get_element_at_impl<N,TS...>::type;
template <size_t N, size_t ... VS>
using get_element_at_i = typename detail::get_element_at_impl<N,int_type<VS>...>::type;

namespace detail
{
    template <size_t N, typename T>
    struct get_element_in_impl;
    template <size_t N, template <typename...> typename T, typename ... TS>
    struct get_element_in_impl<N, T<TS...>>
    {
        typedef get_element_at<N,TS...> type;
    };
    template <size_t N, template <size_t ...> typename T, size_t ... VS>
    struct get_element_in_impl<N, T<VS...>>
    {
        typedef get_element_at_i<N,VS...> type;
    };
} // namespace detail

template <size_t N, typename T>
using get_element_in = typename detail::get_element_in_impl<N,T>::type;

namespace detail
{
    // the interface of this has arguments in an order which makes sense
    template <template <typename> typename PREDICATE>
    struct get_element_of_impl
    {
        template <typename T>
        struct that_satisfies
        {
            typedef no_type type;
        };
        template <template <typename...> typename T, typename E, typename ... ES>
        struct that_satisfies<T<E,ES...>>
        {
            typedef get_element_at<PREDICATE<E>::value != 0,
                                            typename that_satisfies<tuple_t<ES...>>::type,
                                            E>
                        type;
        };

    };

    template <size_t I, typename T, typename ... XS>
    struct index_of_impl;
    template <size_t I, typename T, typename X, typename ... XS>
    struct index_of_impl<I, T, X, XS...>
    {
        enum { value = types_equal<T,X> ? I : index_of_impl<I+1,T,XS...>::value };
        enum { found = types_equal<T,X> ? true : index_of_impl<I+1,T,XS...>::found };
    };
    template <size_t I, typename T>
    struct index_of_impl<I,T>
    {
        enum { value = 0 };
        enum { found = false };
    };

    template <typename T, typename TUP>
    struct index_in_impl;
    template <typename T, template <typename...> typename TUP, typename ... XS>
    struct index_in_impl<T,TUP<XS...>>
    {
        enum { value = index_of_impl<0,T,XS...>::value };
        enum { found = index_of_impl<0,T,XS...>::found };
    };

} // detail

template <typename T>
struct get_element_of
{
    template <template <typename> typename PREDICATE>
    using that_satisfies = typename detail::get_element_of_impl<PREDICATE>::template that_satisfies<T>::type;
};

template <typename T, typename ... XS>
constexpr auto index_of = detail::index_of_impl<0,T,XS...>::value;
template <typename T, typename ... XS>
constexpr bool found_among = detail::index_of_impl<0,T,XS...>::found;
template <typename T, typename XS>
constexpr auto index_in = detail::index_in_impl<T,XS>::value;
template <typename T, typename XS>
constexpr bool found_in = detail::index_in_impl<T,XS>::found;

template <template <typename,typename> typename OPERATOR, template <typename> typename PREDICATE, typename INITVAL>
struct fold
{
    template <typename TUPLE>
    struct on
    {
        enum { value = INITVAL::value };
    };

    template <template <typename...> typename TUPLE, typename E, typename ... ES>
    struct on<TUPLE<E,ES...>>
    {
        enum { value = OPERATOR<PREDICATE<E>,on<tuple_t<ES...>>>::value };
    };
};

namespace detail
{
    template <typename A, typename B>
    struct addition
    {
        enum { value = A::value + B::value };
    };
    template <typename A, typename B>
    struct multiplication
    {
        enum { value = A::value * B::value };
    };
}


template <template <typename> typename PREDICATE>
struct sum
{
    template <typename TUPLE>
    using of = typename fold<detail::addition,PREDICATE,int_type<0>>::template on<TUPLE>;
};

template <template <typename> typename PREDICATE>
struct product
{
    template <typename TUPLE>
    using of = typename fold<detail::multiplication,PREDICATE,int_type<1>>::template on<TUPLE>;
};


namespace detail
{

    template <typename T,typename S>
    struct set_of_impl;

    template <typename ... S>
    struct set_of_impl<tuple_t<>,tuple_t<S...>>
    {
        typedef tuple_t<S...> type;
    };

    template <typename E>
    struct type_equals
    {
        template <typename T>
        struct with
        {
            enum { value = types_equal<E,T> };
        };
    };

    template <typename E, typename ... ES, typename ... S>
    struct set_of_impl<tuple_t<E,ES...>, tuple_t<S...>>
    {
        typedef get_element_at< sum<type_equals<E>::template with>::template of<tuple_t<S...>>::value != 0,
                                 typename set_of_impl<tuple_t<ES...>,tuple_t<S...,E>>::type,
                                 typename set_of_impl<tuple_t<ES...>,tuple_t<S...>>::type
                                > type;

    };

} // namespace detail

/*  Compare all elements against each other to determine if duplicates exists.
    This requires (N^2/)2 comparisons. */

template <typename T>
using set_of = typename detail::set_of_impl<T,tuple_t<>>::type;

template <typename>
struct append
{
    template <typename>
    struct with;
};

template <template <typename...> typename T, typename ... ES>
struct append<T<ES...>>
{
    template <typename B>
    using with = tuple_t<ES...,B>;
};

template <typename>
struct prepend
{
    template <typename>
    struct with;
};

template <template <typename...> typename T, typename ... ES>
struct prepend<T<ES...>>
{
    template <typename A>
    using with = tuple_t<A,ES...>;
};

namespace detail
{
    template <size_t I, typename S, typename T, typename E>
    struct insert_element_at_impl
    {
        typedef S type;
    };
    template <size_t I, template <typename...> typename TUP, typename ... S, typename T, typename ... TS, typename E>
    struct insert_element_at_impl<I, TUP<S...>, TUP<T,TS...>, E>
    {
        typedef typename insert_element_at_impl<I-1, TUP<S...,T>, TUP<TS...>, E>::type type;
    };
    template <template <typename...> typename TUP, typename ... S, typename T, typename ... TS, typename E>
    struct insert_element_at_impl<0, TUP<S...>, TUP<T,TS...>, E>
    {
        typedef TUP<S...,E,T,TS...> type;
    };
    template <template <typename...> typename TUP, typename ... S, typename ... TS, typename E>
    struct insert_element_at_impl<0, TUP<S...>, TUP<TS...>, E>
    {
        typedef TUP<S...,E,TS...> type;
    };
    template <size_t I, typename S, typename T, size_t E>
    struct insert_element_at_i_impl
    {
        typedef S type;
    };
    template <size_t I, template <size_t...> typename TUP, size_t ... S, size_t T, size_t ... TS, size_t E>
    struct insert_element_at_i_impl<I, TUP<S...>, TUP<T,TS...>, E>
    {
        typedef typename insert_element_at_i_impl<I-1, TUP<S...,T>, TUP<TS...>, E>::type type;
    };
    template <template <size_t...> typename TUP, size_t ... S, size_t T, size_t ... TS, size_t E>
    struct insert_element_at_i_impl<0, TUP<S...>, TUP<T,TS...>, E>
    {
        typedef TUP<S...,E,T,TS...> type;
    };
    template <template <size_t...> typename TUP, size_t ... S, size_t ... TS, size_t E>
    struct insert_element_at_i_impl<0, TUP<S...>, TUP<TS...>, E>
    {
        typedef TUP<S...,E,TS...> type;
    };

} // namespace detail

template <typename E,typename T>
struct insert_element_in
{
    template <size_t N>
    struct at;
};
template <typename E,template <typename...> typename T, typename ... TS>
struct insert_element_in<E,T<TS...>>
{
    template <size_t N>
    using at = typename detail::insert_element_at_impl<N,T<>,T<TS...>,E>::type;
};

template <size_t V, typename T>
struct insert_value_in
{
    template <size_t N>
    struct at;
};
template <size_t V,template <size_t...> typename T, size_t ... VS>
struct insert_value_in<V,T<VS...>>
{
    template <size_t N>
    using at = typename detail::insert_element_at_i_impl<N,T<>,T<VS...>,V>::type;
};
namespace detail
{
    template <typename MERGED, typename ... TS>
    struct merge_tuples_impl;
    template <typename ... MS>
    struct merge_tuples_impl<tuple_t<MS...>>
    {
        typedef tuple_t<MS...> type;
    };
    template <typename ... MS, typename ... NWS, typename ... TS>
    struct merge_tuples_impl<tuple_t<MS...>,tuple_t<NWS...>,TS...>
    {
        typedef typename merge_tuples_impl<tuple_t<MS...,NWS...>,TS...>::type type;
    };
    template <typename ... MS, typename T, typename ... TS>
    struct merge_tuples_impl<tuple_t<MS...>,T,TS...>
    {
        typedef typename merge_tuples_impl<tuple_t<MS...,T>,TS...>::type type;
    };

    template <typename MERGED, typename ... TS>
    struct merge_tuples_i_impl;
    template <size_t ... VS>
    struct merge_tuples_i_impl<tuple_i<VS...>>
    {
        typedef tuple_i<VS...> type;
    };
    template <size_t ... VS, size_t ... NVS, typename ... TS>
    struct merge_tuples_i_impl<tuple_i<VS...>, tuple_i<NVS...>, TS...>
    {
        typedef typename merge_tuples_i_impl<tuple_i<VS...,NVS...>,TS...>::type type;
    };

} // namespace detail

template <typename ... TS>
using merge_tuples = typename detail::merge_tuples_impl<tuple_t<>,TS...>::type;
template <typename ... VTS>
using merge_tuples_i = typename detail::merge_tuples_i_impl<tuple_i<>,VTS...>::type;

template <class META, typename ... CS>
struct tree_node : public META
{
};

namespace detail
{
    // tree_impl<CS>::type is a tuple of tuples
    template <typename NODE>
    struct tree_impl
    {
        typedef tuple_t<tuple_t<NODE>> type;
    };
    template <typename C, typename T>
    struct prepend_all_of;
    template <typename C, typename ... ES>
    struct prepend_all_of<C,tuple_t<ES...>>
    {
        typedef tuple_t<typename prepend<ES>::template with<C>...> type;
    };
    template <typename T>
    struct tree_extract_impl;
    template <typename T>
    struct tree_extract_impl<tuple_t<T>>
    {
        typedef T type;
    };
    template <typename META, typename ... CS>
    struct tree_impl<tree_node<META,CS...>>
    {
        // The child of tree_impl will provide a list of all its nodes
        typedef merge_tuples<typename prepend_all_of<META, typename tree_impl<CS>::type>::type...> type;
    };
} // namespace detail

/*  If the given input is 'tree-like', then provide information of every end-node as a tuple_t
    of the path down to it. */

template <typename T>
using tree = typename detail::tree_impl<T>::type;

namespace detail
{
    template <template <typename> typename F, typename T, typename N>
    struct map_impl;
    template <template <typename> typename F, template <typename...> typename T, typename ... NS>
    struct map_impl<F,T<>,tuple_t<NS...>>
    {
        typedef tuple_t<NS...> type;
    };
    template <template <typename> typename F, template <typename...> typename T, typename E, typename ... ES, typename ... NS>
    struct map_impl<F,T<E,ES...>,tuple_t<NS...>>
    {
        typedef typename map_impl<F,T<ES...>,tuple_t<NS...,typename F<E>::type>>::type type;
    };
} // namespace detail

template <template <typename> typename F, typename T>
using map = typename detail::map_impl<F,T,tuple_t<>>::type;

namespace detail
{
    template <typename T, bool IS_CLASS>
    struct data_wrapper : public T
    { };
    template <typename T>
    struct data_wrapper<T, false>
    {
        T data;
    };
    template <size_t I, typename T>
    struct leaf_node : public data_wrapper<T,std::is_class<T>::value>
    {
    };

    template <size_t I, typename T, typename IS_CLASS>
    T& get_leaf_data(leaf_node<I,T> & l, IS_CLASS c)
    {
        return static_cast<T&>(l);
    }

    template <size_t I, typename T>
    T& get_leaf_data(leaf_node<I,T> & l, int_type<0>)
    {
        return static_cast<data_wrapper<T,false>&>(l).data;
    }

    template <size_t ... SEQ>
    struct sequence_impl;

    template <size_t N, size_t ... NS>
    struct sequence_impl<N, NS...> : public sequence_impl<N-1,N-1,NS...>
    {
    };

    template <>
    struct sequence_impl<0>
    {
        typedef tuple_i<0> type;
    };

    template <size_t ... SEQ>
    struct sequence_impl<1,SEQ...>
    {
        typedef tuple_i<0,SEQ...> type;
    };

} // namespace detail

template <size_t ... SEQ>
using sequence = typename detail::sequence_impl<SEQ...>::type;

namespace detail
{
    template <size_t V, typename T>
    struct set_values_to_impl
    {
        typedef tuple_i<> type;
    };

    template <size_t V, size_t ... NS>
    struct set_values_to_impl<V, tuple_i<NS...>>
    {
        typedef tuple_i< (true ? V : NS)... > type;
    };

    template <size_t FROMSIZE, size_t SPLITS>
    struct split_tuple_sizes_impl
    {
        enum { TOSIZE = FROMSIZE / SPLITS };
        enum { REST = FROMSIZE % SPLITS };

        typedef get_element_at< (TOSIZE > 0),
                                typename set_values_to_impl<1,sequence<FROMSIZE>>::type,
                                get_element_at< REST == 0,
                                                merge_tuples_i< typename set_values_to_impl<TOSIZE+1,sequence<REST>>::type,
                                                                typename set_values_to_impl<TOSIZE, sequence<SPLITS-REST>>::type>,
                                                typename set_values_to_impl<TOSIZE,sequence<SPLITS>>::type
                                              >
                             > type;

    };

    template <size_t SELECT, typename S, typename ... TS>
    struct select_first_impl
    {
        typedef S type;
    };

    template <size_t SELECT, typename ... S, typename T, typename ... TS>
    struct select_first_impl<SELECT, tuple_t<S...>, T, TS...>
    {
        typedef typename select_first_impl<SELECT-1, tuple_t<S..., T>, TS...>::type type;
    };
    template <typename ... S, typename T, typename ... TS>
    struct select_first_impl<0, tuple_t<S...>, T, TS...>
    {
        typedef tuple_t<S...> type;
    };
    template <typename ... S>
    struct select_first_impl<0, tuple_t<S...>>
    {
        typedef tuple_t<S...> type;
    };
    template <size_t SELECT, typename S, size_t ... VS>
    struct select_first_i_impl
    {
        typedef S type;
    };
    template <size_t SELECT, size_t ... S, size_t V, size_t ... VS>
    struct select_first_i_impl<SELECT, tuple_i<S...>, V, VS...>
    {
        typedef typename select_first_i_impl<SELECT-1, tuple_i<S..., V>, VS...>::type type;
    };
    template <size_t ... S, size_t V, size_t ... VS>
    struct select_first_i_impl<0, tuple_i<S...>, V, VS...>
    {
        typedef tuple_i<S...> type;
    };
    template <size_t ... S>
    struct select_first_i_impl<0, tuple_i<S...>>
    {
        typedef tuple_i<S...> type;
    };

    template <size_t SELECT, typename ... TS>
    struct select_last_impl
    {
        typedef tuple_t<> type;
    };
    template <size_t SELECT, size_t ... VS>
    struct select_last_i_impl
    {
        typedef tuple_i<> type;
    };
    template <size_t SELECT, typename T, typename ... TS>
    struct select_last_impl<SELECT, T, TS...>
    {
        typedef typename select_last_impl<SELECT-1, TS...>::type type;
    };
    template <size_t SELECT, size_t V, size_t ... VS>
    struct select_last_i_impl<SELECT, V, VS...>
    {
        typedef typename select_last_i_impl<SELECT-1, VS...>::type type;
    };
    template <typename T, typename ... TS>
    struct select_last_impl<0, T, TS...>
    {
        typedef tuple_t<T,TS...> type;
    };
    template <size_t V, size_t ... VS>
    struct select_last_i_impl<0,V,VS...>
    {
        typedef tuple_i<V,VS...> type;
    };
    template <typename ... TS>
    struct select_last_impl<0, TS...>
    {
        typedef tuple_t<TS...> type;
    };
    template <size_t ... VS>
    struct select_last_i_impl<0, VS...>
    {
        typedef tuple_i<VS...> type;
    };

    template <typename SPLITRULE, typename R, typename T>
    struct split_tuple_impl
    {
        typedef R type;
    };

    template <size_t V, size_t ... VS, typename ... RS, typename ... TS>
    struct split_tuple_impl<tuple_i<V,VS...>, tuple_t<RS...>, tuple_t<TS...>>
    {
        typedef typename split_tuple_impl<  tuple_i<VS...>,
                                            tuple_t<RS..., typename select_first_impl<V,tuple_t<>,TS...>::type>,
                                            typename select_last_impl<V,TS...>::type
                                >::type type;
    };
    template <size_t V, size_t ... VS, typename ... RS, size_t ... NS>
    struct split_tuple_impl<tuple_i<V,VS...>, tuple_t<RS...>, tuple_i<NS...>>
    {
        typedef typename split_tuple_impl<  tuple_i<VS...>,
                                            tuple_t<RS..., typename select_first_i_impl<V,tuple_i<>,NS...>::type>,
                                            typename select_last_i_impl<V,NS...>::type
                                >::type type;
    };

    template <typename R, typename T>
    struct split_tuple_impl<tuple_i<>, R, T>
    {
        typedef R type;
    };

} // namespace detail

template <typename T, size_t N>
using split_tuple = typename detail::split_tuple_impl<typename detail::split_tuple_sizes_impl<cardinality<T>, N>::type,tuple_t<>,T>::type;

namespace detail
{
    template <template <typename> typename F, typename BY, typename T, typename S>
    struct filter_impl;
    template <template <typename> typename F, typename BY, template <typename...> typename TUP, typename T, typename ... TS, typename ... S>
    struct filter_impl<F,BY,TUP<T,TS...>,tuple_t<S...>>
    {
        typedef typename filter_impl<   F,
                                        BY,
                                        TUP<TS...>,
                                        get_element_at< types_equal<typename F<T>::type,BY>,
                                                        tuple_t<S...>,
                                                        tuple_t<S...,T>
                                                      >
                                      >::type type;
    };

    template <template <typename> typename F, typename BY, template <typename...> typename TUP, typename S>
    struct filter_impl<F,BY,TUP<>,S>
    {
        typedef S type;
    };

} // namespace detail

template <template <typename> typename F, typename BY, typename T>
using filter = typename detail::filter_impl<F,BY,T,tuple_t<>>::type;

template <typename T>
struct fail
{
    typedef typename T::alalallalalalalla f;
};

} // namespace tmp


















#endif // TMP_H
