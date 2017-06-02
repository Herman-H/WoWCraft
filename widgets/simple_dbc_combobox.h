#ifndef SIMPLE_DBC_COMBOBOX_H
#define SIMPLE_DBC_COMBOBOX_H

#include <QComboBox>
#include "../directory.h"
#include "../dbc/dbc.h"
#include "../dbc/dbc_files.h"
#include "dbc/dbc_table.h"
#include "dbc_item_model.h"
#include <QTableView>

/* this class only shows how to use the dbc_item_model,
 * using a table description for the file "CreatureFamily.dbc" */

template <typename dbc_file_description, typename dbc_table_projection>
class simple_dbc_combobox
{
    QComboBox b;

    typedef dbc_file<dbc_file_description>                           file_type;
    typedef dbc_table<typename file_type::view,dbc_table_projection> table_type;
    typedef typename table_type::view                                table_view;
    file_type                       m_dbc_file;
    table_type                      m_dbc_table;
    table_view                      m_dbc_table_view;
    dbc_model_adaptor<table_view>   m_table_adaptor;
public:
    simple_dbc_combobox(const configuration & cfg) :
        b(),
        m_dbc_file(),
        m_dbc_table(),
        m_dbc_table_view(m_dbc_table()),
        m_table_adaptor(m_dbc_table_view)
    {
        dbc_directory dir(cfg);

        // Step 1: Load the file
        m_dbc_file.configure(dir);
        m_dbc_file.load();
        qDebug(m_dbc_file.error_msg().c_str());

        // Step 2: Load the table that is dependent on the file
        auto file_view = m_dbc_file();
        m_dbc_table.configure(file_view);
        m_dbc_table.load();
        qDebug(m_dbc_table.error_msg().c_str());

        // Step 3: We can now discard file data since all dependencies are initialized. And no other resource
        //          directly depends on file data anymore.
        m_dbc_file.discard();

        // Step 4: Setup the Qt item model, depending on the table
        dbc_item_model * item_model = new dbc_item_model(m_table_adaptor);

        // Step 5: Assign model to view
        b.setModel(item_model);
        b.setModelColumn(1);
    }
    ~simple_dbc_combobox(){}

    QComboBox * get() { return &b; }
};

#endif // SIMPLE_DBC_COMBOBOX_H
