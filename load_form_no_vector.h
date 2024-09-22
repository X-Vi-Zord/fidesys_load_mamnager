#ifndef LOAD_FORM_NO_VECTOR_H
#define LOAD_FORM_NO_VECTOR_H

#include <QWidget>
#include <QJsonObject>
#include <QLineEdit>

namespace Ui {
class load_form_no_vector;
}

class load_form_no_vector : public QWidget
{
    Q_OBJECT

public:
    explicit load_form_no_vector(QWidget *parent = nullptr);
    ~load_form_no_vector();

public slots:

    void set_form(QJsonObject*);

signals:

    void new_load(QJsonObject*,int const);

private slots:

    void on_pushButton_clicked();

private:
    Ui::load_form_no_vector *ui;
    QJsonObject *object;

};

#endif // LOAD_FORM_NO_VECTOR_H
