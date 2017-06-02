#ifndef DBC_H
#define DBC_H

#include <fstream>

#include "../directory.h"
#include "dbc_record.h"
#include "../tmp/tmp_types.h"

enum class dbc_state
{
    BEGIN,
    CONFIGURED,
    LOADED
};

enum class dbc_error
{
    NO_ERROR,
    FILE_NOT_FOUND
};



struct dbc_header
{
    typedef tmp::types::get_unsigned_fundamental_of_bit_size_and_alignment<32,32> uint;
    uint wdbc;
    uint record_count;
    uint field_count;
    uint record_size;
    uint string_block_size;
};

template <typename DBC_FILE>
struct dbc_file : public DBC_FILE
{
private:
    using typename DBC_FILE::field_types;
    typedef dbc_record<field_types> record_type;
    using typename DBC_FILE::field_index;
    using DBC_FILE::key_field;
    using DBC_FILE::file_name;
    template <field_index I>
    using field_type = tmp::get_element_in<static_cast<size_t>(I), field_types>;

    dbc_state               m_state;
    dbc_error               m_error;
    unsigned int            m_number_of_entries;
    const dbc_directory *   m_directory;
    QString                 m_path;

    char       *            m_memory_block;


    inline const char *     get_record_data(unsigned int idx) const
    {
        return m_memory_block + sizeof(dbc_header) + idx*record_type::size;
    }

    template <unsigned int F>
    inline const char *     get_field_data(unsigned int idx) const
    {
        return get_record_data(idx) + record_type::template field_offset<F>::value;
    }

public:

    dbc_file()
    {
        m_state = dbc_state::BEGIN;
        m_error = dbc_error::NO_ERROR;
    }
    ~dbc_file()
    {
        if(m_state == dbc_state::LOADED)
            delete [] m_memory_block;
    }

    void configure(const dbc_directory & dir)
    {
        m_directory = &dir;
        m_path = dir.path() + QString{"/"} + QString{file_name.get_data()} + QString{".dbc"};
        m_state = dbc_state::CONFIGURED;
    }

    /* If the input data can be accessed, then return true, otherwise false */
    bool        is_valid() const        { return m_state == dbc_state::LOADED && m_error == dbc_error::NO_ERROR; }
    /* Display the error, or if no error, display success */
    std::string error_msg() const
    {
        switch(m_error)
        {
        case dbc_error::NO_ERROR:
            return {"No error."};
            break;
        case dbc_error::FILE_NOT_FOUND:
            return std::string{"File \""} + m_path.toStdString() + std::string{"\" was not found."};
            break;
        }
        return std::string{""};
    }
    /* Try to perform the load */
    void        load()
    {
        std::ifstream file(m_path.toStdString(), std::ios::in|std::ios::binary|std::ios::ate);
        std::streampos size;
        if(file.is_open())
        {
            size = file.tellg();
            m_memory_block = new char [size];
            file.seekg(0, std::ios::beg);
            file.read(m_memory_block,size);
            file.close();
            // Load header first
            m_state = dbc_state::LOADED;
            m_error = dbc_error::NO_ERROR;
        }
        else
        {
            m_error = dbc_error::FILE_NOT_FOUND;
        }
    }

    /* Perform necessary work to enable a succesful load */
    bool        correct_error() const   { return m_directory->add_file(QString{file_name.get_data()}); }
    /* Progress */
    float       progress_value() const  { return is_completed() ? 100.0f : 0.0f; }
    /* Is loading completed successfully? */
    bool        is_completed() const    { return m_state == dbc_state::LOADED; }
    /* Discard the data */
    void        discard()
    {
        if(m_state == dbc_state::LOADED)
        {
            m_state = dbc_state::CONFIGURED;
            delete [] m_memory_block;
        }
    }


    struct view
    {
    private:
        const dbc_file & m_dbc;
        template <unsigned int F>
        using field_store_type = typename record_type::template field_store_type<static_cast<unsigned int>(F)>;
    public:
        typedef dbc_file::record_type record_type;
        view(const dbc_file & d) : m_dbc(d) {}

        template <unsigned int F>
        inline const char * field(unsigned int idx) const
        {
            return m_dbc.get_field_data<F>(idx);
        }

        bool is_valid() const { return m_dbc.is_valid(); }
        std::string error_msg() const { return m_dbc.error_msg(); }
        bool correct_error() const { return m_dbc.correct_error(); }
        float progress_value() const { return m_dbc.progress_value(); }
        unsigned int count() const { return reinterpret_cast<dbc_header*>(m_dbc.m_memory_block)->record_count; }
        unsigned int string_block_size() const
        {
            return reinterpret_cast<dbc_header*>(m_dbc.m_memory_block)->string_block_size;
        }
        const char * string_block() const
        {
            return (m_dbc.m_memory_block + sizeof(dbc_header)) + count()*record_type::size;
        }
    };

    /* Get the data view */
    view operator()() const { return view{*this}; }
};

#endif // DBC_H
