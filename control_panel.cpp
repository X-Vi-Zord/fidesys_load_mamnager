#include "control_panel.h"
#include "ui_control_panel.h"

#include <QStandardItem>
#include <QListView>

control_panel::control_panel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::control_panel)
{
    ui->setupUi(this);
    model = new QStandardItemModel(ui->treeView);
    connect(ui->treeView, &QTreeView::clicked, this, &control_panel::handleItemClicked);
}


control_panel::~control_panel()
{
    delete ui;
}

void control_panel::list_of_load(QString name, int value)
{
    QStandardItem *item1 = new QStandardItem(name);
    for(int i = 0; i<value ;i++) {
        QStandardItem *subitem = new QStandardItem(QString("%1").arg(i+1));
        item1->appendRow(subitem);
    }
    model->appendRow(item1);
    // Устанавливаем модель в QTreeView
    ui->treeView->setModel(model);
    update();
}

void control_panel::on_pushButton_clicked()
{
    emit start();  // Испускаем сигнал при нажатии на кнопку
}

void control_panel::handleItemClicked(const QModelIndex &index)
{
    // Получаем текст выбранного элемента
    int id = model->itemFromIndex(index)->text().toInt();
    // Проверяем, является ли это подэлементом (subitem)
    if (!index.parent().isValid()) {
        return;}
    QString parent = model->itemFromIndex(index.parent())->text();
    emit take_info_about(parent, id-1);
}
