#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlError>

/*
 * Interface the underlying database library to the table model.
 *
 * What does the model require from the library?
 *
 *  - A connection where queries can be sent. [void query(const char * str)]
 *
 *  - A way to handle query results. A way to access elements of index and return it as specified type.
 *      These types can be chosen:
 *       - 32 bit int
 *       - 64 bit int
 *       - float
 *       - const char *
 *
 *  - Check to see last error
 *
 */


enum class DB_LIBRARY
{
    QT
};

enum class error
{
    index_out_of_range
};

template <DB_LIBRARY L>
class db_interface;

template <>
class db_interface<DB_LIBRARY::QT>
{
public:
    db_interface(QSqlDatabase & db) :
        m_db(db),
        m_rec(QSqlRecord{})
    {

    }
    ~db_interface(){}

    void query(const char * queryStr)
    {
        m_query = m_db.exec(QString{queryStr});
        previous_query_was_erroneous = m_query.lastError().isValid();
    }

    bool no_error_occured()
    {
        return !previous_query_was_erroneous;
    }

    bool next()
    {
        bool b = m_query.next();
        m_rec = m_query.record();
        return b;
    }

    const char * get_string_at(int index)
    {
        if(index >= m_rec.count())
        {
            last_error = error::index_out_of_range;
            return "";
        }
        return m_rec.value(index).toByteArray().data();
    }

    long long get_longdata_at(int index)
    {
        if(index >= m_rec.count())
        {
            last_error = error::index_out_of_range;
            return 0;
        }
        return m_rec.value(index).toLongLong();
    }

    int get_data_at(int index)
    {
        if(index >= m_rec.count())
        {
            last_error = error::index_out_of_range;
            return 0;
        }
        return m_rec.value(index).toInt();
    }

    float get_float_at(int index)
    {
        if(index >= m_rec.count())
        {
            last_error = error::index_out_of_range;
            return 0.0f;
        }
        return m_rec.value(index).toFloat();
    }

private:

    QSqlDatabase & m_db;
    QSqlQuery m_query;
    QSqlRecord m_rec;
    error last_error;
    bool previous_query_was_erroneous;
};

#endif // DBINTERFACE_H
