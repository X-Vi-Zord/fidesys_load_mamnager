#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class control_panel;
}

class control_panel : public QWidget
{
    Q_OBJECT
public:
    explicit control_panel(QWidget *parent = nullptr);
    ~control_panel();

signals:
    void start();                           // Сигнал для нажатия рассчёта

    void take_info_about(QString, int);     // Сигнал на запрос информации

public slots:
    void list_of_load(QString name, int value);

private slots:
    void on_pushButton_clicked();  // Слот для обработки нажатия кнопки

    void handleItemClicked(const QModelIndex &index);

private:
    Ui::control_panel *ui;
    QStandardItemModel *model;
};

#endif // CONTROL_PANEL_H
