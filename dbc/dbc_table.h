#ifndef DBC_TABLE_H
#define DBC_TABLE_H

#include <vector>
#include "dbc/dbc_files.h"
#include "dbc/dbc_projection.h"
#include "dbc/dbc.h"

enum class dbc_table_state
{
    BEGIN,
    CONFIGURED,
    LOADED
};

enum class dbc_table_error
{
    NO_ERROR,
    INVALID_SOURCE
};

/*
 *  Given is a table T with n rows of records, where each record is uniquely identified by it's key fields
 *
 *  The table T is not mutable at all, so we can identify each row uniquely also by it's index i.
 *  So T[i] is said to be a row with index i. We now need a mapping k -> i where keys k are sorted for fast lookup (logn),
 *  Then T[k] is also allowed but implemented as T[f(k)] = T[i].
 *
 *  But now we also want this table to be sorted by any column, so we apply a sort on all rows and and get a list of
 *  keys in a new order and this order will be unordered with regards to the key, but ordered with regards to the column.
 *  This is a lookup si -> k where si is "sorted index".
 *
 *  The last thing we need is a lookup with k -> si
 */

template <typename K>
struct key_t
{
    unsigned int    index;
    K               key;
    unsigned int    sorted_index;
    key_t(unsigned int idx, K k, unsigned int sorted_idx) : index(idx), key(k), sorted_index(sorted_idx) {}
};

template <typename K>
struct si_t
{
    key_t<K> * key;
    si_t(key_t<K> & k) : key(&k) {}
};

template <typename K, typename RECORD>
struct key_index_lookup_table
{
private:
    std::vector<RECORD>     data;
    std::vector<key_t<K>>   keys;
    std::vector<si_t<K>>    sorted_keys;

    unsigned int lookup_key_impl(const K & k, unsigned int l, unsigned int u) const
    {
        if(dbc_impl::dbc_field_less_than<K>{}(k,keys[(l+u)/2].key))
        {
            return lookup_key_impl(k,l,((l+u)/2));
        }
        else if(dbc_impl::dbc_field_less_than<K>{}(keys[(l+u)/2].key,k))
        {
            return lookup_key_impl(k,((l+u)/2)+1,u);
        }
        return keys[(l+u)/2].index;
    }

    unsigned int lookup_key(const K & k) const
    {
        return lookup_key_impl(k,0,keys.size()-1);
    }
public:

    template <typename F>
    void push_back(RECORD r, F f)
    {
        data.push_back(r);
        keys.push_back(key_t<K>{data.size()-1,f(r),data.size()-1});
    }

    /* Before using any other operation than push on this class, sort_keys() must be performed. */
    void sort_keys()
    {
        sorted_keys.clear();
        std::sort(keys.begin(),keys.end(),[](const key_t<K> & ls, const key_t<K> & rs)
        {
            return dbc_impl::dbc_field_less_than<K>{}(ls.key,rs.key);
        });

        for(unsigned int i = 0; i < keys.size(); ++i)
        {
            sorted_keys.push_back(si_t<K>{keys[i]});
        }
    }


    void sort_by(bool (*cmp)(const RECORD&,const RECORD&))
    {
        std::sort(sorted_keys.begin(),sorted_keys.end(), [=](const si_t<K> & L, const si_t<K> & R)
        {
            return cmp(data[(L.key)->index],data[(R.key)->index]);
        });
        for(unsigned int i = 0; i < sorted_keys.size(); ++i)
        {
            (sorted_keys[i].key)->sorted_index = i;
        }
    }

    const RECORD & at_index(unsigned int idx) const
    {
        return data[lookup_key((*(sorted_keys[idx].key)).key)];
    }
    const RECORD & at_key(const K & key) const
    {
        return data[lookup_key(key)];
    }
    unsigned int key_index(const K & key) const
    {
        return lookup_key(key);
    }

    void clear()
    {
        data.clear();
        keys.clear();
        sorted_keys.clear();
    }
    unsigned int size() const { return data.size(); }
};

struct empty_string
{
    const char * string_block() { return nullptr; }
    void reserve(unsigned int){}
    void copy(const char *, unsigned int){}
};

struct string_wrapper
{
    char * m_data;
    const char * string_block() { return m_data; }
    void reserve(unsigned int n)
    {
        m_data = new char[n];
    }
    void copy(const char * begin, unsigned int count)
    {
        memcpy(m_data,begin,count);
    }

    string_wrapper() : m_data(nullptr) {}
    ~string_wrapper()
    {
        if(m_data)
            delete [] m_data;
    }
};

template <typename VIEW, typename PROJECTION>
struct dbc_table_types
{
    typedef typename VIEW::record_type record_type;
    using map = typename PROJECTION::map;
    typedef typename record_type::template tuple_t<map> record_t;
    typedef typename record_type::template projected_record<map> projected_record;

    enum { has_string = dbc_record<projected_record>::has_string };
};


template <typename VIEW, typename PROJECTION>
class dbc_table :
        /* If there are strings in the projection, we need a vector with all string data from view. */
        public tmp::get_element_at<dbc_table_types<VIEW,PROJECTION>::has_string,empty_string,string_wrapper>
{
private:
    typedef typename VIEW::record_type record_type;
    using map = typename PROJECTION::map;
    using map_index = typename PROJECTION::map_index;

    typedef typename record_type::template tuple_t<map> record_t;

    static_assert(record_type::template is_compatible<map>::value,"A projection mapping must refer to field indices in the view.");

    template <map_index I>
    using map_index_type = typename record_type::template field_store_type<tmp::get_element_in<static_cast<unsigned int>(I), map>::value>;

    typedef map_index_type<PROJECTION::map_key> map_key_type;

    unsigned int                                    m_order_by_field;
    key_index_lookup_table<map_key_type, record_t>  m_lookup_table;

    void sort_by(unsigned int column)
    {
        m_lookup_table.sort_by([=](const record_t& L, const record_t& R)
        {
            return VIEW::record_type::compare_by_field(column,L,R);
        });
    }

    dbc_table_state             m_state;
    dbc_table_error             m_error;
    const VIEW *                m_view;

public:

    dbc_table()
    {
        m_state = dbc_table_state::BEGIN;
        m_error = dbc_table_error::NO_ERROR;
        m_order_by_field = static_cast<unsigned int>(PROJECTION::map_key);
    }
    ~dbc_table(){}

    void configure(const VIEW & view)
    {
        m_view = &view;
        m_state = dbc_table_state::CONFIGURED;
        if(!m_view->is_valid())
            m_error = dbc_table_error::INVALID_SOURCE;
    }

    void load()
    {
        if(m_state == dbc_table_state::CONFIGURED && m_error != dbc_table_error::INVALID_SOURCE)
        {
            if(dbc_table_types<VIEW,PROJECTION>::has_string)
            {
                this->reserve(m_view->string_block_size());
                this->copy(m_view->string_block(),m_view->string_block_size());
            }
            const char * string_block_begin = this->string_block();
            for(unsigned int i = 0; i < m_view->count(); ++i)
            {
                auto key_data_f = [](const record_t & t) -> map_key_type
                {
                    return std::get<static_cast<unsigned int>(PROJECTION::map_key)>(t);
                };
                m_lookup_table.push_back(dbc_impl::dbc_project_on_tuple<VIEW,record_type,map>::project(*m_view,i,string_block_begin),
                                         key_data_f);
            }
            m_lookup_table.sort_keys();

            m_state = dbc_table_state::LOADED;
        }
    }

    bool is_valid() const { return (m_state == dbc_table_state::CONFIGURED) ? m_view.is_valid() : true; }

    std::string error_msg() const
    {
        switch(m_error)
        {
        case dbc_table_error::NO_ERROR:
            return "No error.";
        case dbc_table_error::INVALID_SOURCE:
            return std::string{"Invalid source ("} + m_view->error_msg() + std::string{")"};
        }
        return "";
    }

    bool correct_error() const { return m_view->correct_error(); }

    float progress_value() const
    {
        return (m_state == dbc_table_state::CONFIGURED ? 100.0f*float(m_lookup_table.size())/float(m_view->count()) : 0.0f);
    }

    bool is_completed() const { return m_state == dbc_table_state::LOADED; }

    /* Discard the data */
    void discard()
    {
        if(m_state == dbc_table_state::LOADED)
        {
            m_state = dbc_state::CONFIGURED;
            m_lookup_table.clear();
        }
        m_error = dbc_error::NO_ERROR;
        m_order_by_field = static_cast<unsigned int>(PROJECTION::map_key);
    }

    struct view
    {
    private:
        const dbc_table & m_table;
    public:
        typedef dbc_table::record_t record_t;
        view(const dbc_table & t) : m_table(t) {}

        inline const record_t& record_at(unsigned int idx) const
        {
            return m_table.m_lookup_table.at_index(idx);
        }
        inline unsigned int key_index(const map_key_type& key) const
        {
            return m_table.m_lookup_table.key_index(key);
        }

        bool is_valid() const { return m_table.is_valid(); }
        std::string error_msg() const { return m_table.error_msg(); }
        bool correct_error() const { return m_table.correct_error(); }
        float progress_value() const { return m_table.progress_value(); }
        unsigned int count() const { return m_table.m_lookup_table.size(); }
    };

    /* Get the view */
    view operator()() const
    {
        return view{*this};
    }
};

#endif /* DBC_TABLE_H */
