#ifndef CONFIG_H
#define CONFIG_H

#include <set>
#include <QHostAddress>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QRegularExpression>
#include <QDir>

class configuration
{
    enum class field_type
    {
        string,
        ip,
        number,
        real,
        dbms
    };

    const char m_delimit_begin = '{';
    const char m_delimit_end = '}';

    const QString file_path;

    struct cfg_data
    {
        bool                initialized;
        QString             value;
        field_type          type;
    };

    std::map<QString,cfg_data>  m_data;

    QString                     m_rx_string;

    QString default_value(const QString & id) const
    {
        if      (id.compare("DB.Host") == 0)
        { return "127.0.0.1"; }
        else if (id.compare("DB.Port") == 0)
        { return "3306"; }
        else if (id.compare("DB.Username") == 0)
        { return "user"; }
        else if (id.compare("DB.Password") == 0)
        { return "password"; }
        else if (id.compare("DB.World") == 0)
        { return "mangos"; }
        else if (id.compare("DB.Realm") == 0)
        { return "realm"; }
        else if (id.compare("DB.Characters") == 0)
        { return "characters"; }
        else if (id.compare("DB.DBMS") == 0)
        { return "MYSQL"; }
        else if (id.compare("DBC.Directory") == 0)
        { return QDir::currentPath() + QString{"/dbc/"}; }
        else if (id.compare("Session.Directory") == 0)
        { return QDir::currentPath() + QString{"/session/"}; }
        else if (id.compare("Session.Previous") == 0)
        { return ""; }
        else if (id.compare("Session.File.Prepend") == 0)
        { return ""; }
        else if (id.compare("Session.File.Append") == 0)
        { return ""; }
        return "";
    }

    bool read(QString & s)
    {
        // ([a-zA-Z.]+)[^{]+{([^}]+)
        QRegularExpression regexp{m_rx_string};
        auto it = regexp.globalMatch(s,0,QRegularExpression::NormalMatch);
        unsigned int n = 0;
        while(it.hasNext())
        {
            auto match = it.next();
            if(match.capturedTexts().size() < 2)
            {
            }
            QString id = match.captured(1);
            QString value = match.captured(2);
            cfg_data d = m_data[id];
            d.initialized = validate(id,value);
            d.value = value;
            m_data[id] = d;
            ++n;
        }
        if(n != m_data.size())
        {
            return false;
        }
        return true;
    }

    void initialize()
    {
        m_rx_string = QString{QString{"([a-zA-Z.]+)[^\\"} +
                  QString{m_delimit_begin} +
                  QString{"]+\\"} +
                  QString{m_delimit_begin} +
                  QString{"($|[^\\"} +
                  QString{m_delimit_end} +
                  QString{"]*)"}};

        /* init m_data */
        cfg_data data;

        m_data.clear();
        data.initialized = false;
        data.value = "";
        data.type = field_type::ip;
        m_data["DB.Host"] = data;
        data.type = field_type::number;
        m_data["DB.Port"] = data;
        data.type = field_type::string;
        m_data["DB.Username"] = data;
        m_data["DB.Password"] = data;
        m_data["DB.World"] = data;
        m_data["DB.Realm"] = data;
        m_data["DB.Characters"] = data;
        data.type = field_type::dbms;
        m_data["DB.DBMS"] = data;
        data.type = field_type::string;
        m_data["DBC.Directory"] = data;
        m_data["Session.Directory"] = data;
        m_data["Session.Previous"] = data;
        m_data["Session.File.Prepend"] = data;
        m_data["Session.File.Append"] = data;
    }

    void set_uninitialized_to_default()
    {
        auto it = m_data.begin();
        while(it != m_data.end())
        {
            if(!(*it).second.initialized)
            {
                QString k = (*it).first;
                cfg_data d = (*it).second;
                d.value = default_value((*it).first);
                d.initialized = true;
                m_data[k] = d;
            }
            ++it;
        }
    }

    void save()
    {
        QFile file(file_path);
        bool file_open = false;
        if((file_open = file.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)))
        {
            QTextStream s{&file};
            s.flush();
            auto it = m_data.begin();
            while(it != m_data.end())
            {
                QString id = (*it).first;
                QString value = (*it).second.value;
                s << id << " = " << m_delimit_begin << value << m_delimit_end << "\n";
                ++it;
            }
        }
        if(file_open)
        {
            file.close();
            file_open = false;
        }
        // For all directories pointed to by config, check that they exist
        QDir dbc_dir{m_data["DBC.Directory"].value};
        if(!dbc_dir.exists())
            dbc_dir.mkpath(m_data["DBC.Directory"].value);
        QDir session_dir{m_data["Session.Directory"].value};
        if(!session_dir.exists())
            session_dir.mkpath(m_data["Session.Directory"].value);
    }

    bool validate(const QString value, field_type type)
    {
        bool ok = true;
        switch(type)
        {
        case field_type::string:
            return true;
        case field_type::dbms:
        {
            return value.compare("MYSQL") == 0;
        }
        case field_type::ip:
        {
            QHostAddress dummy;
            return dummy.setAddress(value);
        }
        case field_type::real:
        {
            value.toFloat(&ok);
            return ok;
        }
        case field_type::number:
        {
            value.toInt(&ok);
            return ok;
        }
        }
        return false;
    }

    bool validate(const QString id, const QString value)
    {
        cfg_data d = m_data[id];
        return validate(value,d.type);
    }

    cfg_data get_data(const QString & id) const
    {
        cfg_data d;

        auto it = m_data.find(id);
        if(it != m_data.end())
            d = (*it).second;

        return d;
    }

public:
    configuration(const QString file_path) : file_path(file_path), m_data()
    {
        initialize();
        QFile file(file_path);
        bool file_open = false;
        bool incomplete_cover = true;
        if(file.exists())
        {
            if((file_open = file.open(QIODevice::ReadOnly | QIODevice::Text)))
            {
                QTextStream s{&file};
                QString input = s.readAll();
                incomplete_cover = !read(input);
            }
            if(file_open)
            {
                file.close();
                file_open = false;
            }
        }
        if(incomplete_cover)
        {
            set_uninitialized_to_default();
            save();
        }
    }
    ~configuration(){}

    char begin_value_delimiter_character() const
    {
        return m_delimit_begin;
    }
    char end_value_delimiter_character() const
    {
        return m_delimit_end;
    }

    QString get_string(const QString id) const
    {
        const cfg_data d = get_data(id);
        return d.value;
    }
    QHostAddress get_ip(const QString id) const
    {
        const cfg_data d = get_data(id);
        return QHostAddress{d.value};
    }
    float get_float(const QString id) const
    {
        const cfg_data d = get_data(id);
        return d.value.toFloat();
    }
    int get_number(const QString id) const
    {
        const cfg_data d = get_data(id);
        return d.value.toInt();
    }
    QString get_dbms(const QString id) const
    {
        const cfg_data d = get_data(id);
        if(d.value.compare("MYSQL") == 0)
        {
            return "QMYSQL";
        }
        return "";
    }

    bool set_string(const QString id, const QString value)
    {
        cfg_data d = m_data[id];
        if(d.type == field_type::string)
        {
            d.value = value;
            m_data[id] = d;
            save();
            return validate(id,d.value);
        }
        return false;
    }
    bool set_ip(const QString id, const QHostAddress value)
    {
        cfg_data d = m_data[id];
        if(d.type == field_type::ip)
        {
            d.value = value.toString();
            m_data[id] = d;
            save();
            return validate(id,d.value);
        }
        return false;
    }
    bool set_dbms(const QString id, const QString value)
    {
        cfg_data d = m_data[id];
        if(d.type == field_type::dbms)
        {
            if(value.compare("QMYSQL") == 0)
                d.value = "MYSQL";

            save();
            m_data[id] = d;
            return validate(id,value);
        }
        return false;
    }
    bool set_float(const QString id, const float value)
    {
        cfg_data d = m_data[id];
        if(d.type == field_type::real)
        {
            d.value = QString::number(value);
            save();
            m_data[id] = d;
            return validate(id,d.value);
        }
        return false;
    }
    bool set_number(const QString id, const int value)
    {
        cfg_data d = m_data[id];
        if(d.type == field_type::number)
        {
            d.value = QString::number(value);
            save();
            m_data[id] = d;
            return validate(id,d.value);
        }
        return false;
    }
};

#endif // CONFIG_H
