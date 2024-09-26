#include "mainwindow.h"
#include "ui_mainwindow.h"

// Libs for Interface
#include <QFileDialog>
// Working with Files
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItem>
#include <QModelIndex>
#include <QProcess>
#include <QMessageBox>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _lines = {ui->x_le, ui->y_le, ui->z_le, ui->Mom_x_le, ui->Mom_y_le, ui->Mom_z_le};
    _labels = {ui->x_lbl, ui->y_lbl, ui->z_lbl, ui->Mom_x_lbl, ui->Mom_y_lbl, ui->Mom_z_lbl};
    _flags = {ui->x_ChBx, ui->y_ChBx, ui->z_ChBx, ui->Mom_x_ChBx, ui->Mom_y_ChBx, ui->Mom_z_CheBx};
    _model = new QStandardItemModel(ui->treeView);
    ui->treeView->setModel(_model);

    for(const auto& line: _lines)
    {line->setValidator(new QIntValidator);}
    ui->displ_nt_le->setValidator(new QIntValidator);
    ui->pres_le->setValidator(new QIntValidator);
    Hide();
    ui->Save_Bm->hide();
    ui->parameters->hide();

    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::TakeItemFromTree);
    connect(ui->Save_Bm, &QPushButton::clicked, this, &MainWindow::Save);
    connect(ui->Start_Bm, &QPushButton::clicked, this, &MainWindow::Start);
    connect(ui->find_file_Bm, &QPushButton::clicked, this, &MainWindow::FindFile);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::FindConditions()
{
    _read_file = {};
    if(!_doc.contains("restraints")) {
        qCritical() << "no restraints";
    }
    else{
        QJsonArray restraints;
        for (const auto restraint : _doc["restraints"].toArray()){
            QJsonObject sub_restraints;
            sub_restraints.insert("type", 0);
            sub_restraints.insert("name", "resrtraint");
            sub_restraints.insert("id", restraint.toObject()["id"].toInt());
            restraints.append(sub_restraints);
        }
    _read_file.insert("restraints", restraints);
    }
    if(_doc["loads"].isNull()){
        qCritical() << "no loads";
    }
    else{
    QJsonArray force, dis_force, pressure;
    for (const auto& load : _doc["loads"].toArray()){
        QJsonObject subload;
        subload.insert("type", load.toObject()["type"].toInt());
        subload.insert("name", load.toObject()["name"].toString());
        subload.insert("id", load.toObject()["id"].toInt());
        switch(load.toObject()["type"].toInt()){
        case 3: {pressure.append(subload);break;}
        case 5: {force.append(subload);break;}
        case 35: {dis_force.append(subload);break;}
        default: {statusBar()->showMessage("uncnoun load");}
        }
        _read_file.insert("force",force);
        _read_file.insert("pressure",pressure);
        _read_file.insert("distributed force",dis_force);
        }
    }
    NewTree();
}

void MainWindow::Hide() const
{
    for(int i = 0; i < 6; i++) {
        _lines[i]->hide();
        _flags[i]->hide();
        _labels[i]->hide();
    }
    ui->pres_le->hide();
    ui->pres_lbl->hide();
    ui->displ_nt_lbl->hide();
    ui->displ_nt_le->hide();
}

void MainWindow::Display() const
{
    if(_buffer.isEmpty()){
        return;
    }
    ui->Save_Bm->show(); ui->parameters->show();
    if(_buffer.contains("flag")) {
        ui->displ_nt_lbl->show();
        ui->displ_nt_le->show();
        ui->displ_nt_le->setText("0");
        for(int i = 0; i < 6 ;i++) {
            _flags[i]->show();
            _flags[i]->setChecked(_buffer.value("flag")[i].toInt());
            _labels[i]->show();
            if(_buffer["data"][0][i]!=0) {
                ui->displ_nt_le->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
            }
        }
    return;}
    if(_buffer["data"].toArray().size() > 1) {
        for(int i = 0; i < 6 ;i++){
        _lines[i]->show();
        _lines[i]->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
        _labels[i]->show();}
    }else{
        ui->pres_le->show();
        ui->pres_le->setText(QString::number(_buffer.value("data")[0][0].toDouble()));
        ui->pres_lbl->show();
    }
}

void MainWindow::TakeItemFromTree(const QModelIndex &index)
{
    if (!index.parent().isValid()) {
        return;}
    _item =  _model->itemFromIndex(index);
    if (!(_item->data(1).toInt() == 0)) {
        for(const auto& value : _doc["loads"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"].toInt()==_item->data(3).toInt()&&iterator["type"].toInt()==_item->data(1).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }else{
        for(const auto& value : _doc["restraints"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"]==_item->data(3).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }
    Hide();
    Display();
}

void MainWindow::TakeChanges()
{
    if(_item->data(1).toInt() == 0) {
        QJsonArray r_array =  _doc["restraints"].toArray();
        for( int i = 0; i < r_array.size();i++) {
            if (r_array[i].toObject()["id"]==_item->data(3).toInt()){
                r_array[i] = QJsonValue(_buffer);
                _doc.insert("restraints",r_array);
                statusBar()->showMessage("Changes for" + _item->data(2).toString() +  _item->data(3).toString() + " saved");
                return;}
        }
    }
    QJsonArray l_array = _doc["loads"].toArray();
    for( int i = 0; i < l_array.size();i++) {
        if (l_array[i].toObject()["id"]==_buffer["id"] && l_array[i].toObject()["name"] ==_buffer["name"]){
            l_array[i] = QJsonValue(QJsonObject(_buffer));
            _doc["loads"] = l_array;
            statusBar()->showMessage("Changes for" + _item->data(2).toString() +  _item->data(3).toString() + " saved");
            return;}
    }
}

void MainWindow::FindFile()
{
    _file_path = QFileDialog::getOpenFileName(this);
    if (_file_path.isEmpty()){
        statusBar()->showMessage("file is Empty");
        return;
    }
    if(_file_path.section(".",-1)!="fc"){
        statusBar()->showMessage("file not .fc");
        return;
    }
    QFile file(_file_path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, QString("%1").arg(_file_path),
                             QString("Can't open file %1\nError: %2.")
                                 .arg(_file_path)
                                 .arg(file.errorString()));
        return;
    }

    _doc = (QJsonDocument::fromJson(file.readAll())).object();
    _model->clear();
    FindConditions();
    Hide();
    ui->Save_Bm->hide();
    ui->parameters->hide();
    statusBar()->showMessage("file " + _file_path.section("/",-1) +  " is open");
    ui->filename_lbl->setText(_file_path.section("/",-1));
}

void MainWindow::Save()
{
    QJsonArray data, flag, pres, subpres, displace;
    for(int i = 0; i < 6; i++){
        QJsonArray subdata, subdisplace;
        subdisplace.append(0.0);
        displace.append(subdisplace);
        subdata.append(_lines[i]->text().toDouble());
        data.append(subdata);
        flag.append(_flags[i]->isChecked()?1:0);
    }
    QJsonArray subdis;
    subdis.append(ui->displ_nt_le->text().toDouble());
    displace.pop_back();
    displace.append(subdis);
    subpres.append(ui->pres_le->text().toDouble());
    pres.append(subpres);
    switch (_item->data(1).toInt()) {
    case 0:     {_buffer.insert("flag", flag);_buffer.insert("data", displace);break;}
    case 3:     {_buffer.insert("data", pres);break;}
    default:    {_buffer.insert("data", data);break;}
    }
    TakeChanges();
}

bool MainWindow::Start()
{
    statusBar()->showMessage("Calculation started");
    QApplication::processEvents();
    QFile tmp_fc("tmp.fc");
    if (!tmp_fc.open(QIODevice::WriteOnly))
    {
        qCritical() << "Can't write new .fc file";
        statusBar()->showMessage("Can't write new .fc file");
        return false;
    }
    tmp_fc.write(QJsonDocument(_doc).toJson(QJsonDocument::Indented));
    tmp_fc.close();

    QString path_to_Calc = "D:/CAE-Fidesys-7.0/bin/FidesysCalc.exe";

    if (!QFile(path_to_Calc).exists())
    {
        qCritical() << "FidesysCalc not found!";
        statusBar()->showMessage("FidesysCalc not found!");

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
    statusBar()->showMessage("Error");
    statusBar()->showMessage("Calc output" + QString(calc_output["output"]),2000);
    qInfo() << "Errors:" << QString(calc_output["errors"]);
    qInfo() << "Calc output" << QString(calc_output["output"]);
    if (QString(calc_output["errors"]) != "")
    {
        return false;
    }
    qInfo() << "!!DONE!!";
    statusBar()->showMessage("Calculation is DONE");
    return true;
}

void MainWindow::NewTree(){
    _model->setHorizontalHeaderItem(0, new QStandardItem("data"));
    _model->setHorizontalHeaderItem(1, new QStandardItem("id"));
    for(const auto& imap: _loads){
        if(_read_file.contains(imap.second))
        {
            QStandardItem *item = new QStandardItem(imap.second);
            for(const auto& value : _read_file[imap.second].toArray())
            {
                QStandardItem *sub = new QStandardItem();
                sub->setData(QString::number(value.toObject()["type"].toInt()),1);
                sub->setData(value.toObject()["name"].toString(),2);
                sub->setData(QString::number(value.toObject()["id"].toInt()),3);
                const QString name = value.toObject()["name"].toString();
                sub->setText(name);
                item->setChild(value.toObject()["id"].toInt()-1,sub);
                item->setChild(value.toObject()["id"].toInt()-1,1,new QStandardItem(QString::number(value.toObject()["id"].toInt())));
                sub->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
            _model->appendRow(item);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }
}
