#ifndef DBC_ITEM_MODEL_H
#define DBC_ITEM_MODEL_H

#include "dbc/dbc_table.h"
#include <QStandardItemModel>

struct model_adaptor_base
{
public:
    virtual QVariant data(const QModelIndex &, int) const = 0;
    virtual int rows() const = 0;
    virtual int columns() const = 0;
};

template <typename T>
struct wrap_value
{
    static QVariant value(T t){ return QVariant{t}; }
};
template <>
struct wrap_value<const char*>
{
    static QVariant value(const char* c_str){ return QVariant{QString{c_str}}; }
};

template <typename VIEW,unsigned int N>
struct set_nth_gets
{
    typedef typename VIEW::record_t record_t;
    typedef QVariant (*gets_t)(const record_t&);
    template <unsigned int M>
    using field_type = tmp::get_element_in<M,record_t>;
    static void set(gets_t* gets)
    {
        gets[N-1] = [](const record_t& r) -> QVariant
        {
            return wrap_value<field_type<N-1>>::value(std::get<N-1>(r));
        };
        set_nth_gets<VIEW,N-1>::set(gets);
    }
};
template <typename VIEW>
struct set_nth_gets<VIEW,0>
{
    typedef typename VIEW::record_t record_t;
    typedef QVariant (*gets_t)(const record_t&);
    static void set(gets_t*){}
};

template <typename VIEW>
struct dbc_model_adaptor : public model_adaptor_base
{
    const VIEW & m_view;

    typedef typename VIEW::record_t record_t;
    typedef QVariant (*gets_t)(const record_t&);
    typedef QVariant (*lambda_t)(const QModelIndex&, int);


    gets_t m_gets[tmp::cardinality<record_t>];

    dbc_model_adaptor(const VIEW & view) : m_view(view)
    {
        set_nth_gets<VIEW,tmp::cardinality<record_t>>::set(m_gets);
    }

    QVariant data(const QModelIndex& index,int) const
    {
        const record_t & r = m_view.record_at(index.row());
        return m_gets[index.column()](r);
    }

    int rows() const
    {
        return m_view.count();
    }

    int columns() const
    {
        return tmp::cardinality<record_t>;
    }
};


class dbc_item_model : public QAbstractItemModel
{
public:
    enum class field_type
    {
        STRING,
        INTEGER,
        FLOAT
    };

private:
    const model_adaptor_base & m_view;
public:
    dbc_item_model(const model_adaptor_base & view) :
        m_view(view)
    {
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        if(parent.isValid())
        {
            return 0;
        }
        else
        {
            return m_view.columns();
        }
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        if(parent.isValid())
        {
            return 0;
        }
        else
        {
            return m_view.rows();
        }
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
    {
        if(!hasIndex(row,column,parent))
        {
            return QModelIndex{};
        }
        if(parent.isValid())
            return QModelIndex{};

        return createIndex(row,column);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if(!index.isValid())
        {
            return QVariant{};
        }
        if(role == Qt::DisplayRole)
            return m_view.data(index,role);
        else
            return QVariant{};
    }

    QModelIndex parent(const QModelIndex &) const
    {
        return QModelIndex{};
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const
    {
        if(role == Qt::DisplayRole)
        {
            switch(section)
            {
            case 0:
                return QString{"ID"};
            case 1:
                return QString{"Name"};
            }
        }
        return QString{""};
    }
};

#endif // DBC_ITEM_MODEL_H
