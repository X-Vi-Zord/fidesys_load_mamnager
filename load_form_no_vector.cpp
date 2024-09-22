#include "load_form_no_vector.h"
#include "ui_load_form_no_vector.h"


#include <QJsonObject>
#include <QJsonArray>
#include <QDoubleValidator>


load_form_no_vector::load_form_no_vector(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::load_form_no_vector)
{
    ui->setupUi(this);
    ui->lineEdit->setValidator(new QDoubleValidator(this));


}

load_form_no_vector::~load_form_no_vector()
{
    delete ui;
}

void load_form_no_vector::set_form(QJsonObject *obj)
{
    object = obj;
    ui->lineEdit->setText(QString::number(obj->value("data")[0][0].toDouble()));
}

void load_form_no_vector::on_pushButton_clicked(){
    QJsonArray  data,rowdata;
    rowdata.append(ui->lineEdit->text().toDouble());
    data.append(rowdata);
    object->insert("data", data);
    emit new_load(object, 2);

}
