#ifndef DBTMP_H
#define DBTMP_H

#include <cstdlib>
#include <utility>

// We need a class that can store arbitrary fields of a query. A field is associated with a unique type

namespace dbtmp
{

struct no_type{};

template <typename T1, typename T2>
struct types_equal
{
    enum { value = false };
};

template <typename T>
struct types_equal<T,T>
{
    enum { value = true };
};

template <size_t N>
struct int_type{ enum{value = N}; };

template <typename ... TS>
struct tuple_t;

template <size_t ... VS>
struct tuple_v;

template <typename TS>
struct size_of_tuple;
template <template <typename...> class T, typename ... TS>
struct size_of_tuple<T<TS...>>
{
    enum { value = sizeof...(TS) };
};

template <template <size_t...> class T, size_t ... VS>
struct size_of_tuple<T<VS...>>
{
    enum { value = sizeof...(VS) };
};

namespace detail
{
template <size_t I, typename ... TS>
struct get_type_at_impl;
template <size_t I>
struct get_type_at_impl<I>
{
    typedef no_type type;
};
template <typename T, typename ... TS>
struct get_type_at_impl<0, T, TS...>
{
    typedef T type;
};

template <size_t I, typename T, typename ... TS>
struct get_type_at_impl<I, T, TS...>
{
    typedef typename get_type_at_impl<I-1,TS...>::type type;
};

template <size_t I, typename TPL>
struct get_type_at_tuple_impl;

template <size_t I, template <typename ...> class TPL, typename ... TS>
struct get_type_at_tuple_impl<I,TPL<TS...>>
{
    typedef typename get_type_at_impl<I,TS...>::type type;
};

} // namespace detail

template <size_t I, typename ... TS> using get_type_at = typename detail::get_type_at_impl<I,TS...>::type;
template <size_t I, typename T> using get_type_at_tuple = typename detail::get_type_at_tuple_impl<I,T>::type;

namespace detail
{
    template <size_t V, size_t ... VS>
    struct seq : public seq<V-1, V-1, VS...>
    {};
    template <size_t ... VS>
    struct seq<1,VS...>
    {
        typedef tuple_v<0,VS...> type;
    };
    template <>
    struct seq<0>
    {
        typedef tuple_v<0> type;
    };
} // namespace detail

template <size_t N> using sequence = typename detail::seq<N>::type;

namespace detail
{
    template <typename T, bool IS_CLASS>
    struct data_wrapper : public T
    {
        data_wrapper() :
            T()
        {}
        ~data_wrapper(){}
        data_wrapper(const T& t) :
            T(t)
        {
        }
    };
    template <typename T>
    struct data_wrapper<T,false>
    {
        data_wrapper(){}
        ~data_wrapper(){}
        data_wrapper(const T& t)
        {
            data = t;
        }

        T data;
    };
    template <size_t N, typename T>
    struct leaf_node : protected data_wrapper<T,std::is_class<T>::value>
    {
        leaf_node(){}
        ~leaf_node(){}
        leaf_node(const T& t) :
            data_wrapper<T,std::is_class<T>::value>(t)
        {

        }

        data_wrapper<T,std::is_class<T>::value>& data(){ return static_cast<data_wrapper<T,std::is_class<T>::value>&>(*this); }
    };

    template <size_t N, typename T, typename B>
    T& get_data(leaf_node<N,T>& ln, B)
    {
        return static_cast<T&>(ln.data());
    }

    template <size_t N, typename T>
    T& get_data(leaf_node<N,T>& ln, int_type<false>)
    {
        return ln.data().data;
    }

    template <typename SEQ, typename ... TS>
    struct tuple_impl;
    template <size_t ... VS, typename ... TS>
    struct tuple_impl<tuple_v<VS...>,TS...> : public leaf_node<VS,TS>...
    {

    };
}

template <size_t I, typename R, typename T>
R& get(const T & t);

template <typename ... TS>
struct tuple : public detail::tuple_impl<sequence<sizeof...(TS)>,TS...>
{
private:
    template <size_t I, typename ... PS>
    struct for_each_t{static void do_init(tuple<TS...>&){}};
    template <size_t I, typename P, typename ... PS>
    struct for_each_t<I,P,PS...>
    {
        static void do_init(tuple<TS...>& t, P p, PS ... ps)
        {
            get<I>(t) = p;
            for_each_t<I+1,PS...>::do_init(t,ps...);
        }
    };

    template <typename SEQ>
    struct cmp;
    template <size_t V>
    struct cmp<tuple_v<V>>
    {
        static int compare(const tuple& this_, const tuple& other)
        {
            return (get<V>(this_) == get<V>(other) ?
                        0 :
                        (get<V>(this_) < get<V>(other)? -1 : 1));
        }
    };
    template <size_t V, size_t ... VS>
    struct cmp<tuple_v<V,VS...>>
    {
        static int compare(const tuple& this_, const tuple& other)
        {
            return (get<V>(this_) == get<V>(other) ?
                        cmp<tuple_v<VS...>>::compare(this_,other) :
                        (get<V>(this_) < get<V>(other) ? -1 : 1));
        }
    };


public:
    tuple(){}
    ~tuple(){}
    tuple(TS ... ts)
    {
        for_each_t<0,TS...>::do_init(*this,ts...);
    }

    bool operator==(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) == 0;
    }
    bool operator !=(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) != 0;
    }
    bool operator<(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) == -1;
    }
    bool operator>(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) == 1;
    }
    bool operator <=(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) != 1;
    }
    bool operator >=(const tuple& other) const
    {
        return cmp<sequence<sizeof...(TS)>>::compare(*this,other) != -1;
    }
};

template <size_t I, typename ... TS>
get_type_at<I,TS...>& get(const tuple<TS...> & t)
{
    typedef get_type_at<I,TS...> T;
    return detail::get_data(static_cast<detail::leaf_node<I,T>&>(const_cast<tuple<TS...>&>(t)), int_type<std::is_class<T>::value>());
}

struct sized_cstring
{
    const int size;
    const char * data;
    template <size_t N>
    constexpr sized_cstring(const char (&c)[N]) : size(N), data(c)
    { }

    constexpr sized_cstring(const sized_cstring& s) : size(s.size), data(s.data)
    { }
    constexpr int get_size() const { return size; }
    constexpr const char * get_data() const { return data; }
};

namespace detail
{
    template <size_t N, typename S, typename R>
    struct find_and_remove;
    template <size_t N, size_t V, size_t ... VS, size_t ... RS>
    struct find_and_remove<N, tuple_v<V, VS...>, tuple_v<RS...>>
    {
        typedef get_type_at<   N == V,
                               typename find_and_remove<N, tuple_v<VS...>, tuple_v<RS...,V>>::type,
                               tuple_v<RS...,VS...> >
            type;
    };
    template <size_t N, size_t ... RS>
    struct find_and_remove<N, tuple_v<>, tuple_v<RS...>>
    {
        typedef tuple_v<RS...> type;
    };

    template <typename S1, typename S2>
    struct find_and_remove_all;
    template <size_t ... WS>
    struct find_and_remove_all<tuple_v<>, tuple_v<WS...>>
    {
        typedef tuple_v<WS...> type;
    };
    template <size_t V, size_t ... VS, size_t ... WS>
    struct find_and_remove_all<tuple_v<V,VS...>,tuple_v<WS...>>
    {
        typedef typename find_and_remove_all<
                            tuple_v<VS...>,
                            typename find_and_remove<V,tuple_v<WS...>,tuple_v<>>::type
                         >::type
                     type;

    };

} // namespace detail

template <typename S1, typename S2>
struct set_equals
{
    enum { value = 0 };
};

template <size_t V, size_t ... VS, size_t ... WS>
struct set_equals<tuple_v<V, VS...>,tuple_v<WS...>>
{
    enum { value = (sizeof...(VS) + 1  == sizeof...(WS)) &&
                    set_equals<
                        tuple_v<VS...>,
                        typename detail::find_and_remove<V, tuple_v<WS...>, tuple_v<>>::type
                    >::value
         };
};

template <size_t ... WS>
struct set_equals<tuple_v<>, tuple_v<WS...>>
{
    enum { value = sizeof...(WS) == 0 };
};

template <typename S1, typename S2>
struct is_subset_of
{
    enum { value = ((size_of_tuple<S2>::value - size_of_tuple<S1>::value) ==
                     size_of_tuple<typename detail::find_and_remove_all<S1,S2>::type>::value) };
};

} // namespace dbtmp



















#endif // DBTMP_H
