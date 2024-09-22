#ifndef RESTRAINTS_FORM_H
#define RESTRAINTS_FORM_H

#include <QWidget>
#include <QJsonObject>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>



namespace Ui {
class restraints_form;
}

class restraints_form : public QWidget
{
    Q_OBJECT

public:
    explicit restraints_form(QWidget *parent = nullptr);
    ~restraints_form();

public slots:

    void set_form(QJsonObject*);

private slots:
    void on_pushButton_clicked();


signals:

    void new_res(QJsonObject*, int);

private:
    Ui::restraints_form *ui;

    QJsonObject *object;
    std::array <QLineEdit*, 6> lanes;

    std::array <QCheckBox*, 6> Check;
};

#endif // RESTRAINTS_FORM_H
