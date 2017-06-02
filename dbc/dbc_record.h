#ifndef DBC_RECORD_H
#define DBC_RECORD_H

#include "../tmp/tmp.h"
#include "../tmp/tmp_math.h"
#include <tuple>

enum class dbc_field_type
{
    PTR,FLOAT,STRING,INT
};

template <size_t BYTES, dbc_field_type>
struct dbc_field
{
};

template <size_t BYTES>
using dbc_ptr = dbc_field<BYTES,dbc_field_type::PTR>;
template <size_t BYTES>
using dbc_float = dbc_field<BYTES,dbc_field_type::FLOAT>;
template <size_t BYTES>
using dbc_string = dbc_field<BYTES,dbc_field_type::STRING>;
template <size_t BYTES>
using dbc_int = dbc_field<BYTES,dbc_field_type::INT>;

namespace dbc_impl
{
    template <typename T>
    struct dbc_field_size;
    template <size_t BYTES, dbc_field_type T>
    struct dbc_field_size<dbc_field<BYTES,T>>
    {
        static constexpr unsigned int size = static_cast<unsigned int>(BYTES);
    };

    template <typename T>
    struct dbc_field_store_type;
    template <size_t BYTES>
    struct dbc_field_store_type<dbc_field<BYTES,dbc_field_type::PTR>>
    {
        typedef int* type;
        static type from_data(const char * data, const char * )
        {
            return *reinterpret_cast<const type*>(data);
        }
    };
    template <size_t BYTES>
    struct dbc_field_store_type<dbc_field<BYTES,dbc_field_type::INT>>
    {
        typedef int type;
        static type from_data(const char * data, const char * )
        {
            return *reinterpret_cast<const type*>(data);
        }
    };
    template <size_t BYTES>
    struct dbc_field_store_type<dbc_field<BYTES,dbc_field_type::FLOAT>>
    {
        typedef float type;
        static type from_data(const char * data, const char * )
        {
            return *reinterpret_cast<const type*>(data);
        }
    };

    template <size_t BYTES>
    struct dbc_field_store_type<dbc_field<BYTES,dbc_field_type::STRING>>
    {
        typedef const char* type;
        static type from_data(const char * data, const char * base)
        {
            return base + *reinterpret_cast<const unsigned int*>(data);
        }
    };

    template <typename T>
    struct if_string_then_add_offset_impl
    {
        static T f(T val, const char *) { return val; }
    };
    template <>
    struct if_string_then_add_offset_impl<const char*>
    {
        static const char* f(const char* val, const char * base)
        {
            return base + *reinterpret_cast<const unsigned int*>(val);
        }
    };

    template <typename T>
    T if_string_then_add_offset(T val, const char * base)
    {
        return if_string_then_add_offset_impl<T>::f(val,base);
    }

    template <typename T>
    struct dbc_field_less_than
    {
        inline bool operator () (const T & ls, const T & rs) const
        {
            return ls < rs;
        }
    };

    template <>
    struct dbc_field_less_than<const char *>
    {
        bool operator () (const char * ls, const char * rs) const
        {
            unsigned int offset = 0;
            while(ls[offset] != '\0' && rs[offset] != '\0')
            {
                if(ls[offset] < rs[offset])
                {
                    return true;
                }
                else if(ls[offset] > rs[offset])
                {
                    return false;
                }
                ++offset;
            }
            if(ls[offset] == rs[offset])
                return true;
            return false;
        }
    };

    template <>
    struct dbc_field_less_than<int*>
    {
        bool operator () (const int * ls, const int * rs) const
        {
            return *ls < *rs;
        }
    };


    template <typename T, template <typename...> class TUP, template <unsigned int> class F>
    struct tuple_t;
    template <template <typename...> class TUP, template <unsigned int> class F, unsigned int ... NS>
    struct tuple_t<tmp::tuple_i<NS...>,TUP,F>
    {
        typedef TUP<F<NS>...> type;
    };

    template <typename T, template <unsigned int> class F>
    struct projected_record;
    template <template <unsigned int> class F, unsigned int ... NS>
    struct projected_record<tmp::tuple_i<NS...>,F>
    {
        typedef tmp::tuple_t<F<NS>...> type;
    };

    template <typename T, typename ... FS>
    struct is_compatible;
    template <unsigned int ... NS, typename ... FS>
    struct is_compatible<tmp::tuple_i<NS...>,FS...>
    {
        static constexpr unsigned int size = (sizeof...(FS));
        static constexpr bool value = tmp::math::sum<NS >= size ...> == 0;
    };

    template <typename F>
    struct is_string
    {
        enum { value = false };
    };

    template <size_t BYTES>
    struct is_string<dbc_field<BYTES,dbc_field_type::STRING>>
    {
        enum { value = true };
    };

    template <typename ... FS>
    struct has_string
    {
        static constexpr bool value = tmp::math::sum<is_string<FS>::value ...> > 0;
    };

}

template <typename T>
constexpr unsigned int dbc_field_size = dbc_impl::dbc_field_size<T>::size;

template <typename T>
struct dbc_record;

template <typename ... FS>
struct dbc_record<tmp::tuple_t<FS...>>
{
    static constexpr unsigned int number_of_fields = sizeof...(FS);
    static constexpr unsigned int size = tmp::math::sum_tuple<tmp::tuple_i<dbc_field_size<FS>...>>;
    static constexpr bool has_string = dbc_impl::has_string<FS...>::value;

    template <unsigned int N>
    struct field_offset
    {
        enum { value = N == 0 ? 0 : tmp::math::sum_first<N == 0 ? 0 : N,tmp::tuple_i<dbc_field_size<FS>...>> };
    };

    template <unsigned int N>
    using field_store_type = typename dbc_impl::dbc_field_store_type<tmp::get_element_at<N, FS...>>::type;
    template <unsigned int N>
    using field_type = tmp::get_element_at<N, FS...>;

    template <typename T>
    using tuple_t = typename dbc_impl::tuple_t<T,std::tuple,field_store_type>::type;
    template <typename T>
    using projected_record = typename dbc_impl::projected_record<T,field_type>::type;

    template <unsigned int N>
    using field_less_than = dbc_impl::dbc_field_less_than<field_store_type<N>>;

    template <typename T, unsigned int N>
    struct less_than
    {
        bool operator () (const tuple_t<T> & ls, const tuple_t<T> & rs) const
        {
            return field_less_than<N>{}(std::get<N>(ls),std::get<N>(rs));
        }
    };
    template <unsigned int F, unsigned int S, typename T>
    struct compare_by_field_impl
    {
        static bool eval(unsigned int field, const tuple_t<T> & ls, const tuple_t<T> & rs)
        {
            return field == F ? less_than<T,F>{}(ls,rs) : compare_by_field_impl<F+1,S,T>::eval(field,ls,rs);
        }
    };
    template <unsigned int S, typename T>
    struct compare_by_field_impl<S,S,T>
    {
        static bool eval(unsigned int field, const tuple_t<T> & ls, const tuple_t<T> & rs)
        {
            return field == S ? less_than<T,S>{}(ls,rs) : false;
        }
    };
    template <typename T>
    static bool compare_by_field(unsigned int field, const tuple_t<T> & ls, const tuple_t<T> & rs)
    {
        return compare_by_field_impl<0,tmp::cardinality<T>,T>(field, ls, rs);
    }
    template <typename T>
    static bool return_field(unsigned int field, const tuple_t<T> & ls, const tuple_t<T> & rs)
    {
        return compare_by_field_impl<0,tmp::cardinality<T>,T>(field, ls, rs);
    }

    template <typename T>
    struct is_compatible
    {
        enum { value = dbc_impl::is_compatible<T,FS...>::value };
    };

};


namespace dbc_impl
{
    template <typename VIEW, typename RECORD_TYPE, typename M>
    struct dbc_project_on_tuple;
    template <typename VIEW, typename RECORD_TYPE, unsigned int ... NS>
    struct dbc_project_on_tuple<VIEW,RECORD_TYPE,tmp::tuple_i<NS...>>
    {
        typedef typename RECORD_TYPE:: template tuple_t<tmp::tuple_i<NS...>> tuple_t;
        template <unsigned int N>
        using dbc_field_type = dbc_field_store_type<typename RECORD_TYPE::template field_type<N>>;
        static inline tuple_t project(const VIEW & view, unsigned int idx, const char* string_block)
        {
            return tuple_t{dbc_field_type<NS>::from_data(view.template field<NS>(idx),string_block)...};
        }
    };
} // dbc_impl

#endif /* DBC_RECORD_H */
