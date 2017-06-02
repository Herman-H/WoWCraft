#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "widgets/simple_dbc_combobox.h"
#include "config.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* A resource dependent widget is immediately loaded to a "unloaded state". In this state it can observe
     * it's underlying object loading progress or errors. If it is in progress, you can not interact with the
     * widget, you are only shown the progress information. If there is an error with loading, then you can
     * click on the widget to help resolve the error.
     */

    /* In addition to the widgets themselves being aware to their resources' progress, there is also a page that
     * tracks all loading information and errors.
     */

    /* The core of the loading progress is a directed acyclic graph which has a declared order in which
     * objects are loaded.
     */

    // A first look at the DBC components of this resource management system
    configuration cfg("config");
    m_creature_family = new creature_family_cb(cfg);
    m_creature_type = new creature_type_cb(cfg);
    ui->centralWidget->layout()->addWidget(m_creature_family->get());
    ui->centralWidget->layout()->addWidget(m_creature_type->get());
}

MainWindow::~MainWindow()
{
    delete ui;
}
