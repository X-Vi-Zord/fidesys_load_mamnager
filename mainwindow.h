#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QLineEdit>
#include <QCheckBox>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QLabel>
#include <QModelIndex>



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
public slots:

    void handleItemClicked(const QModelIndex &index);

private slots :

    void on_path_textEdited(const QString);

    void on_pushButton_clicked();

    void on_Save_clicked();

    bool on_Start_clicked();

private:

    Ui::MainWindow *ui;
    // можно обобщить до find.

    std::array<QLineEdit*, 6> _lines;
    std::array<QCheckBox*, 6> _flags;
    std::array<QLabel*, 6> _labels;

    QJsonObject _doc, _buffer;
    QStandardItemModel *_model;


    void find_conditions();
    void set_conditions(const QString, const int);
    void hide() const;
    void display() const;
    void take_Changes();
    void loadFile(const QString &fileName); //Открытие файла


};
#endif // MAINWINDOW_H
