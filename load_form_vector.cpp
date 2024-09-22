#include "load_form_vector.h"
#include "ui_load_form_vector.h"


#include <QJsonObject>
#include <QJsonArray>
#include <QDoubleValidator>

load_form_vector::load_form_vector(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::load_form_vector)
{
    ui->setupUi(this);

    lanes = {ui->lineEdit,ui->lineEdit_2,ui->lineEdit_3,ui->lineEdit_4,ui->lineEdit_5,ui->lineEdit_6};
    for(const auto& line : lanes) {
        line->setValidator(new QDoubleValidator(this));
    }
}

load_form_vector::~load_form_vector()
{
    delete ui;
}
void load_form_vector::set_form(QJsonObject *obj)
{
    object = obj;
    for (int i = 0; i < 6; i++){
        lanes[i]->setText(QString::number(obj->value("data")[i][0].toDouble()));
    }
}

void load_form_vector::on_pushButton_clicked()
{
    QJsonArray  data;
    for(int i = 0; i < 6; i++){
        QJsonArray rowdata;
        rowdata.append(lanes[i]->text().toDouble());
        data.append(rowdata);
    }
    object->insert("data", data);
    emit new_load(object, 2);

}

