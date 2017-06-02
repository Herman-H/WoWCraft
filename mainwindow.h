#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "widgets/simple_dbc_combobox.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    typedef simple_dbc_combobox<creature_family_dbc,creature_family_projection> creature_family_cb;
    typedef simple_dbc_combobox<creature_type_dbc,creature_type_projection>     creature_type_cb;
    creature_family_cb* m_creature_family;
    creature_type_cb* m_creature_type;
};

#endif // MAINWINDOW_H
