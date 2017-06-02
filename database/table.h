#ifndef TABLE_H
#define TABLE_H

#include <vector>
#include <map>
#include <string>
#include <string.h>
#include <cstring>
#include <array>
#include "dbtmp.h"
#include <iostream>

template <size_t N>
using varchar = std::array<char,N>;


namespace dbutil
{
namespace detail
{
    template <typename U>
    struct estimated_size_used_by_type
    {
        enum { value = 2 };
    };
    template <>
    struct estimated_size_used_by_type<int>
    {
        enum { value = 8 };
    };
    template <size_t N>
    struct estimated_size_used_by_type<varchar<N>>
    {
        enum { value = (N*3)/4 };
    };


    template <typename P>
    struct get_field_from_query_helper;

    template <>
    struct get_field_from_query_helper<int>
    {
        template <size_t INDEX, typename I>
        static int get(I & i){ return i.get_data_at(INDEX);}
    };
    template <>
    struct get_field_from_query_helper<long long>
    {
        template <size_t INDEX, typename I>
        static long long get(I & i){ return i.get_longdata_at(INDEX);}
    };
    template <>
    struct get_field_from_query_helper<bool>
    {
        template <size_t INDEX, typename I>
        static bool get(I & i){ return static_cast<bool>(i.get_data_at(INDEX));}
    };
    template <size_t N>
    struct get_field_from_query_helper<varchar<N>>
    {
        template <size_t INDEX, typename I>
        static const char * get(I & i){ return i.get_string_at(INDEX);}
    };
    template <>
    struct get_field_from_query_helper<float>
    {
        template <size_t INDEX, typename I>
        static float get(I & i){ return i.get_float_at(INDEX);}
    };
    template <>
    struct get_field_from_query_helper<std::string>
    {
        template <size_t INDEX, typename I>
        static std::string get(I & i){ return std::string{i.get_string_at(INDEX)}; }
    };

    template <typename V>
    struct field_data
    {
    protected:
        V m_data;
    public:
        field_data(){}
        ~field_data(){}

        field_data(const V & d) :
            m_data(d)
        {

        }

        const V * get_data(){ return &m_data; }
        const char * get_data_string(){ return std::to_string(m_data).c_str(); }
    };

    template <>
    struct field_data<std::string>
    {
    protected:
        std::string m_data;
    public:
        field_data<std::string>() : m_data(std::string{""}){}
        ~field_data<std::string>(){}
        field_data(const std::string & s) :
            m_data(s)
        {

        }
        field_data(const char * cstr) :
            m_data(cstr)
        {

        }
        template <size_t N>
        field_data(const varchar<N> & s) :
            m_data(s.data())
        {

        }

        const std::string * get_data(){ return &m_data; }
        const char * get_data_string(){ return m_data.c_str(); }
    };

    template <size_t N>
    struct field_data<varchar<N>>
    {
    protected:
        varchar<N> m_data;
    public:
        field_data(){}
        ~field_data(){}

        field_data(const varchar<N> & d)
        {
            m_data = d;
        }
        field_data(const char * cstr)
        {
            strncpy(m_data.data(),cstr,N);
        }
        field_data(const std::string & str)
        {
            strncpy(m_data.data(),str.data(),N);
        }

        const varchar<N> * get_data() const { return &m_data; }
        const char * get_data_string() const { return m_data.data(); }
    };

    template <typename U>
    struct is_dbstring
    {
        enum { value = false };
    };
    template <size_t N>
    struct is_dbstring<varchar<N>>
    {
        enum { value = true };
    };
    template <typename U>
    struct dbstring_size
    {
        enum { value = 0 };
    };
    template <size_t N>
    struct dbstring_size<varchar<N>>
    {
        enum { value = N };
    };

    template <typename TS>
    void add_string_literal_begin_or_end(std::string &, const TS*)
    {

    }

    inline void add_string_literal_begin_or_end(std::string & str, const std::string*)
    {
        str.push_back('\'');
    }

    template <size_t N>
    void add_string_literal_begin_or_end(std::string & str, const varchar<N>*)
    {
        str.push_back('\'');
    }


} // namespace detail

// Parameter T is the table
template <typename T>
class table : public T
{
private:
    using typename T::field_index;
    using typename T::primary_key_fields;
    using T::field_name;
    using typename T::field_types;
    using T::table_name;
    template <field_index I>
    using field_type = dbtmp::get_type_at_tuple<static_cast<size_t>(I),typename T::field_types>;


    template <field_index I, bool IS_STRING>
    struct field_impl : public detail::field_data<field_type<I>>
    {

        const field_type<I> * get_data() const
        {
            return static_cast<detail::field_data<field_type<I>>*>(const_cast<field_impl*>(this))->get_data();
        }

        const char * get_string() const
        {
            return static_cast<detail::field_data<field_type<I>>*>(const_cast<field_impl*>(this))->get_data_string();
        }
        field_impl() :
            detail::field_data<field_type<I>>()
        {

        }

        field_impl(const field_type<I> & d) :
            detail::field_data<field_type<I>>(d)
        {
        }
        ~field_impl(){}

        field_impl& operator=(const field_impl & r)
        {
            this->m_data = *(r.get_data());
            return *this;
        }
        field_impl& operator=(const field_type<I> & r)
        {
            this->m_data = r;
            return *this;
        }


        bool operator ==(const field_impl& other) const
        {
            return *(this->get_data()) == *(other.get_data());
        }
        bool operator <(const field_impl& other) const
        {
            return *(this->get_data()) < *(other.get_data());
        }
        bool operator >(const field_impl& other) const
        {
            return *(this->get_data()) > *(other.get_data());
        }
        bool operator <=(const field_impl& other) const
        {
            return *(this->get_data()) <= *(other.get_data());
        }
        bool operator >=(const field_impl& other) const
        {
            return *(this->get_data()) >= *(other.get_data());
        }
        static constexpr const char * name = field_name[static_cast<size_t>(I)].data;
        static constexpr const size_t name_size = field_name[static_cast<size_t>(I)].size;
    };

    template <field_index I>
    struct field_impl<I, true> : public detail::field_data<field_type<I>>
    {
        using detail::field_data<field_type<I>>::get_data_string;
        using detail::field_data<field_type<I>>::get_data;

        const field_type<I> * get_data () const
        {
            return detail::field_data<field_type<I>>::get_data();
        }

        const char * get_string () const
        {
            return get_data_string();
        }
        field_impl() :
            detail::field_data<field_type<I>>("")
        {}
        field_impl(const field_type<I> & d) :
            detail::field_data<field_type<I>>(d)
        {
        }
        field_impl(const char * cstr) :
            detail::field_data<field_type<I>>(cstr)
        {
        }
        field_impl(const std::string & str) :
            detail::field_data<field_type<I>>(str)
        {
        }

        ~field_impl<I,true>(){}

        field_impl<I,true>& operator=(const field_impl<I,true> & r)
        {
            this->m_data = *(r.get_data());
            return *this;
        }
        field_impl<I,true>& operator=(const field_type<I> & d)
        {
            this->m_data = d;
            return *this;
        }
        field_impl<I,true>& operator=(const char * cstr)
        {
            strncpy(this->m_data.data(),cstr,detail::dbstring_size<field_type<I>>::value);
            return *this;
        }
        field_impl<I,true>& operator=(const std::string & str)
        {
            strncpy(this->m_data.data(),str.data(),detail::dbstring_size<field_type<I>>::value);
            return *this;
        }

        bool operator ==(const field_impl<I,true>& other) const
        {
            return std::strcmp((this->get_data())->data(),(other.get_data())->data()) == 0;
        }
        bool operator <(const field_impl<I,true>& other) const
        {
            return std::strcmp((this->get_data())->data(),(other.get_data())->data()) < 0;
        }
        bool operator >(const field_impl<I,true>& other) const
        {
            return std::strcmp((this->get_data())->data(),(other.get_data())->data()) > 0;
        }
        bool operator <=(const field_impl<I,true>& other) const
        {
            return std::strcmp((this->get_data())->data(),(other.get_data())->data()) <= 0;
        }
        bool operator >=(const field_impl<I,true>& other) const
        {
            return std::strcmp((this->get_data())->data(),(other.get_data())->data()) >= 0;
        }

        static constexpr const char * name = field_name[static_cast<size_t>(I)].get_data();
        static constexpr const size_t name_size = field_name[static_cast<size_t>(I)].get_size();
    };

public:

    template <field_index I>
    using field = field_impl<I,detail::is_dbstring<field_type<I>>::value>;

private:

    template <typename TS>
    struct table_record_impl;

    template <size_t ... SEQ>
    struct table_record_impl<dbtmp::tuple_v<SEQ...>>
    {
        typedef dbtmp::tuple<field<static_cast<field_index>(SEQ)>...> type;
        typedef dbtmp::tuple_t<field<static_cast<field_index>(SEQ)>...> type_t;
    };

    typedef typename table_record_impl<dbtmp::sequence<static_cast<size_t>(T::field_index::SIZE)>>::type table_record;
    typedef typename table_record_impl<dbtmp::sequence<static_cast<size_t>(T::field_index::SIZE)>>::type_t table_record_t;

    table_record default_record;

    template <typename TV>
    struct key_type_impl;

    template <size_t V>
    struct key_type_impl<dbtmp::tuple_v<V>>
    {
        typedef field<static_cast<field_index>(V)> type;
    };

    template <size_t ... VS>
    struct key_type_impl<dbtmp::tuple_v<VS...>>
    {
        typedef dbtmp::tuple<field_type<static_cast<field_index>(VS)>...> type;
    };

    typedef typename key_type_impl<primary_key_fields>::type key_type;

    /* When a new record is inserted it either overwrites an old record or is a completely new entry:
     *
     *  Conditions: C1 - Entry does not exist in 'insertions' and 'deletions'
     *              C2 - Entry does exist in 'insertions' but not in 'deletions'
     *              C3 - Entry exists in both 'insertions' and 'deletions'
     *              C4 - Entry does not exist in 'insertions' but exists in 'deletions'
     *
     *  Case 'overwrite': C1 -> Put overwritten entry in 'deletions' and insert new entry in 'insertions'
     *                    C2 -> Put new entry in 'insertions' only.
     *                    C3 -> Put new entry in 'insertions' only.
     *                    C4 -> Put new entry in 'insertions' only.
     *  Case 'new entry': C1 -> Insert new entry in 'insertions'
     *                    C2 -> This can not happen.
     *                    C3 -> This can not happen.
     *                    C4 -> Insert new entry in 'insertions'
     *  Case 'delete':    C1 -> Put deleted entry in deletions.
     *                    C2 -> Delete entry from insertions.
     *                    C3 -> Delete entry from insertions.
     *                    C4 -> Cant happen.
     *
     *  This is enough to construct forward and rollback queries.
     *
     *  Rollback: Delete all entries from 'insertions'. Then insert all entries from 'deletions'.
     *  Forward: Delete all entries from 'deletions'. Then insert all entries from 'insertions'.
     */
    std::map<key_type, table_record> insertions;
    std::map<key_type, table_record> deletions;

    template <typename ... FS>
    struct fields
    {
        enum { totalsize = 0 };
        enum { datasize = 0 };
        static void field_labels(std::string &) {}
        static void insert_into_fields_data(std::string &, const FS& ...) {}
        static void where_equals_to(std::string &, const FS& ...) {}
    };



    template <typename T::field_index I, bool B>
    struct fields<field_impl<I,B>>
    {
        static const size_t totalsize = field_impl<I,B>::name_size-1;
        enum { datasize = detail::estimated_size_used_by_type<field_type<I>>::value };
        static void field_labels(std::string & str)
        {
            str.append(field_impl<I,B>::name);
        }
        static void insert_into_fields_data(std::string & str, const field_impl<I,B>& f)
        {
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.append(f.get_string());
            detail::add_string_literal_begin_or_end(str,f.get_data());
        }
        static void where_equals_to(std::string & str, const field_impl<I,B>& f)
        {
            str.append(field_impl<I,B>::name);
            str.push_back('=');
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.append(f.get_string());
            detail::add_string_literal_begin_or_end(str,f.get_data());
        }
    };

    template <typename T::field_index I, bool B, typename FN, typename ... FS>
    struct fields<field_impl<I,B>, FN, FS...>
    {
        static const size_t totalsize = field_impl<I,B>::name_size-1 + fields<FN,FS...>::totalsize;
        enum { datasize = detail::estimated_size_used_by_type<field_type<I>>::value + fields<FN,FS...>::datasize };

        static void field_labels(std::string & str)
        {
            str.append(field_impl<I,B>::name);
            str.push_back(',');
            fields<FN,FS...>::field_labels(str);
        }

        static void insert_into_fields_data(std::string & str, const field_impl<I,B>& f, const FN& fn, const FS& ... fs)
        {
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.append(f.get_string());
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.push_back(',');
            fields<FN,FS...>::insert_into_fields_data(str,fn,fs...);
        }

        static void where_equals_to(std::string & str, const field_impl<I,B>& f, const FN& fn, const FS& ... fs)
        {
            str.append(field_impl<I,B>::name);
            str.push_back('=');
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.append(f.get_string());
            detail::add_string_literal_begin_or_end(str,f.get_data());
            str.append(" AND ");
            fields<FN,FS...>::where_equals_to(str,fn,fs...);
        }
    };

    template <typename TS>
    struct fields_;
    template <typename ... FS>
    struct fields_<dbtmp::tuple_t<FS...>>
    {
        typedef fields<FS...> type;
    };

public:

    void set_default()
    {
    }

    template <field_index I , typename ... FS>
    void set_default(const field<I> & f, const FS & ... fs)
    {
        dbtmp::get<static_cast<size_t>(I)>(default_record) = *f.get_data();
        set_default(fs...);
    }

private:

    template <typename TS>
    struct init_default;

    template <field_index I, bool B>
    struct init_default<dbtmp::tuple_t<field_impl<I,B>>>
    {
        static void init(table_record & r)
        {
            dbtmp::get<static_cast<size_t>(I)>(r) = field<I>();
        }
    };

    template <field_index I, bool B, typename ... FS>
    struct init_default<dbtmp::tuple_t<field_impl<I,B>,FS...>>
    {
        static void init(table_record & r)
        {
            dbtmp::get<static_cast<size_t>(I)>(r) = field<I>();
            init_default<dbtmp::tuple_t<FS...>>::init(r);
        }
    };
public:

    constexpr const char * tablename() const { return T::table_name.get_data(); }

private:

    template <typename TS, typename ... FS>
    struct get_indices;
    template <size_t ... IS, field_index I, bool B, typename ... FS>
    struct get_indices<dbtmp::tuple_v<IS...>, field_impl<I,B>, FS...>
    {
        typedef typename get_indices<dbtmp::tuple_v<IS..., static_cast<size_t>(I)>, FS...>::type type;
    };
    template <size_t ... IS>
    struct get_indices<dbtmp::tuple_v<IS...>>
    {
        typedef dbtmp::tuple_v<IS...> type;
    };

    template <field_index G, field_index I, bool B>
    static field<G>& get_field(const table_record& r,const field_impl<I,B>& f)
    {
        return (static_cast<size_t>(G) == static_cast<size_t>(I) ? reinterpret_cast<field<G>&>(const_cast<field_impl<I,B>&>(f)) :
                                                                   static_cast<field<G>&>(dbtmp::get<static_cast<size_t>(G)>(r)));
    }

    template <field_index G, field_index I, bool B, typename ... FS>
    static field<G>& get_field(const table_record& r, const field_impl<I,B>& f, const FS& ... fs)
    {
        return (static_cast<size_t>(G) == static_cast<size_t>(I) ? reinterpret_cast<field<G>&>(const_cast<field_impl<I,B>&>(f)) :
                                                                   static_cast<field<G>&>(get_field<G>(r,fs...)));
    }

    template <field_index G, typename I>
    static field<G> get_field_from_query(I & i)
    {
        return field<G>{detail::get_field_from_query_helper<field_type<G>>::template get<static_cast<size_t>(G)>(i)};
    }

public:

    table()
    {
        init_default<table_record_t>::init(default_record);
    }

    ~table()
    {
    }
private:
    template <typename ... FS>
    void insert_into_str(std::string & str, FS ... fs) const
    {
        str.append("INSERT INTO ");
        str.append(tablename());
        if(!dbtmp::types_equal<dbtmp::tuple_t<FS...>, table_record_t>::value)
        {
            str.append(" (");
            fields<FS...>::field_labels(str);
            str.append(")");
        }
        str.append(" VALUES (");
        fields<FS...>::insert_into_fields_data(str,fs...);
        str.append(");");
    }

public:
    template <typename I, typename ... FS>
    std::string insert_into(I & i, FS ... fs)
    {
        /* INSERT INTO table_name (FS...) VALUES (fs...); */
        const int estimated_capacity =
                sizeof("INSERT INTO ") +
                T::table_name.size +
                2 +
                fields<FS...>::totalsize +
                sizeof...(FS) - 1 +
                sizeof(") VALUES (") +
                sizeof...(FS) - 1 +
                2 +
                fields<FS...>::datasize +
                50; // Just guess
        std::string query;
        query.reserve(estimated_capacity);
        insert_into_str(query, fs...);
        i.query(query.c_str());
        return query;
    }

private:
    template <typename ... KS>
    void delete_from_str(std::string & str, KS ... ks) const
    {
        str.append("DELETE FROM ");
        str.append(tablename());
        str.append(" WHERE ");
        fields<KS...>::where_equals_to(str,ks...);
        str.push_back(';');
    }

public:

    template <typename I, typename ... KS>
    std::string delete_from(I & i, KS ... ks)
    {
        /* Ensure that KS... are the key field */
        static_assert(dbtmp::set_equals<typename T::primary_key_fields,typename get_indices<dbtmp::tuple_v<>, KS...>::type>::value,
                      "Arguments does not constitute a primary key for this table.");
        /* DELETE FROM table_name WHERE prmky0=V0 AND prmky1=V1...; */
        const int estimated_capacity =
                sizeof("DELETE FROM ") +
                T::table_name.size +
                sizeof(" WHERE ") +
                (sizeof...(KS)-1) * sizeof(" AND ") +
                fields<KS...>::totalsize +
                sizeof...(KS) * sizeof("=") +
                fields<KS...>::datasize;
        std::string query;
        query.reserve(estimated_capacity);
        delete_from_str(query,ks...);
        i.query(query.c_str());
        return query;
    }

private:
    template <typename ... KS>
    void select_from_str(std::string & str, KS ... ks) const
    {
        str.append("SELECT ");
        fields_<table_record_t>::type::field_labels(str);
        str.append(" FROM ");
        str.append(tablename());
        str.append(" WHERE ");
        fields<KS...>::where_equals_to(str,ks...);
        str.push_back(';');
    }

public:

    template <typename I, typename ... KS>
    std::string select_from(I & i, KS ... ks)
    {
        /* Ensure that KS... are the key field */
        static_assert(dbtmp::set_equals<typename T::primary_key_fields,typename get_indices<dbtmp::tuple_v<>, KS...>::type>::value,
                      "Arguments does not constitute a primary key for this table.");
        /* SELECT c1,c2,c3...  FROM table_name WHERE prmky0=V0 AND prmky1=V1...; */
        const int estimated_capacity =
                sizeof("SELECT ") +
                fields_<table_record_t>::type::totalsize +
                dbtmp::size_of_tuple<table_record_t>::value - 1 +
                sizeof(" FROM ") +
                T::table_name.size +
                (sizeof...(KS)-1) * sizeof(" WHERE ") +
                fields<KS...>::totalsize +
                sizeof...(KS) * sizeof("=") +
                fields<KS...>::datasize;
        std::string query;
        query.reserve(estimated_capacity);
        select_from_str(query,ks...);
        i.query(query.c_str());
        return query;
    }

private:
    template <typename K>
    struct record_helper;
    template <size_t V>
    struct record_helper<dbtmp::tuple_v<V>>
    {
        template <typename ... FS>
        static key_type get_key(const table_record & r, const FS& ... fs)
        {
            return get_field<static_cast<field_index>(V)>(r,fs...);
        }
        template <typename ... FS>
        static table_record get_record(const table_record & r, const FS& ... fs)
        {
            return table_record{get_field<static_cast<field_index>(V)>(r,fs...)};
        }
        template <typename I>
        static table_record load_record_from_query(I & i)
        {
            return table_record{get_field_from_query<static_cast<field_index>(V)>(i)};
        }
        template <typename ... FS>
        static void insert_into_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.insert_into_str(str, dbtmp::get<V>(t));
        }
        template <typename ... FS>
        static void delete_from_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.delete_from_str(str, dbtmp::get<V>(t));
        }
        template <field_index FI>
        static void delete_from_str(const table & tb, std::string & str, const field<FI>& f)
        {
            tb.delete_from_str(str,f);
        }
        template <typename ... FS>
        static void select_from_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.select_from_str(str, dbtmp::get<V>(t));
        }
        template <field_index FI>
        static void select_from_str(const table & tb, std::string & str, const field<FI>& f)
        {
            tb.select_from_str(str,f);
        }
        template <typename I, typename ... FS>
        static std::string insert_into(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.insert_into(i, dbtmp::get<V>(t));
        }
        template <typename I, typename ... FS>
        static std::string delete_from(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.delete_from(i, dbtmp::get<V>(t));
        }
        template <typename I, field_index FI>
        static std::string delete_from(table & tb, I & i, const field<FI>& f)
        {
            return tb.delete_from(i,f);
        }
        template <typename I, typename ... FS>
        static std::string select_from(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.select_from(i, dbtmp::get<V>(t));
        }
        template <typename I, field_index FI>
        static std::string select_from(table & tb, I & i, const field<FI>& f)
        {
            return tb.select_from(i,f);
        }
    };
    template <size_t ... VS>
    struct record_helper<dbtmp::tuple_v<VS...>>
    {
        template <typename ... FS>
        static key_type get_key(const table_record& r,const FS& ... fs)
        {
            return key_type{get_field<static_cast<field_index>(VS)>(r,fs...)...};
        }
        template <typename ... FS>
        static table_record get_record(const table_record& r,const FS& ... fs)
        {
            return table_record{get_field<static_cast<field_index>(VS)>(r,fs...)...};
        }
        template <typename I>
        static table_record load_record_from_query(I & i)
        {
            return table_record{get_field_from_query<static_cast<field_index>(VS)>(i)...};
        }
        template <typename ... FS>
        static void insert_into_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.insert_into_str(str, dbtmp::get<VS>(t)...);
        }
        template <typename ... FS>
        static void delete_from_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.delete_from_str(str, dbtmp::get<VS>(t)...);
        }
        template <typename ... FS>
        static void select_from_str(const table & tb, std::string & str, const dbtmp::tuple<FS...>& t)
        {
            tb.select_from_str(str, dbtmp::get<VS>(t)...);
        }
        template <typename I, typename ... FS>
        static std::string insert_into(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.insert_into(i, dbtmp::get<VS>(t)...);
        }
        template <typename I, typename ... FS>
        static std::string delete_from(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.delete_from(i, dbtmp::get<VS>(t)...);
        }
        template <typename I, typename ... FS>
        static std::string select_from(table & tb, I & i, const dbtmp::tuple<FS...>& t)
        {
            return tb.select_from(i, dbtmp::get<VS>(t)...);
        }

    };

public:

    template <typename I, typename ... FS>
    std::string insert_entry(I & i, FS ... fs)
    {
        typedef dbtmp::sequence<dbtmp::size_of_tuple<table_record>::value> TBLSEQ;
        // Get the key
        static_assert(dbtmp::is_subset_of<primary_key_fields, typename get_indices<dbtmp::tuple_v<>, FS...>::type>::value,
                      "A primary key could not be constituted from any of the provided arguments.");
        key_type primary_key = record_helper<primary_key_fields>::get_key(default_record,fs...);
        // Get the record to be inserted
        table_record new_record = record_helper<TBLSEQ>::get_record(default_record,fs...);
        auto itd = deletions.find(primary_key);
        auto iti = insertions.find(primary_key);
        /* It does exist in deletions, which means we will replace insertions */
        if(itd != deletions.end())
        {
            //qDebug("The record with the same primary key has been deleted already.\n");
            if(iti != insertions.end())
            {
                //qDebug("... and it was deleted to make place for another item with the same primary key.\n");
                if((*iti).second == new_record)
                {
                    //qDebug("Since the record data mirrors what has been placed already, we simply abort here.\n");
                    return "";
                }
                record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
                if(i.no_error_occured())
                {
                    //qDebug("... so we delete the newest entry from the database.\n");
                    insertions.erase(iti);
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
            }
            if((*itd).second == new_record)
            {
                //qDebug("We now found out that what we want to insert is exactly what we originally deleted. So we insert that data back again.\n");
                record_helper<TBLSEQ>::insert_into(*this,i,new_record);
                if(i.no_error_occured())
                {
                    //qDebug("... and we can now delete the recordings of what was deleted.\n");
                    deletions.erase(itd);
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
                return "";
            }
            else
            {
                //qDebug("We found out that what we want to insert is new data for this session.\n");
                record_helper<TBLSEQ>::insert_into(*this,i,new_record);
                if(i.no_error_occured())
                {
                    //qDebug("... so we record what that data is.\n");
                    insertions.insert(std::pair<key_type,table_record>{primary_key,new_record});
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
                return "";
            }
        }
        /* It does not exist in deletions and not in insertions, we know nothing so we have to query db */
        else if(iti == insertions.end())
        {
            //qDebug("The primary key of the entry we want to insert was not previously recorded to have changed.\n");
            bool found = false;
            /* First save (if found) the record to be replaced in the db */
            record_helper<primary_key_fields>::select_from(*this,i,primary_key);
            if( i.no_error_occured() && (found = i.next()))
            {
                //qDebug("... so we first query the database and find that there already exists an entry with the same key.\n");
                table_record old_record = record_helper<TBLSEQ>::load_record_from_query(i);
                if(new_record == old_record)
                {
                    //qDebug("... and we see that what we want to insert is what is already there. Therefore simply abort.\n");
                    return "";
                }
                //qDebug("... and we see that what we want to insert does not match what we want to insert. Therefore we must delete that data and record it.\n");
                deletions.insert(std::pair<key_type,table_record>{primary_key,old_record});
                /* Then delete it */
                record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
            }
            if((!found) || (found && i.no_error_occured()))
            {
                //qDebug("We can now safely insert the new data.\n");
                /* Then insert the new record */
                record_helper<TBLSEQ>::insert_into(*this,i,new_record);
                if(i.no_error_occured())
                {
                    insertions.insert(std::pair<key_type,table_record>{primary_key,new_record});
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
            }
            else
            {
                // Rollback last transaction if possible, report error status
            }
        }
        /* It does not exist in deletions but exists in insertions */
        else
        {
            //qDebug("We have previously inserted this data and didnt need to delete an older entry while doing so.\n");
            if((*iti).second == new_record)
            {
                //qDebug("It was found that this old data is exactly what we want to insert. Simply abort here.\n");
                return "";
            }
            //qDebug("... so delete this old entry.\n");
            record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
            if(i.no_error_occured())
            {
                insertions.erase(primary_key);
                record_helper<TBLSEQ>::insert_into(*this,i,new_record);
                if(i.no_error_occured())
                {
                    //qDebug("... and now we can safely insert the new entry.\n");
                    insertions.insert(std::pair<key_type,table_record>{primary_key,new_record});
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
            }
            else
            {
                // Rollback last transaction if possible, report error status
            }
        }
        return "";
    }

    template <typename I, typename ... KS>
    std::string delete_entry(I & i, KS ... ks)
    {
        typedef dbtmp::sequence<dbtmp::size_of_tuple<table_record>::value> TBLSEQ;
        static_assert(dbtmp::set_equals<typename T::primary_key_fields,typename get_indices<dbtmp::tuple_v<>, KS...>::type>::value,
                      "Arguments does not constitute a primary key for this table.");

        key_type primary_key = record_helper<primary_key_fields>::get_key(default_record,ks...);
        auto itd = deletions.find(primary_key);
        auto iti = insertions.find(primary_key);
        /* It does exist in deletions */
        if(itd != deletions.end())
        {
            if(iti != insertions.end())
            {
                record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
                if(i.no_error_occured())
                {
                    insertions.erase(iti);
                }
                else
                {
                    // Rollback last transaction if possible, report error status
                }
            }
        }
        /* It does not exist in deletions and not in insertions */
        else if(iti == insertions.end())
        {
            bool found = false;
            /* First save (if found) the record to be replaced in the db */
            record_helper<primary_key_fields>::select_from(*this, i,primary_key);
            if( i.no_error_occured() && (found = i.next()))
            {
                table_record old_record = record_helper<TBLSEQ>::load_record_from_query(i);
                deletions.insert(std::pair<key_type,table_record>{primary_key,old_record});
                /* Then delete it */
                record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
            }
        }
        /* It does not exist in deletions but exists in insertions */
        else
        {
            record_helper<primary_key_fields>::delete_from(*this,i,primary_key);
            if(i.no_error_occured())
            {
                insertions.erase(primary_key);
            }
            else
            {
                // Rollback last transaction if possible, report error status
            }
        }
        return "";
    }


    template <typename I, field_index IDX>
    field_type<IDX> max(I & i, const field<IDX>) const
    {
        std::string query = "SELECT MAX(";
        query.append(field<IDX>::name);
        query.append(") FROM ");
        query.append(tablename());
        query.append(";");
        i.query(query.data());
        if(i.next())
        {
            return detail::get_field_from_query_helper<field_type<IDX>>::template get<static_cast<size_t>(IDX)>(i);
        }
        return field_type<IDX>{};
    }

    std::string get_table_patch() const
    {
        typedef dbtmp::sequence<dbtmp::size_of_tuple<table_record>::value> TBLSEQ;
        std::string str;
        // First print whats to be deleted
        for(auto it = deletions.begin(); it != deletions.end(); ++it)
        {
            record_helper<primary_key_fields>::delete_from_str(*this,str,(*it).first);
            str.push_back('\n');
        }
        // Then print what data to insert
        for(auto it = insertions.begin(); it != insertions.end(); ++it)
        {
            record_helper<TBLSEQ>::insert_into_str(*this,str,(*it).second);
            str.push_back('\n');
        }
        return std::string(str);
    }

    std::string get_table_rollback_patch() const
    {
        typedef dbtmp::sequence<dbtmp::size_of_tuple<table_record>::value> TBLSEQ;
        std::string str;
        // First print whats to be deleted
        for(auto it = insertions.begin(); it != insertions.end(); ++it)
        {
            record_helper<primary_key_fields>::delete_from_str(*this,str,(*it).first);
            str.push_back('\n');
        }
        // Then print what data to insert
        for(auto it = deletions.begin(); it != deletions.end(); ++it)
        {
            record_helper<TBLSEQ>::insert_into_str(*this,str,(*it).second);
            str.push_back('\n');
        }
        return std::string(str);
    }

    bool is_modified() const
    {
        return (insertions.size() || deletions.size());
    }

};

} // namespace dbutil

#endif // TABLE_H
