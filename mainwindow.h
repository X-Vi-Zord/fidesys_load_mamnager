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

public slots:

    void TakeItemFromTree(const QModelIndex&);

private slots :

    void FindFile();
    void SetChanges();
    bool SaveToFile();

private:

    Ui::MainWindow  *ui;
    std::array<QLineEdit*, 6>   _lines;
    std::array<QCheckBox*, 6>   _flags;
    std::array<QLabel*, 6>      _labels;
    std::map<unsigned, QString> _loads = {{3,"pressure" },{5,"force"},{35,"distributed force"}, {0,"restraints"}};
    QJsonObject                 _doc, _buffer;
    QJsonObject                 _read_file;
    QStandardItemModel         *_model;
    QStandardItem              *_item;
    QString                     _file_path;

    void Display() const;
    void FindConditions();
    void Hide() const;
    void LoadFile(const QString&);
    void NewTree();
    void TakeChanges();
};
#endif // MAINWINDOW_H
