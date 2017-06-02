#ifndef TMP_TYPES_H
#define TMP_TYPES_H

#include <cstddef>
#include <utility>
#include <type_traits>
#include "tmp.h"
#include "tmp_math.h"

namespace tmp
{
namespace types
{
namespace detail
{

    /* Refer to [basic.fundamental] in the C++ Standard
     *
     * (1)      {"char", "signed char", "unsigned char"} are distinct types but with the same storage and same alignment
     *
     * (2)      There are five standard signed integer types: "signed char", "short int", "int", "long int",
     *          "long long int".
     *
     * (3)      For all of the standard signed integer types, there are five corresponding standard unsigned integer types,
     *          with the same storage and alignment: "unsigned char", "unsigned short int", "unsigned int",
     *          "unsigned long int", "unsigned long long int".
     *
     * (4)      {"wchar_t", "char16_t", "char32_t"} all relies on underlying types.
     *
     *
     *  Conclusion: "wchar_t", "char16_t", "char32_t" will not be used or covered here in any way. Handle "char"
     *              as a special case (see "to_signed" and "to_unsigned" implementation here). All other combinations
     *              of standard integer types shall be handled.
     */

    /* Note that the implementation in this header relies on matching storage sizes and alignment of the elements at the
       same indices in "signed_fundamentals" and "unsigned_fundamentals" and it is why there are obvious redundancies in
       "unsigned_fundamentals". */
    typedef tuple_t<signed char,
                    signed short,
                    signed short int,
                    signed,
                    signed int,
                    signed long,
                    signed long int,
                    signed long long,
                    signed long long int,
                    short int,
                    int,
                    long int,
                    long long int> signed_fundamentals;
    typedef tuple_t<unsigned char,
                    unsigned short,
                    unsigned short int,
                    unsigned,
                    unsigned int,
                    unsigned long,
                    unsigned long int,
                    unsigned long long,
                    unsigned long long int,
                    unsigned short int,
                    unsigned int,
                    unsigned long int,
                    unsigned long long int> unsigned_fundamentals;

    typedef set_of<signed_fundamentals> signed_fundamentals_set;
    typedef set_of<unsigned_fundamentals> unsigned_fundamentals_set;

    template <typename T, typename FROMSET, typename TOSET>
    struct unsigned_signed_conversion_impl
    {
        enum { index = index_in<T,FROMSET> };
        typedef get_element_in<index,TOSET> type;
    };



} // namespace detail

    template <typename T>
    constexpr bool is_unsigned = static_cast<T>(-1) > static_cast<T>(0);
    template <typename T>
    constexpr bool is_signed = !is_unsigned<T>;

namespace detail
{
    template <typename T>
    struct containing_fundamental_type
    {
        typedef get_element_at<is_unsigned<T>, detail::signed_fundamentals, detail::unsigned_fundamentals> type;
    };

    template <typename T>
    struct to_signed_impl
    {
        typedef typename detail::unsigned_signed_conversion_impl<T,typename detail::containing_fundamental_type<T>::type,detail::signed_fundamentals>::type type;
    };
    template <>
    struct to_signed_impl<char>
    {
        typedef signed char type;
    };

    template <typename T>
    struct to_unsigned_impl
    {
        typedef typename detail::unsigned_signed_conversion_impl<T,typename detail::containing_fundamental_type<T>::type,detail::unsigned_fundamentals>::type type;
    };
    template <>
    struct to_unsigned_impl<char>
    {
        typedef unsigned char type;
    };

} // namespace detail

template <typename T>
using to_signed = typename detail::to_signed_impl<T>::type;
template <typename T>
using to_unsigned = typename detail::to_unsigned_impl<T>::type;

namespace detail
{
    template <typename T, T VAL, bool IS_ZERO>
    struct number_of_bits_of_value_impl
    {
        static constexpr T shifted_val = static_cast<T>(VAL << 1);
        enum { value = 1 + number_of_bits_of_value_impl<T, shifted_val, shifted_val == 0>::value };
    };
    template <typename T, T VAL>
    struct number_of_bits_of_value_impl<T,VAL,true>
    {
        enum { value = 0 };
    };
    template <typename T>
    struct number_of_bits_of_fundamental_
    {
        enum { value = number_of_bits_of_value_impl<to_unsigned<T>, static_cast<to_unsigned<T>>(1), (static_cast<to_unsigned<T>>(1) << 1) == 0>::value };
    };
} // namespace detail

    template <typename T>
    constexpr auto number_of_bits_of_fundamental = detail::number_of_bits_of_fundamental_<T>::value;

namespace detail
{
    template <size_t B>
    struct compare_type_size
    {
        template <typename R>
        struct with_size_of
        {
            enum { value = (detail::number_of_bits_of_fundamental_<R>::value == B) };
        };
    };
    template <size_t B>
    struct compare_alignment_size
    {
        template <typename R>
        struct with_alignment_of
        {
            enum { value = (alignof(R)*number_of_bits_of_fundamental<unsigned char> == B) };
        };
    };
    template <size_t S, size_t A>
    struct compare_size_and_alignment
    {
        template <typename R>
        struct with_size_and_alignment_of
        {
            enum { value = (
            (number_of_bits_of_fundamental<R> == S) &&
            (alignof(R)*number_of_bits_of_fundamental<unsigned char> == A)) };
        };
    };
} // namespace detail

    template <size_t S>
    using get_signed_fundamental_of_bit_size = to_signed<get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_type_size<S>::template with_size_of>>;
    template <size_t S>
    using get_unsigned_fundamental_of_bit_size = get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_type_size<S>::template with_size_of>;
    template <size_t A>
    using get_signed_fundamental_of_alignment = to_signed<get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_alignment_size<A>::template with_alignment_of>>;
    template <size_t A>
    using get_unsigned_fundamental_of_alignment = get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_alignment_size<A>::template with_alignment_of>;
    template <size_t S, size_t A>
    using get_signed_fundamental_of_bit_size_and_alignment = to_signed<get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_size_and_alignment<S,A>::template with_size_and_alignment_of>>;
    template <size_t S, size_t A>
    using get_unsigned_fundamental_of_bit_size_and_alignment = get_element_of<detail::unsigned_fundamentals_set>::template that_satisfies<detail::compare_size_and_alignment<S,A>::template with_size_and_alignment_of>;

namespace detail
{
    /* These definitions are required in order to select types with "smallest type that has at least x number of bits", etc */
    struct explicitly_sized_type
    {
    };

    template <typename T>
    struct sized_type;

} // namespace detail

template <typename R, typename T>
R fundamental_cast(detail::sized_type<T> const&);

namespace detail
{
    template <typename T>
    struct sized_type : public explicitly_sized_type
    {
    private:
        T data;
    public:

        sized_type()
        { }
        ~sized_type()
        { }
        sized_type(const sized_type & d) :
            data(d.data)
        { }
        template <typename FUNDAMENTAL>
        sized_type(const FUNDAMENTAL& c) :
            data(c)
        { }
        sized_type& operator=(const sized_type& c)
        {
            data = c.data;
            return *this;
        }
        /* This usually works but can incur compiler warnings */
        template <typename FUNDAMENTAL>
        sized_type& operator=(const FUNDAMENTAL& c)
        {
            data = c;
            return *this;
        }

        /* "Inherit" all operations that can be used on fundamental types here */
        // Addition
        friend sized_type operator+(sized_type const& l, sized_type const& r)
        {
            return sized_type(l.data + r.data);
        }
        sized_type& operator+=(sized_type& r)
        {
            data += r.data;
            return *this;
        }
        // Subtraction
        friend sized_type<T> operator-(sized_type<T> const& l, sized_type<T> const& r)
        {
            return sized_type<T>(l.data - r.data);
        }
        sized_type& operator-=(sized_type& r)
        {
            data -= r.data;
            return *this;
        }
        // Additive inverse
        friend sized_type<T> operator -(sized_type<T> const& l)
        {
            return sized_type<T>(-l.data);
        }
        // Multiplication
        friend sized_type<T> operator*(sized_type<T> const& l, sized_type<T> const& r)
        {
            return sized_type<T>(l.data * r.data);
        }
        sized_type& operator*=(sized_type const& r)
        {
            data *= r.data;
            return *this;
        }
        // Division
        friend sized_type<T> operator/(sized_type<T> const& l, sized_type<T> const& r)
        {
            return sized_type<T>(l.data / r.data);
        }
        sized_type& operator/=(sized_type const& r)
        {
            data /= r.data;
            return *this;
        }
        // Modulo
        friend sized_type<T> operator%(sized_type<T> const& l, sized_type<T> const& r)
        {
            return sized_type<T>(l.data % r.data);
        }
        sized_type& operator%=(sized_type const& r)
        {
            data %= r.data;
            return *this;
        }
        // Prefix increment
        sized_type& operator++()
        {
            data += 1;
            return *this;
        }
        // Postfix increment
        sized_type operator++(int)
        {
            data += 1;
            return *this;
        }
        // Prefix decrement
        sized_type& operator--()
        {
            data -= 1;
            return *this;
        }
        // Postfix decrement
        sized_type operator--(int)
        {
            data -= 1;
            return *this;
        }
        // Equal to
        friend bool operator ==(sized_type const& l, sized_type const& r)
        {
            return l.data == r.data;
        }
        // Not equal to
        friend bool operator !=(sized_type const& l, sized_type const& r)
        {
            return l.data != r.data;
        }
        // Greater than
        friend bool operator >(sized_type const& l, sized_type const& r)
        {
            return l.data > r.data;
        }
        // Less than
        friend bool operator <(sized_type const& l, sized_type const& r)
        {
            return l.data < r.data;
        }
        // Greater than or equal to
        friend bool operator >=(sized_type const& l, sized_type const& r)
        {
            return l.data >= r.data;
        }
        // Less than or equal to
        friend bool operator <=(sized_type const& l, sized_type const& r)
        {
            return l.data <= r.data;
        }
        // Logical negation
        friend sized_type<T> operator !(sized_type<T> l)
        {
            return sized_type<T>(!l.data);
        }
        // Logical AND
        friend sized_type<T> operator &&(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data && r.data);
        }
        // Logical OR
        friend sized_type<T> operator ||(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data || r.data);
        }
        // Bitwise NOT
        friend sized_type<T> operator ~(sized_type<T> l)
        {
            return sized_type<T>(~l.data);
        }
        // Bitwise AND
        friend sized_type<T> operator &(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data & r.data);
        }
        sized_type& operator &=(sized_type r)
        {
            data &= r.data;
            return *this;
        }
        // Bitwise OR
        friend sized_type<T> operator |(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data | r.data);
        }
        sized_type& operator |=(sized_type r)
        {
            data |= r.data;
            return *this;
        }
        // Bitwise XOR
        friend sized_type<T> operator ^(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data ^ r.data);
        }
        sized_type& operator ^=(sized_type r)
        {
            data ^= r.data;
            return *this;
        }
        // Bitwise left shift
        friend sized_type<T> operator <<(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data << r.data);
        }
        template <typename FUNDAMENTAL>
        friend sized_type<T> operator <<(sized_type<T> l, FUNDAMENTAL r)
        {
            return sized_type<T>(l.data << r);
        }
        sized_type& operator <<=(sized_type r)
        {
            data <<= r.data;
            return *this;
        }
        template <typename FUNDAMENTAL>
        sized_type& operator <<=(FUNDAMENTAL r)
        {
            data <<= r;
        }
        // Bitwise right shift
        friend sized_type<T> operator >>(sized_type<T> l, sized_type<T> r)
        {
            return sized_type<T>(l.data >> r.data);
        }
        template <typename FUNDAMENTAL>
        friend sized_type<T> operator >>(sized_type<T> l, FUNDAMENTAL r)
        {
            return sized_type<T>(l.data >> r);
        }
        sized_type& operator >>=(sized_type r)
        {
            data >>= r.data;
            return *this;
        }
        template <typename FUNDAMENTAL>
        sized_type& operator >>=(FUNDAMENTAL r)
        {
            data >>= r;
        }

        template <typename R, typename U>
        friend R tmp::types::fundamental_cast(sized_type<U> const& t);

    };
} // namespace detail

    template <typename R, typename U>
    R fundamental_cast(detail::sized_type<U> const& t){ return static_cast<R>(t.data); }

    template <size_t S, size_t A> using unsigned_sized_type = detail::sized_type<get_unsigned_fundamental_of_bit_size_and_alignment<S,A>>;
    template <size_t S, size_t A> using signed_sized_type = detail::sized_type<get_signed_fundamental_of_bit_size_and_alignment<S,A>>;

namespace detail
{
    template <typename T, size_t N>
    struct ones
    {
        static constexpr T value = math::power<2,N-1> + ones<T,N-1>::value;
    };
    template <typename T>
    struct ones<T,0>
    {
        static constexpr T value = 0;
    };
} // namespace detail

    template <size_t S, size_t A, size_t ... FIELDS>
    struct sized_bitfield;
    template <size_t S, size_t A>
    struct sized_bitfield<S,A> : public detail::explicitly_sized_type
    {
    private:
        typedef get_unsigned_fundamental_of_bit_size_and_alignment<S,A> type;
        type data;
    public:
        sized_bitfield(){}
        ~sized_bitfield(){}
        sized_bitfield(sized_bitfield<S,A> const& a) :
            data(a.data)
        { }
        template <typename FUNDAMENTAL>
        sized_bitfield(FUNDAMENTAL const& a) :
            data(a)
        { }
        template <size_t I,size_t U, size_t V>
        friend get_unsigned_fundamental_of_bit_size_and_alignment<U,V> get_field(sized_bitfield<U,V> const& a);

        template <size_t I, size_t U, size_t V, typename FUNDAMENTAL>
        friend void set_field(sized_bitfield<U,V> & a, FUNDAMENTAL const& s);
    };

    template <size_t I, size_t S, size_t A>
    get_unsigned_fundamental_of_bit_size_and_alignment<S,A> get_field(sized_bitfield<S,A> const& a)
    {
        static_assert(I == 0, "Index out of range.\n");
        return a.data;
    }

    template <size_t I, size_t S, size_t A, typename FUNDAMENTAL>
    void set_field(sized_bitfield<S,A> & a, FUNDAMENTAL const& s)
    {
        static_assert(I == 0, "Index out of range.\n");
        a.data = s;
    }

    template <size_t S, size_t A, size_t F, size_t ... FIELDS>
    struct sized_bitfield<S,A,F,FIELDS...> : public detail::explicitly_sized_type
    {
    private:
        typedef get_unsigned_fundamental_of_bit_size_and_alignment<S,A> type;
        type data;

    public:
        sized_bitfield(){}
        ~sized_bitfield(){}
        sized_bitfield(sized_bitfield<S,A,F,FIELDS...> const& c) :
            data(c.data)
        { }
        template <size_t I, size_t U, size_t V, size_t G, size_t ... GS>
        friend get_unsigned_fundamental_of_bit_size_and_alignment<U,V> get_field(sized_bitfield<U,V,G,GS...> const& a);

        template <size_t I, size_t U, size_t V, size_t ... FS, typename FUNDAMENTAL>
        friend void set_field(sized_bitfield<U,V,FS...> & a, FUNDAMENTAL const& s);
     private:
        template <size_t N, typename ... TS>
        struct helper
        {
            static void impl(sized_bitfield *, TS const& ...){}
        };
        template <size_t N, typename T, typename ... TS>
        struct helper<N,T,TS...>
        {
            static void impl(sized_bitfield * c, T const& t, TS const& ... ts)
            {
                set_field<N>(*c, t);
                helper<N+1,TS...>::impl(c, ts...);
            }
        };
     public:
        template <typename ... TS>
        sized_bitfield(TS ... ts) : data(0)
        {
            static_assert(sizeof...(TS) <= sizeof...(FIELDS)+1, "Too many arguments provided.\n");
            helper<0, TS...>::impl(this,ts...);
        }
    };

    template <size_t I, size_t S, size_t A, size_t F, size_t ... FIELDS>
    get_unsigned_fundamental_of_bit_size_and_alignment<S,A> get_field(sized_bitfield<S,A,F,FIELDS...> const& a)
    {
        static_assert(I >= 0 && I < sizeof...(FIELDS)+1, "Index out of range.\n");
        enum { offset = math::sum_first<I,tuple_i<F,FIELDS...>> };
        enum { size = get_element_at_i<I,F,FIELDS...>::value };
        return (a.data >> offset) & detail::ones<get_unsigned_fundamental_of_bit_size_and_alignment<S,A>,size>::value;
    }
    template <size_t I, size_t S, size_t A, size_t ... FIELDS, typename FUNDAMENTAL>
    void set_field(sized_bitfield<S,A,FIELDS...> & a, FUNDAMENTAL const& s)
    {
        static_assert(I >= 0 && I < sizeof...(FIELDS), "Index out of range.\n");
        enum { offset = math::sum_first<I,tuple_i<FIELDS...>> };
        enum { size = get_element_at_i<I,FIELDS...>::value };
        typedef get_unsigned_fundamental_of_bit_size_and_alignment<S,A> type;
        constexpr type mask = (~detail::ones<type,size>::value << offset)| detail::ones<type,offset>::value;
        a.data = (a.data & mask) | ((s << offset) & ~mask);
    }

namespace detail
{
    template <typename SEQ, typename ... TS>
    struct tpl_gen_leafs;
    template <size_t ... SEQ, typename ... TS>
    struct tpl_gen_leafs<tuple_i<SEQ...>,TS...> : protected ::tmp::detail::leaf_node<SEQ,TS>...
    {

    };
    template <typename ... TS>
    struct tuple;
} // namespace detail

template <size_t N, typename ... TS>
get_element_at<N,TS...>& get(detail::tuple<TS...> & t);

namespace detail
{
    template <typename T, typename ... TS>
    struct tuple<T,TS...> : public tpl_gen_leafs<sequence<sizeof...(TS)+1>,T,TS...>
    {
        template <size_t N, typename ... PS> using leaf_type = get_element_at<N,PS...>;
        template <size_t N, typename ... PS>
        friend leaf_type<N,PS...>& tmp::types::get(tuple<PS...> & t);

        tuple(){ }
        ~tuple(){ }
        tuple(T const&& t, TS const&& ... ts)
        {
            for_all<0,T,TS...>::init(*this, std::forward<const T>(t), std::forward<const TS>(ts)...);
        }

    private:
        template <size_t I, typename ... PS>
        struct for_all
        {
            static void init(tuple &, PS const&& ...){ }
        };
        template <size_t I, typename P, typename ... PS>
        struct for_all<I,P,PS...>
        {
            static void init(tuple & tup, P const&& p, PS const&& ... ps)
            {
                tmp::types::get<I>(tup) = std::forward<const P>(p);
                for_all<I+1,PS...>::init(tup,std::forward<const PS>(ps)...);
            }
        };
    };

}// namespace detail

template <size_t N, typename ... TS>
get_element_at<N,TS...>& get(detail::tuple<TS...> & t){ return get_leaf_data(static_cast<::tmp::detail::leaf_node<N,get_element_at<N,TS...>>&>(t),int_type<std::is_class<get_element_at<N,TS...>>::value>()); }
template <typename ... TS>
using tuple = detail::tuple<TS...>;

namespace detail{

    template <typename T>
    struct int_type_of_bit_size
    {
        typedef int_type<detail::number_of_bits_of_fundamental_<T>::value> type;
    };

    typedef map<int_type_of_bit_size,unsigned_fundamentals_set> unsigned_fundamentals_sizes;
    typedef unsigned_fundamentals_sizes signed_fundamentals_sizes;

    template <size_t S, size_t A>
    struct composed_sized_type : explicitly_sized_type
    {
    private:
        /* Select the type such that bitsoftype(type)*N=size where N is an integer*/
        enum { greatest_divisor = math::greatest_divisor<S, unsigned_fundamentals_sizes> };
    public:
    };

} // namespace detail
} // namespace types
} // namespace tmp


























#endif // TMP_TYPES_H
