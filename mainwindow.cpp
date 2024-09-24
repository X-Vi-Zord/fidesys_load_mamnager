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



// for setup
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _lines = {ui->arg_1, ui->arg_2, ui->arg_3, ui->arg_4, ui->arg_5, ui->arg_6};
    _labels = {ui->com_1, ui->com_2, ui->com_3, ui->com_4, ui->com_5, ui->com_6};
    _flags = {ui->fl_1, ui->fl_2, ui->fl_3, ui->fl_4, ui->fl_5, ui->fl_6};
    _model = new QStandardItemModel(ui->treeView);
    hide();
    ui->treeView->header()->hide();
    ui->Save->hide(); ui->parameters->hide();
    this->setMinimumSize(680,640);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleItemClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::find_conditions()
{
    if(!_doc.contains("restraints")) {
        qCritical() << "no restraints";
    }
    else{
        set_conditions("restraints",_doc["restraints"].toArray().size());
    }
    if(_doc["loads"].isNull()){
        qCritical() << "no loads";
        return;
    }
    int i = 0;
    QString key = "";
    for (const auto& load : _doc["loads"].toArray()){
        if (load.toObject()["name"] != key) {
            // тут отправляем кол-во элементов
            if(i!=0) {
                set_conditions(key.toLower(), i);
            }
            key = load.toObject()["name"].toString();
            i=0;
        }
        i++;}
    set_conditions(key.toLower(), i);
}

void MainWindow::set_conditions(const QString name, const int value)
{
    QStandardItem *item1 = new QStandardItem(name);
    for(int i = 0; i<value ;i++) {
        QStandardItem *subitem = new QStandardItem(name +"_" + QString("%1").arg(i+1));
        subitem->setFlags(item1->flags() & ~Qt::ItemIsEditable);
        item1->appendRow(subitem);
    }
    _model->appendRow(item1);
    item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
    ui->treeView->setModel(_model);
    update();
}

void MainWindow::hide() const
{
    for(int i = 0; i < 6; i++) {
        _lines[i]->hide();
        _flags[i]->hide();
        _labels[i]->hide();
    }
    ui->arg_pressure->hide(); ui->com_7->hide();

}

void MainWindow::display() const
{
    if(_buffer.isEmpty()){
        return;
    }
    ui->Save->show(); ui->parameters->show();
    if(_buffer.contains("flag")) {
        for(int i = 0; i < 6 ;i++) {
            _lines[i]->show();
            _flags[i]->show();
            _lines[i]->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
            _flags[i]->setChecked(static_cast<bool>(_buffer.value("flag")[i].toInt()));
            _labels[i]->show();
        }
    return;}
    if(_buffer["data"].toArray().size() > 1) {
        for(int i = 0; i < 6 ;i++){
        _lines[i]->show();
        _lines[i]->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
        _labels[i]->show();}
    }else{
        ui->arg_pressure->show();
        ui->arg_pressure->setText(QString::number(_buffer.value("data")[0][0].toDouble()));
        ui->com_7->show();
    }
}

void MainWindow::handleItemClicked(const QModelIndex &index)
{
    const int id = _model->itemFromIndex(index)->text().section("_",-1).toInt()-1;
    qDebug() << id;
    if (!index.parent().isValid()) {
        return;}
    const QString parent = _model->itemFromIndex(index.parent())->text();
    if(parent.toLower() == "restraints")
    {
        _buffer = _doc["restraints"][id].toObject();
    }
    else {
        qDebug() << _doc["loads"];
        for (const auto& value : _doc["loads"].toArray()){
            _buffer = value.toObject();
            qDebug() << _buffer["id"];
            if(_buffer["name"].toString().toLower()== parent && _buffer["id"] == id+1){
                break;}}}
    hide();
    display();
}

void MainWindow::on_Save_clicked()
{
    QJsonArray data, flag;
        for(int i = 0; i < 6; i++){
            QJsonArray rowdata;
            rowdata.append(_lines[i]->text().toDouble());
            data.append(rowdata);
            flag.append(_flags[i]->isChecked()?1:0);
        }
    QJsonArray pres, rowpres;
    rowpres.append(ui->arg_pressure->text().toDouble());
    pres.append(rowpres);
    if (_buffer.contains("flag")) {
        _buffer.insert("flag", flag);
        _buffer.insert("data", data);
    }
    else if(_buffer["data"].toArray().size() > 2) {
        _buffer.insert("data", data);
    }
    else {
        _buffer.insert("data", pres);
    }
    take_Changes();
}


void MainWindow::take_Changes()
{
    if(!_flags[0]->isHidden()) {
        QJsonArray r_array =  _doc["restraints"].toArray();
        for( int i = 0; i < r_array.size();i++) {
            if (r_array[i].toObject()["id"]==_buffer["id"]){
                r_array[i] = QJsonValue(QJsonObject(_buffer));
                _doc["restraints"]=r_array;
                statusBar()->showMessage("Changes from restraints_" +  QString::number(_buffer["id"].toInt()) + " saved", 3000);

                return;}
        }
    }
    QJsonArray l_array = _doc["loads"].toArray();
    for( int i = 0; i < l_array.size();i++) {
        if (l_array[i].toObject()["id"]==_buffer["id"] && l_array[i].toObject()["name"] ==_buffer["name"]){
            l_array[i] = QJsonValue(QJsonObject(_buffer));
            _doc["loads"] = l_array;
            statusBar()->showMessage("Changes from " + _buffer["name"].toString().toLower() +"_"+ QString::number(_buffer["id"].toInt()) + " saved", 3000);
            return;}
    }
}

bool MainWindow::on_Start_clicked()
{

    statusBar()->showMessage("Calculation started");
    QApplication::processEvents();
    QFile tmp_fc("tmp.fc");
    if (!tmp_fc.open(QIODevice::WriteOnly))
    {
        qCritical() << "Can't write new .fc file";
        statusBar()->showMessage("Can't write new .fc file", 3000);
        return false;
    }
    tmp_fc.write(QJsonDocument(_doc).toJson(QJsonDocument::Indented));
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

void MainWindow::on_pushButton_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.isEmpty()){
        statusBar()->showMessage("file is Empty");
            return;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("%1").arg(fileName),
                             tr("Can't open file %1\nError: %2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
        return;
    }
    QJsonDocument file_json(QJsonDocument::fromJson(file.readAll()));
    _doc = file_json.object();
    _model->clear();
    _model->setObjectName(fileName.section("/",-1));
    find_conditions();
    hide();
    ui->Save->hide(); ui->parameters->hide();
    statusBar()->showMessage("file " + fileName.section("/",-1) +  " is open");
}
