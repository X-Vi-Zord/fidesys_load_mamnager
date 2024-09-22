#include "mainwindow.h"
#include "ui_mainwindow.h"

// Для интерфейса и отладки
#include <QFileDialog>
#include <QDebug>
// Для работы с файлами
#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHBoxLayout>

// Для запуска процесса Fidesys
#include <QProcess>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
// заполнение MainWindow активными виджетами
    QSplitter *splitter = new QSplitter(this);
    panel = new control_panel(this);
    settings = new QStackedWidget(this);
    l_form = new load_form_vector(this);
    r_form = new restraints_form(this);
    lnv_form = new load_form_no_vector(this);
    // Добавляем виджеты в QStackedWidget
    settings->addWidget(l_form);
    settings->addWidget(lnv_form);
    settings->addWidget(r_form);
    settings->hide();
    settings->setMinimumSize(480, 320);
    splitter->addWidget(panel);
    splitter->addWidget(settings);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    // Устанавливаем сплиттер как центральный виджет
    setCentralWidget(splitter);
    // соединяем виджеты между собой
    // для передачи информации на панель управления
    connect(this,&MainWindow::menu, panel,&control_panel::list_of_load);
    // для получения информации с панели управления
    connect(panel,&control_panel::take_info_about,this,&MainWindow::take_info_about);
    // для отображения окна редактирования ГУ и ВСФ (внеш. сил. факторы)
    connect(this,&MainWindow::set_form_r,r_form,&restraints_form::set_form);
    connect(this,&MainWindow::set_form_l_v,l_form,&load_form_vector::set_form);
    connect(this,&MainWindow::set_form_l_nv,lnv_form,&load_form_no_vector::set_form);
    // для сохранения новых ГУ и ВСФ
    connect(r_form,&restraints_form::new_res,this,&MainWindow::take_Changes);
    connect(l_form,&load_form_vector::new_load,this,&MainWindow::take_Changes);
    connect(lnv_form,&load_form_no_vector::new_load,this,&MainWindow::take_Changes);
    // кнопка запуска рассчёта
    connect(panel,&control_panel::start, this, &MainWindow::start_calc);
    open();
};

void MainWindow::open()
{
    QFile   file("3D.fc");
    if (!file.open(QIODevice::ReadOnly)){
        qCritical() << "Can't open 'fc'";
        return;}
    QJsonDocument file_json(QJsonDocument::fromJson(file.readAll()));
    F_Jn_Ob = file_json.object();
    find_BC();
    find_loads();
    file.close();
}

void MainWindow::find_BC()
{
    if(!F_Jn_Ob.contains("restraints")) {
        qCritical() << " нет ограничений в постановке задачи";
        return;
    }
    emit menu("restraints", F_Jn_Ob["restraints"].toArray().size());
}

void MainWindow::find_loads()
{
    if(F_Jn_Ob["loads"].isNull()){
        qCritical() << "нет обрабатываемых усилий в постановке задачи";
        return;
    }
    int i = 0;
    QString key = "";
    for (const auto& load : F_Jn_Ob["loads"].toArray()){
        if (load.toObject()["name"] != key) {
            // тут отправляем кол-во элементов
            if(i!=0) {
                emit menu(key,i);
            }
            key = load.toObject()["name"].toString();
            i=0;
        }
    i++;}
    emit menu(key,i);
}

void MainWindow::take_info_about(QString const name, int const id)
{
    settings->show();
    if(name == "restraints")
    {
        buffer = F_Jn_Ob["restraints"][id].toObject();
        settings->setCurrentWidget(r_form);
        emit set_form_r(&buffer);
        return;}
    for (const auto& value : F_Jn_Ob["loads"].toArray()){
        buffer = value.toObject();
        if(buffer["name"]== name && buffer["id"] == id+1){
            if(buffer["data"].toArray().size() > 2){
                emit set_form_l_v(&buffer);
                settings->setCurrentWidget(l_form);
            } else{
                emit set_form_l_nv(&buffer);
                settings->setCurrentWidget(lnv_form);}}}
    update();
}

void MainWindow::take_Changes(QJsonObject *new_ob, int const id)
{
    switch(id){
    case 1 : {
        QJsonArray r_array =  F_Jn_Ob["restraints"].toArray();
        for( int i = 0; i < r_array.size();i++) {
            if (r_array[i].toObject()["id"]==new_ob->value("id")){
                r_array[i] = QJsonValue(*new_ob);
                F_Jn_Ob["restraints"]=r_array;
                return;}
        }
    }
    case 2: {
        QJsonArray l_array = F_Jn_Ob["loads"].toArray();
        for( int i = 0; i < l_array.size();i++) {
            if (l_array[i].toObject()["id"]==new_ob->value("id") && l_array[i].toObject()["name"] ==new_ob->value("name")){
                l_array[i] = QJsonValue(*new_ob);
                F_Jn_Ob["loads"] = l_array;
                return;}
        }
    }
    default: {return;}
    }
}

double MainWindow::python_get_stress()
{
    QProcess *process = new QProcess();
    QStringList process_args;
    process_args << "vtk_stress.py";
    process->setProgram("python");
    process->setArguments(process_args);
    process->start();
    process->waitForStarted(-1);
    qInfo() << "Python started";
    process->waitForFinished(-1);
    qInfo() << "Done";

    QMap<QString, QByteArray> python_output;
    python_output["errors"] = process->readAllStandardError();
    python_output["output"] = process->readAllStandardOutput();
    qInfo() << "Python script results:";
    qInfo() << "Errors:" << QString(python_output["errors"]);
    qInfo() << "Output" << QString(python_output["output"]);

    if (python_output["errors"].size())
        return -1;
    return QString(python_output["output"]).toDouble();
}

bool MainWindow::start_calc()
{
    QFile tmp_fc("tmp.fc");
    if (!tmp_fc.open(QIODevice::WriteOnly))
    {
        qCritical() << "Can't write new .fc file";
        return false;
    }
    tmp_fc.write(QJsonDocument(F_Jn_Ob).toJson(QJsonDocument::Indented));
    tmp_fc.close();

    QString path_to_Calc = "D:/CAE-Fidesys-7.0";

    if (QSysInfo::kernelType() != "linux")
    {
        path_to_Calc += "/bin/FidesysCalc.exe";
    }
    else
        path_to_Calc += "/calc/bin/FidesysCalc";

    qInfo() << "Path to FidesysCalc:" << path_to_Calc;
    if (!QFile(path_to_Calc).exists())
    {
        qCritical() << "FidesysCalc not found!";
        return false;
    }

    if (!QDir(QDir::currentPath().append("results")).exists())
    {
        QDir().mkdir("results");
    }

    QMap<QString, QByteArray> calc_output;

    QProcess *process = new QProcess();
    QStringList calc_args;
    calc_args << "--input=" + QDir::currentPath() + "/tmp.fc"
              << "--output=" + QDir::currentPath() + "/results/tmp.pvd";
    qInfo() << "Starting process" << path_to_Calc << calc_args[0] << calc_args[1];

    process->setProgram(path_to_Calc);
    process->setArguments(calc_args);
    process->start();
    process->waitForStarted(-1);
    qInfo() << "Calculation started";
    process->waitForFinished(-1);

    calc_output["errors"] = process->readAllStandardError();
    calc_output["output"] = process->readAllStandardOutput();
    qInfo() << "Errors:" << QString(calc_output["errors"]);
    qInfo() << "Calc output" << QString(calc_output["output"]);
    if (QString(calc_output["errors"]) != "")
    {
        return false;
    }

    qInfo() << "!!DONE!!";
    return true;
}

MainWindow::~MainWindow()
{
    delete ui;
}
