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

    void takeItemFromTree(const QModelIndex &index);

private slots :

    void on_find_file_Bm_clicked();

    void on_Save_Bm_clicked();

    bool on_Start_Bm_clicked();

private:

    Ui::MainWindow *ui;
    // можно обобщить до find.

    std::array<QLineEdit*, 6> _lines;
    std::array<QCheckBox*, 6> _flags;
    std::array<QLabel*, 6> _labels;
    std::map<unsigned, QString> _loads = {{3,"pressure" },{5,"force"},{35,"distributed force"}, {0,"restraints"}};
    QJsonObject _doc, _buffer;
    QStandardItemModel *_model;
    QStandardItem *item;




    void findConditions();
    void setConditions(const unsigned id, const QString = "restraint", const unsigned type = 0);
    void hide() const;
    void display() const;
    void TakeChanges();
    void loadFile(const QString &fileName); //Открытие файла
    void createTree();
    void FindPath();


};
#endif // MAINWINDOW_H
