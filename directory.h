#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QDir>
#include <QFileDialog>
#include <QApplication>
#include "config.h"

class directory
{
private:
    QDir m_dir;
public:
    directory(QString path) :
        m_dir(path)
    {
    }
    ~directory(){}

    void configure(QString path){ m_dir.setPath(path); }

    QString path() const
    {
        return m_dir.absolutePath();
    }

    bool add_file(const QString & file_name) const
    {
        QString file = QFileDialog::getOpenFileName(QApplication::activeWindow(),
                                                  QString{"Open "} + file_name,
                                                  m_dir.absolutePath(),
                                                  QString{file_name});
        if(!file.isNull())
        {
            return QFile::copy(file, m_dir.absolutePath() + file_name);
        }
        else
            return false;
    }
    bool exists(const QString & file_name) const { return m_dir.exists(file_name); }
};

class dbc_directory
{
private:
    directory m_directory;
public:
    dbc_directory(const configuration & cfg) :
        m_directory(cfg.get_string("DBC.Directory"))
    {

    }
    ~dbc_directory(){}

    void configure(const configuration & cfg)
    {
        m_directory.configure(cfg.get_string("DBC.Directory"));
    }


    QString path() const
    {
        return m_directory.path();
    }

    bool exists(const QString & file_name) const { return m_directory.exists(file_name); }

    bool add_file(const QString & file_name) const
    {
        return m_directory.add_file(file_name);
    }
};

#endif // DIRECTORY_H
