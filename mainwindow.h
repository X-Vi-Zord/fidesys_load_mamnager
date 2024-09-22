#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QSplitter>
#include <QJsonObject>
#include <QStackedWidget>


#include "control_panel.h"
#include "load_form_vector.h"
#include "load_form_no_vector.h"

#include "restraints_form.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void menu(QString, int);

    void set_form_r(QJsonObject*);

    void set_form_l_v(QJsonObject*);

    void set_form_l_nv(QJsonObject*);

public slots:

    bool start_calc();

    void take_info_about(QString, int);

    void take_Changes(QJsonObject*, int);

private:

    QStackedWidget *settings;
    control_panel *panel;
    load_form_vector *l_form;
    load_form_no_vector *lnv_form;
    restraints_form *r_form;

    QJsonObject F_Jn_Ob, buffer;

    Ui::MainWindow *ui;

    void open();

    void find_BC();
    void find_loads();

    double python_get_stress();

};
#endif // MAINWINDOW_H
