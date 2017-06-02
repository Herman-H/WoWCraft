#-------------------------------------------------
#
# Project created by QtCreator 2017-05-23T16:27:14
#
#-------------------------------------------------

QT       += core gui
QT       += sql
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wowcraft
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    database/creature_template.cpp \
    database/page_text.cpp \
    database/test.cpp \
    dbc/dbc_files.cpp

HEADERS  += mainwindow.h \
    database/circularqueue.h \
    database/creature_template.h \
    database/dbinterface.h \
    database/dbtmp.h \
    database/page_text.h \
    database/table.h \
    database/test.h \
    dbc/dbc.h \
    dbc/dbc_files.h \
    dbc/dbc_projection.h \
    dbc/dbc_record.h \
    dbc/dbc_table.h \
    tmp/tmp.h \
    tmp/tmp_function.h \
    tmp/tmp_math.h \
    tmp/tmp_string.h \
    tmp/tmp_type_traits.h \
    tmp/tmp_types.h \
    config.h \
    widgets/dbc_item_model.h \
    widgets/simple_dbc_combobox.h \
    directory.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++14
