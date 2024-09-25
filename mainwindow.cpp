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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _lines = {ui->x_le, ui->y_le, ui->z_le, ui->Mom_x_le, ui->Mom_y_le, ui->Mom_z_le};
    _labels = {ui->x_lbl, ui->y_lbl, ui->z_lbl, ui->Mom_x_lbl, ui->Mom_y_lbl, ui->Mom_z_lbl};
    _flags = {ui->x_ChBx, ui->y_ChBx, ui->z_ChBx, ui->Mom_x_ChBx, ui->Mom_y_ChBx, ui->Mom_z_CheBx};
    _model = new QStandardItemModel(ui->treeView);
    createTree();
    ui->treeView->setModel(_model);
    hide();
    ui->treeView->header()->hide();
    ui->Save_Bm->hide(); ui->parameters->hide();
    this->setMinimumSize(680,640);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::takeItemFromTree);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::findConditions()
{
    if(!_doc.contains("restraints")) {
        qCritical() << "no restraints";
    }
    else{
        for (const auto restraint : _doc["restraints"].toArray()){
            setConditions(restraint.toObject()["id"].toInt());
        }
    }
    if(_doc["loads"].isNull()){
        qCritical() << "no loads";
        return;
    }
    for (const auto& load : _doc["loads"].toArray()){
        setConditions(load.toObject()["id"].toInt(),load.toObject()["name"].toString().toLower(), load.toObject()["type"].toInt());
    }
}

void MainWindow::setConditions(const unsigned id, const QString name, const unsigned type)
{
    // type of load by name
    for(int row = 0; row < _model->rowCount();row++){
        if(_model->item(row)->text() == _loads[type]){
            QStandardItem *subitem = new QStandardItem();
            subitem->setData(type,1);
            subitem->setData(name,2);
            subitem->setData(id,3);
            subitem->setFlags(_model->item(row)->flags() & ~Qt::ItemIsEditable);
            _model->item(row)->appendRow(subitem);
            return;
        }
    }
}

void MainWindow::hide() const
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

void MainWindow::display() const
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

void MainWindow::takeItemFromTree(const QModelIndex &index)
{
    if (!index.parent().isValid()) {
        return;}
    item =  _model->itemFromIndex(index);
    if (!(item->data(1).toInt() == 0)) {
        for(const auto& value : _doc["loads"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"].toInt()==item->data(3).toInt()&&iterator["type"].toInt()==item->data(1).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }else{
        for(const auto& value : _doc["restraints"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"]==item->data(3).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }
    hide();
    display();
}

void MainWindow::TakeChanges()
{
    if(item->data(1).toInt() == 0) {
        QJsonArray r_array =  _doc["restraints"].toArray();
        for( int i = 0; i < r_array.size();i++) {
            if (r_array[i].toObject()["id"]==item->data(3).toInt()){
                r_array[i] = QJsonValue(_buffer);
                _doc.insert("restraints",r_array);
                statusBar()->showMessage("Changes for" + item->data(2).toString() +  item->data(3).toString() + " saved");
                return;}
        }
    }
    QJsonArray l_array = _doc["loads"].toArray();
    for( int i = 0; i < l_array.size();i++) {
        if (l_array[i].toObject()["id"]==_buffer["id"] && l_array[i].toObject()["name"] ==_buffer["name"]){
            l_array[i] = QJsonValue(QJsonObject(_buffer));
            _doc["loads"] = l_array;
            statusBar()->showMessage("Changes for" + item->data(2).toString() +  item->data(3).toString() + " saved");
            return;}
    }
}

void MainWindow::createTree()
{
    for(auto const& imap: _loads) {
        QStandardItem *item = new QStandardItem(imap.second);
        _model->appendRow(item);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

void MainWindow::on_find_file_Bm_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this);
    if (filePath.isEmpty()){
        statusBar()->showMessage("file is Empty");
        return;
    }
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("%1").arg(filePath),
                             tr("Can't open file %1\nError: %2.")
                                 .arg(filePath)
                                 .arg(file.errorString()));
        return;
    }

    _doc = (QJsonDocument::fromJson(file.readAll())).object();
    _model->clear();
    createTree();
    findConditions();
    hide();
    ui->Save_Bm->hide();
    ui->parameters->hide();
    statusBar()->showMessage("file " + filePath.section("/",-1) +  " is open");
    ui->filename_lbl->setText(filePath.section("/",-1));
}

void MainWindow::on_Save_Bm_clicked()
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
    switch (item->data(1).toInt()) {
    case 0:     {_buffer.insert("flag", flag);_buffer.insert("data", displace);break;}
    case 3:     {_buffer.insert("data", pres);break;}
    default:    {_buffer.insert("data", data);break;}
    }
    TakeChanges();
}

bool MainWindow::on_Start_Bm_clicked()
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
