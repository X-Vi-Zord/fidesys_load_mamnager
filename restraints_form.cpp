#include "restraints_form.h"
#include "ui_restraints_form.h"


#include <QJsonObject>
#include <QJsonArray>
#include <QDoubleValidator>

restraints_form::restraints_form(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::restraints_form)
{
    ui->setupUi(this);

    lanes = {ui->lineEdit,ui->lineEdit_2,ui->lineEdit_3,ui->lineEdit_4,ui->lineEdit_5,ui->lineEdit_6};
    Check = {ui->checkBox,ui->checkBox_2,ui->checkBox_3,ui->checkBox_4,ui->checkBox_5,ui->checkBox_6};
    for(const auto& line : lanes) {
        line->setValidator(new QDoubleValidator(this));
    }
}

restraints_form::~restraints_form()
{
    delete ui;
}
void restraints_form::set_form(QJsonObject *obj)
{
    object = obj;
    for (int i = 0; i < 6; i++){
        lanes[i]->setText(QString::number(obj->value("data")[i][0].toDouble()));
        Check[i]->setChecked(static_cast<bool>(obj->value("flag")[i].toInt()));
    }
}

void restraints_form::on_pushButton_clicked()
{
    QJsonArray flag, data;
    for(int i = 0; i < 6; i++){
        QJsonArray rowdata;
        rowdata.append(lanes[i]->text().toDouble());
        data.append(rowdata);
        flag.append(Check[i]->isChecked()?1:0);
    }
    object->insert("data", data);
    object->insert("flag", flag);
    emit new_res(object, 1);
}

